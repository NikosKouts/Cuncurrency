#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "header.h"

typedef struct arguments {
    int total_people;
    int N;
    int people_on_board;
    int remaining_people;
    int semid;
} arguments_t;


void *get_on_train(void *arguments){
    arguments_t *args = (arguments_t *) arguments;

    mysem_down(args->semid, 2);
    args->people_on_board++;
    args->remaining_people--;
    printf("\033[1;31mPeople on Board: %d\033[0m\n", args->people_on_board);
    if(args->people_on_board >= args->N){
        mysem_up(args->semid, 0);
        mysem_down(args->semid, 1);
    }
    if(args->remaining_people <= 0){
        mysem_up(args->semid, 0);
    }
    
    mysem_up(args->semid, 2);

    return NULL;
}


void *start_train_ride(void *arguments){
    arguments_t *args = (arguments_t *) arguments;
    while(1){
         mysem_down(args->semid, 0);
         if(args->remaining_people == 0 && args->people_on_board < args->N)
            break;
         printf("\033[0;32mTrain Ride Start\033[0m\n");
         sleep(2);
         printf("\033[0;32mTrain Ride End\033[0m\n");
         args->people_on_board = 0;
         mysem_up(args->semid, 1);
    }
    mysem_up(args->semid, 3);
    return NULL;
}


bool arguments_init(arguments_t *arguments){
    int sem_values[] = {0, 0, 1, 0}; //tmtx, waiting, pmtx, join
    printf("People: ");
    scanf("%d", &arguments->total_people);
    if(arguments->total_people <= 0){
        printf("People is a Positive Value\n");
        return false;
    }
    printf("\n");
    printf("Seats: ");
    scanf("%d", &arguments->N);
    if(arguments->N <= 0){
        printf("Seats is a Positive Value\n");
        return false;
    }
    printf("\n");
    arguments->people_on_board = 0;
    arguments->remaining_people = arguments->total_people;

    //Create 3 Semaphores
    arguments->semid = mysem_create(4, sem_values);

    return true;
}

//Create People (Threads)
pthread_t *create_people(arguments_t *arguments){
    pthread_t *threads;

    threads = (pthread_t *) malloc(arguments->total_people * sizeof(pthread_t));
    if(!threads)
        return NULL;
    
    for(int i = 0; i < arguments->total_people; i++){
        if(pthread_create(threads + i, NULL, get_on_train, (void *) arguments) != 0)
            return NULL;
    }
    return threads;
}

//Create Train
pthread_t create_train(arguments_t *arguments){
    pthread_t thread;
    if(pthread_create(&thread, NULL, start_train_ride, (void *) arguments) != 0)
        exit(-1);
    return thread;
}


int main(int argc, const char *argv[]){
    arguments_t arguments;
    pthread_t *people, train;

    if(!arguments_init(&arguments))
        return -1;
    
    //Create People Threads
    people = create_people(&arguments);
    if(!people)
        return -1;

    //Create Train
    train = create_train(&arguments);

    
    mysem_down(arguments.semid, 3);


    return 0;
}