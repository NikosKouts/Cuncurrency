#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "header.h"
#include <pthread.h>

typedef struct arguments {
     int value;
     int id;
     semaphore_t sthread;
}argument_t;


void thread_fuction(void *arguments);
int isPrime(int val);
void CreateNumbers(char *c);

semaphore_t smain;
bool finish = false;

int main(int argc, char *argv[]) {
    int total_threads, val;
    argument_t *arguments;
    FILE *input_file;
    thread_t *thread;

    input_file = fopen("input.txt", "r");

    //HOW MANY THREADS WE WILL HAVE
    if(argc != 3){
        printf("ERROR!\nargv[1] = numbers of threads.\nargv[2] = how many numbers we want to check.\n");
        exit(1);
    }
    else
        total_threads = atoi(argv[1]);

    //CREATE NUMBERS
    CreateNumbers(argv[2]);

    //CREATE STRUCTS FOR ALL THREADS
    arguments = (argument_t *) malloc(sizeof(argument_t) * total_threads);
    if(!arguments)
        exit(1);

    //CREATE THREADS AND INITIALIZE THE STRUCTS
    thread = (thread_t *) malloc(total_threads * sizeof(thread_t));
    if(!thread)
        exit(1);   

    mythreads_init();
    mythreads_sem_init(&smain, 1);

    for(int i = 0; i < total_threads; i++) {
        arguments[i].value = -1;
        arguments[i].id = i;
        mythreads_sem_init(&arguments[i].sthread, 0);
        mythreads_create(&thread[i], thread_fuction, (void *)&arguments[i]);
    }
 
    //WHILE(JOB EXIST)
    int i = 0;
    while(fscanf(input_file, "%d", &val) != EOF){
      mythreads_sem_down(&smain);
        
      arguments[i].value = val;
      mythreads_sem_up(&arguments[i].sthread);

      i++;
      if(i >= total_threads)
        i = 0;
    }

    mythreads_sem_down(&smain);
    mythreads_sem_up(&smain);

    finish = true;

    for (i = 0; i < total_threads; i++) {
        mythreads_sem_down(&smain);
        mythreads_sem_up(&arguments[i].sthread);
        mythreads_join(&thread[i]);
    }
    mythreads_sem_down(&smain);

    //Destroy Semaphores and Threads
    for(int i; i < total_threads; i++){
        //mythreads_sem_destroy(&arguments[i].sthread);  
        //mythreads_destroy(&thread[i]); 
    }
    
    printf("Main Finished\n");
    
    return(0);
}

void thread_fuction(void * arguments){
    argument_t *container = (argument_t *) arguments;
    while(1){
      mythreads_sem_down(&container->sthread);

      if(finish)
        break;

      mythreads_sem_up(&smain);

      if(isPrime(container->value) == 0) {
          printf("\033[0;31mThread %d: %d is not a Prime\033[0m\n",container->id, container->value);
          fflush(stdout);
      }
      else {
          printf("\033[0;32mThread %d: %d is a Prime\033[0m\n",container->id, container->value);
          fflush(stdout);
      }
    }
    printf("\e[0;36mThread[%d]: Completed\e[0m\n", container->id);
    fflush(stdout);
    mythreads_sem_up(&smain);
}

int isPrime(int val){
    
    for(int i = 2; i <= val/2; ++i){
        if(val %  i == 0){
            return(0);
        }
    }
    return(1);   
}

void CreateNumbers(char *c){
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
    srand((unsigned) time(&t));

    for (int i = 0; i < numbers; i++) {
        num = rand() % 100;
        printf("[%d] ", num);
        fprintf(file,"%d", num);
        fprintf(file,"%s", space);
    }
    printf("\n");
    fclose(file);
}