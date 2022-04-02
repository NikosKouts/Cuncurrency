#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "header.h"

typedef struct arguments {
     int value;
     int semid;
     int sem_num;
     int mtx;
}argument_t;


void *func(void *arguments);
int isPrime(int val);
void CreateNumbers(char *c);

int main(int argc, char *argv[]) {
    int num_of_threads, val = -2, semid, mtx, i, *sem_array, m[] = {1};
    argument_t *arguments;
    pthread_t *thread;
    FILE *input_file;
    input_file = fopen("input.txt", "r");

    

    //HOW MANY THREADS WE WILL HAVE
    if(argc != 3){
        printf("ERROR!\nargv[1] = numbers of threads.\nargv[2] = how many numbers we want to check.\n");
        exit(1);
    }
    else
        num_of_threads = atoi(argv[1]);

    //CREATE NUMBERS
    CreateNumbers(argv[2]);

    //SEMAPHORES
    sem_array = (int *) malloc(sizeof(int) * num_of_threads);
    for (i = 0; i < num_of_threads; i++)
        sem_array[i] = 0;
    semid = mysem_create(num_of_threads, sem_array);
    mtx = mysem_create(1, m);

    //CREATE STRUCTS FOR ALL THREADS
    arguments = (argument_t *) malloc(sizeof(argument_t) * num_of_threads);
    if(!arguments)
        exit(1);

    //CREATE THREADS AND INITIALIZE THE STRUCTS
    thread = (pthread_t *) malloc(num_of_threads * sizeof(pthread_t));
    if(!thread)
        exit(1);   

    for(int i = 0; i < num_of_threads; i++) {
        arguments[i].value = -1;
        arguments[i].semid = semid;
        arguments[i].sem_num = i;
        arguments[i].mtx = mtx;
        pthread_create(thread + i, NULL, func, (void *) &arguments[i]);
    }
    
    //WHILE(JOB EXIST)
    i = 0;
    while(fscanf(input_file, "%d", &val) != EOF){
        mysem_down(mtx, 0);
        arguments[i].value = val;
        //printf("Sending Value: %d\n", val);
        mysem_up(semid, i);
        i++;
        if(i >= num_of_threads)
            i = 0;
    }

    //WAIT ALL THREADS TO FINISH
    mysem_down(mtx, 0);
    mysem_up(mtx, 0);

    for (i = 0; i < num_of_threads; i++) {
        mysem_down(mtx, 0);
        arguments[i].value = -2;
        mysem_up(semid, i);
    }
    mysem_down(mtx, 0);

    mysem_destroy(semid);
    mysem_destroy(mtx);

    return(0);
}

void *func(void * arguments){
    argument_t *container = (argument_t *) arguments;

    while(1){
        mysem_down(container->semid, container->sem_num);
        if(container->value == -2)
              break;
        
        mysem_up(container->mtx, 0);
    
        if(isPrime(container->value) == 0) {
            printf("Thread %d: %d is not a Prime.\n",container->sem_num, container->value);
            fflush(stdout);
        }
        else {
            printf("Thread %d: %d is a Prime.\n",container->sem_num, container->value);
            fflush(stdout);
        }
    }
    printf("thread %d completed\n", container->sem_num);
    fflush(stdout);
    mysem_up(container->mtx, 0);

    return NULL;
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