#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct arguments {
  int value;
  int id;
} argument_t;

pthread_mutex_t mtx;
pthread_cond_t main_queue, threads_queue;

// GLOBAL
int total_threads, waiting = 0, thread_index;
bool JOIN = false;

void *prime_thread(void *arguments);
int isPrime(int val);
void CreateNumbers(char *c);
void entry(argument_t *arg);
void check_exit(argument_t *arg);

int main(int argc, char *argv[]) {
  int value;
  argument_t *arguments;
  pthread_t *threads;
  FILE *input_file = fopen("input.txt", "r");

  // Arguments
  if (argc != 3) {
    printf("argv[0]: Executable\nargv[1]: Threads\nargv[2]: Random Numbers\n");
    exit(1);
  }
  total_threads = atoi(argv[1]);

  // CREATE NUMBERS
  CreateNumbers(argv[2]);

  // CREATE STRUCTS FOR ALL THREADS
  arguments = (argument_t *)malloc(total_threads * sizeof(argument_t));
  if (!arguments) 
    exit(1);

  //Create Thread List
  threads = (pthread_t *)malloc(total_threads * sizeof(pthread_t));
  if (!threads) exit(1);

  //Initialize Arguments
  for (int i = 0; i < total_threads; i++) {
    arguments[i].value = -1;
    arguments[i].id = i;
  }

  //Create Threads
  for(int i = 0; i < total_threads; i++){
    if(pthread_create(threads + i, NULL, prime_thread, (void *) (arguments + i)))
      exit(0);
  }

  // WHILE(JOB EXIST)
	pthread_mutex_lock(&mtx);
  thread_index = 0;
  while (fscanf(input_file, "%d", &value) != EOF){
    //Populate Threads
    if(thread_index >= total_threads){
      
      if(waiting > 0){
        pthread_cond_signal(&threads_queue);
      }
      pthread_cond_wait(&main_queue, &mtx);
    }
    arguments[thread_index].value = value;
    thread_index++;
	}
  if(waiting > 0){
    pthread_cond_signal(&threads_queue);
  }

  pthread_mutex_unlock(&mtx);

  //printf("***** Joining *****\n");
  JOIN = true;

  for (int i = 0; i < total_threads; i++) {
    pthread_join(threads[i], NULL);
  }
    

  return (0);
}

void entry(argument_t *argument) {
  pthread_mutex_lock(&mtx);
  if(argument->value < 0){
    waiting++;
    
    //fprintf(stderr, "[WAIT] Thread[%d]\n", argument->id);
    
    pthread_cond_wait(&threads_queue, &mtx);

    waiting--;
    if(waiting > 0){
      pthread_cond_signal(&threads_queue);
    }
  }

  pthread_mutex_unlock(&mtx);
}

void check_exit(argument_t *argument) {
	pthread_mutex_lock(&mtx);
  argument->value = -1;
  thread_index--;
  if(!thread_index){
    pthread_cond_signal(&main_queue);
  }
  
  pthread_mutex_unlock(&mtx);
}

void *prime_thread(void *arguments) {
  while (1) {
    entry((argument_t *)arguments);

    if(((argument_t *)arguments)->value > 0) {
      if(isPrime(((argument_t *)arguments)->value) == 0) {
        printf("Thread %d: %d is not a Prime.\n",((argument_t *)arguments)->id, ((argument_t *)arguments)->value);
      }
      else {
        printf("Thread %d: %d is a Prime.\n",((argument_t *)arguments)->id, ((argument_t *)arguments)->value);
      }
    }
    sleep(2);
    check_exit((argument_t *)arguments);
    if(JOIN)
      break;
  }
  printf("thread %d completed\n", ((argument_t *)arguments)->id);
  return NULL;
}

int isPrime(int val) {
  for (int i = 2; i <= val / 2; ++i) {
    if (val % i == 0) {
      return (0);
    }
  }
  return (1);
}

void CreateNumbers(char *c) {
  int numbers, num;
  char space[] = " ";
  FILE *file;
  time_t t;

  file = fopen("input.txt", "w");

  if (file == NULL) {
    printf("Error with the file!\n");
    exit(1);
  }

  numbers = atoi(c);
  srand((unsigned)time(&t));

  for (int i = 0; i < numbers; i++) {
    num = rand() % 100;
    printf("[%d] ", num);
    fprintf(file, "%d", num);
    fprintf(file, "%s", space);
  }
  printf("\n");
  fclose(file);
}