#ifndef HEADER_H
#define HEADER_H

#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct region {
  pthread_mutex_t mutex;
  pthread_cond_t q1;
  pthread_cond_t q2;
  int q1_len;
  int q2_len;
} region_t;

void init(region_t *label){
  //Initialize Mutex Lock
  if(pthread_mutex_init(&label->mutex, NULL))
    exit(0);

  //Initialize Queues
  if(pthread_cond_init(&label->q1, NULL) || pthread_cond_init(&label->q2, NULL))
    exit(0);

  //Initialize Queues Length
  label->q1_len = label->q2_len = 0;

}

#define CCR_DECLARE(label) region_t label

#define CCR_INIT(label) init(&label)

#define CCR_DO(label, lexpr, body){\
  pthread_mutex_lock(&label.mutex);\
  while (!(lexpr)){\
    label.q1_len++;\
    if(label.q2_len > 0){\
      label.q2_len--;\
      pthread_cond_signal(&label.q2);\
    }\
    fprintf(stderr, "\033[0;32mQueue[1] %d WAIT %ld\n\033[0m", label.q1_len, pthread_self());\
    fflush(stderr);\
    pthread_cond_wait(&label.q1, &label.mutex);\
    fprintf(stderr, "\033[0;33mQueue[1] SIGNAL %ld\n\033[0m", pthread_self());\
    fflush(stderr);\
    label.q2_len++;\
    if(label.q1_len > 0){\
      label.q1_len--;\
      pthread_cond_signal(&label.q1);\
      fprintf(stderr, "\033[0;32mQueue[2] %d WAIT %ld\n\033[0m", label.q2_len, pthread_self());\
      fflush(stderr);\
      pthread_cond_wait(&label.q2, &label.mutex);\
      fprintf(stderr, "\033[0;33mQueue[2] SIGNAL %ld\n\033[0m", pthread_self());\
      fflush(stderr);\
    }\
    else {\
      if(label.q2_len >= 2){\
        label.q2_len--;\
        pthread_cond_signal(&label.q2);\
        fprintf(stderr, "\033[0;32mQueue[2] %d WAIT %ld\n\033[0m", label.q2_len, pthread_self());\
        fflush(stderr);\
        pthread_cond_wait(&label.q2, &label.mutex);\
        fprintf(stderr, "\033[0;33mQueue[2] SIGNAL %ld\n\033[0m", pthread_self());\
        fflush(stderr);\
      }\
    }\
  }\
  body;\
  if(label.q1_len > 0){\
    label.q1_len--;\
    pthread_cond_signal(&label.q1);\
  }\
  else if(label.q2_len > 0){\
    label.q2_len--;\
    pthread_cond_signal(&label.q2);\
  }\
  pthread_mutex_unlock(&label.mutex);\
}
  

#endif