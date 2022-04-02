#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "header.h"

CCR_DECLARE(label);

typedef struct arguments {
  int value;
  int id;
} argument_t;

int total_threads, saver = -1;
bool blocked_main = false;
bool JOIN = false;

void *prime_thread(void *arguments);
int isPrime(int val);
void CreateNumbers(char *c);
void entry(argument_t *argument);
void check_exit(argument_t *argument);

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

  CCR_INIT(label);

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
  int i = 0;
  while (fscanf(input_file, "%d", &value) != EOF) {
    
    if(arguments[i].value > 0) {
      saver = arguments[i].id;
      
      CCR_DO(label, true, NULL);
     // printf("-main-\n");
     // fflush(stdout);
      CCR_DO(label, blocked_main, NULL);

      blocked_main = false;
    }

    arguments[i].value = value;
    fprintf(stderr, "main[%d] -> %d\n", arguments[i].id, arguments[i].value);
    fflush(stderr);
    i++;

    if(i == total_threads)
      i = 0;
  }

  JOIN = true;
  CCR_DO(label, true, NULL); 

  for (int i = 0; i < total_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  
  return (0);
}

void entry(argument_t *argument) {
    //printf("ENTRY [%d]\n", argument->id);
    //fflush(stdout);
}

void check_exit(argument_t *argument) {
	fprintf(stderr, "EXIT [%d]\n", argument->id);
  fflush(stderr);
  argument->value = -1;

  if(argument->id == saver){
    blocked_main =true;
    saver = -1;
    //printf("t -> main [%d]\n", argument->id);
    fflush(stdout);
  }
}

void *prime_thread(void *arguments) {
  while (!JOIN) {
    CCR_DO(label, ((argument_t *)arguments)->value > 0  || JOIN, entry((argument_t *)arguments));

    if (((argument_t *) arguments)->value > 0){
      if(isPrime(((argument_t *)arguments)->value) == 0) {
        printf("Thread %d: %d is not a Prime.\n",((argument_t *)arguments)->id, ((argument_t *)arguments)->value);
        fflush(stdout);
      }
      else {
        printf("Thread %d: %d is a Prime.\n",((argument_t *)arguments)->id, ((argument_t *)arguments)->value);
        fflush(stdout);
      }
    }
    
 

   CCR_DO(label, true, check_exit((argument_t *)arguments));
  }
  printf("Thread[%d] Completed\n", ((argument_t *)arguments)->id);
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