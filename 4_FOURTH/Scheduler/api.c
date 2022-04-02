#include <stdio.h> 
#include <ucontext.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "header.h"

#define CLK_PERIOD 2

ucontext_t terminate, join;
sigset_t sig_set;

static int signal_scheduler();

//Create a CPU Clock
void create_timer(){
  struct itimerval timer = { {0} };
  
  //Timer Configuration
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 600000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 600000;
  
  setitimer(ITIMER_REAL, &timer, NULL);
}

//Block Incoming Signals
void block_sigalarm(){  
  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGALRM);
  if(sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0){
    perror("Block Signal");
    exit(-1);
  }
}

//Unblock Signals
void unblock_sigalarm(){ 
  if(sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0){
    perror("Unblock Signal");
    exit(-1);
  }
}

//Stack and Pointers for Thread
static int context_make(thread_t *thread, void (body)(void *), void *arguments){
  //Create Context
  thread->context = (ucontext_t *) malloc(sizeof(ucontext_t));
  if(!thread->context)
    return -1;

  //Build Context
  getcontext(thread->context);
  thread->context->uc_stack.ss_sp = (char *) malloc(SIGSTKSZ * sizeof(char));
  thread->context->uc_stack.ss_size = SIGSTKSZ * sizeof(char);
  thread->context->uc_link = &terminate;
  makecontext(thread->context, (void (*)(void)) body, 1, arguments);

  return 0;
}

static void terminate_context(){
  getcontext(&terminate);
  
  if(scheduler_FIFO->size >= 2){
    scheduler_FIFO->index->thread->state = terminated;
    scheduler_FIFO->size--;
    setcontext(scheduler_FIFO->head->thread->context);  //Transfer Control Back to main()
  }
}



static bool scheduler_enqueue(thread_t *thread){
  task_t *task = (task_t *) malloc(sizeof(task_t));
  if(!task)
    return false;
  
  //Assign Pointers
  if(!scheduler_FIFO->head || !scheduler_FIFO->tail){
    scheduler_FIFO->head = scheduler_FIFO->tail = scheduler_FIFO->index = task;
  }
  
  task->next = scheduler_FIFO->head;
  scheduler_FIFO->tail->next = task;
  scheduler_FIFO->tail = task;
  
  //Initialize Data
  task->thread = thread;
  
  scheduler_FIFO->size++;

  return true;
}

static void scheduler(int signum){
  //Signal Safe with Block and Unblock
  block_sigalarm();

  //Current Running Context
  scheduler_FIFO->current = scheduler_FIFO->index;
  
  do {
    scheduler_FIFO->index = scheduler_FIFO->index->next;
  } while (scheduler_FIFO->index->thread->state != ready);
  
  printf("\033[0;33mSaving %d Running %d\033[0m\n", scheduler_FIFO->current->thread->thread_id, scheduler_FIFO->index->thread->thread_id);
  

  if(scheduler_FIFO->size >= 2){
    swapcontext(scheduler_FIFO->current->thread->context, scheduler_FIFO->index->thread->context);
  }
  unblock_sigalarm();
}

static int signal_scheduler(){
  
  //Run Scheduler
  signal(SIGALRM, scheduler);
  if(kill(getpid(), SIGALRM) < 0)
    return -1;

  return 0;
}

void print_semaphore_fifo(semaphore_t *semaphore){
  printf("Scheduler FIFO: ");
  for(task_t *current = semaphore->head; current != NULL; current = current->next)
    printf("%d -> ", current->thread->thread_id);
  printf("NULL\n");
}


//****************************************************** API ******************************************************//

//Initialize Scheduler, Append main() as a Head Thread and Activate Scheduler
int mythreads_init(){
  thread_t *main;
  struct sigaction act = {{ 0 }};

  //Use thread_count to Specify IDs
  thread_count = 0;
  //Use thread_count to Specify Semaphorse's ID
  semaphore_count = 0;

  //Create Scheduler
  scheduler_FIFO = (scheduler_t *) malloc(sizeof(scheduler_t));
  if(!scheduler_FIFO)
    return -1;

  scheduler_FIFO->head = scheduler_FIFO->tail = scheduler_FIFO->index = NULL;
  
  //Thread: main()
  main = (thread_t *) malloc(sizeof(thread_t));
  if(!main)
    return -1;
  
  main->thread_id = thread_count++;
  main->state = ready;
  main->context = (ucontext_t *) malloc(sizeof(ucontext_t));
 
  if(!scheduler_enqueue(main))
    return -1;
  
  //Listen for Context Termination
  terminate_context();


  //Activate Scheduler Timer
  act.sa_handler = scheduler;
  if(sigaction(SIGALRM, &act, NULL) < 0){
    perror("Timer Set Error");
    exit(-1);
  }

  //Activate Scheduler
  create_timer();
  return 0;
}

//Create a Thread and Append it to Scheduler
int mythreads_create(thread_t *thread, void (body)(void *), void *arguments){
  thread->thread_id = thread_count++;
  thread->state = ready;
  
  //Build a Context
  if(context_make(thread, body, arguments) < 0)
    return -1;

  //Schedule Context as a Scheduler Job
  if(!scheduler_enqueue(thread))
    return -1;

  return 0;
}

//Yield Thread By Calling Scheduler To Assign The Next Job
int mythreads_yield(){
  block_sigalarm();
  printf("Thread Yield\n");
  if(signal_scheduler() < 0)
    return -1;
  unblock_sigalarm();
  return 0;
}

int mythreads_join(thread_t *thread){
  
  //Find If Node Was 
  while(thread->state != terminated);

  return 0;
}

int mythreads_sem_init(semaphore_t *semaphore, int value){
  block_sigalarm();
  if(!semaphore)
    return -1;
  //Set Value
  semaphore->value = value;
  semaphore->id = semaphore_count++;
  semaphore->head = semaphore->tail = NULL;

  printf("Semaphore Init [%d]: %d\n", semaphore->id, semaphore->value);
  unblock_sigalarm();
  return 0;
}

int mythreads_sem_down(semaphore_t *semaphore){
  task_t *task;

  block_sigalarm();
  semaphore->value--;
  if(semaphore->value < 0) {
    scheduler_FIFO->index->thread->state = blocked;
    task = (task_t *) malloc(sizeof(task_t));
    if(!task)
      return -1;

    //Enque on Single Linked List (End of List)
    if(!semaphore->head || !semaphore->tail)
      semaphore->head = task;
    else
      semaphore->tail->next = task;

    semaphore->tail = task;
    task->next = NULL;

    //Initialize Data
    task->thread = scheduler_FIFO->index->thread;
    print_semaphore_fifo(semaphore);
    semaphore->value = -1;
    if(semaphore->value < 0)
      printf("Down[%d]: 0 [BLOCKED]\n", semaphore->id);
    else
      printf("Down[%d]: %d\n", semaphore->id, semaphore->value);

    //Yield to Next Thread
    block_sigalarm();
    if(signal_scheduler() < 0)
      return -1;
    unblock_sigalarm();
  }
  if(semaphore->value < 0)
    printf("Down[%d]: 0 [BLOCKED]\n", semaphore->id);
  else
    printf("Down[%d]: %d\n", semaphore->id, semaphore->value);

  unblock_sigalarm();
  return 0;
}

int mythreads_sem_up(semaphore_t *semaphore){
  task_t *current;
  block_sigalarm();
  //Blocked
  if(semaphore->value < 0){
    semaphore->head->thread->state = ready;

    //Remove Node
    current = semaphore->head;
    semaphore->head = semaphore->head->next;
    free(current);
  }
  semaphore->value++;

  printf("Up[%d]: %d\n", semaphore->id, semaphore->value);
  unblock_sigalarm();
  return 0;
}

int mythreads_sem_destroy(semaphore_t *semaphore){
  task_t *previous, *current;
  
  if(!semaphore->head || !semaphore->tail){
    return -1;
  }
  //Destroy List
  for(previous = semaphore->head, current = semaphore->head->next; current != NULL; previous = current, current = current->next){
    free(previous);
  }

    
  printf("Destroy Semaphore[%d]\n", semaphore->id);
  semaphore->head = semaphore->tail = NULL;
  
  return 0;
}

int mythreads_destroy(thread_t *thread){
  task_t *previous, *current;
  
  for(previous = scheduler_FIFO->head, current = scheduler_FIFO->head->next; current != scheduler_FIFO->head; previous = current, current = current->next){
    current = current->next;
    free(previous->next);
    previous->next = current;
  }
  unblock_sigalarm();
  return 0;
}