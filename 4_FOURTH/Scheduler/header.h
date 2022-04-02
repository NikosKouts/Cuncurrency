#ifndef HEADER_H
#define HEADER_H

#include <ucontext.h>

//************** Struct Schemas **************//
enum state {
	terminated = -1,
	blocked = 0,
	ready = 1,
	running = 2
}; 

typedef struct thread {
	volatile int thread_id;
	ucontext_t *context; 
	enum state state;
} thread_t;

typedef struct task {
	thread_t *thread;
	struct task *next;
} task_t;

typedef struct semaphore {
	int value;
	int id;
	task_t *head;
	task_t *tail;
} semaphore_t;


typedef struct scheduler {
	task_t *head;
	task_t *tail;
	task_t *current;
	task_t *index;
	int size;
} scheduler_t;


//************** Global Variables **************//
scheduler_t *scheduler_FIFO;
int thread_count, semaphore_count;
ucontext_t scheduler_context;
//************** Export Functions **************//
int mythreads_init();
int mythreads_create(thread_t *thread, void (body)(void *), void *arguments);
int mythreads_yield();
int mythreads_join(thread_t *thread);
int mythreads_sem_init(semaphore_t *semaphore, int value);
int mythreads_sem_down(semaphore_t *semaphore);
int mythreads_sem_up(semaphore_t *semaphore);
int mythreads_sem_destroy(semaphore_t *semaphore);
int mythreads_destroy(thread_t *thread);
#endif
    