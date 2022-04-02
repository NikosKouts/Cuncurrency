#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct arguments {
int seats;
int reservations;
int taken;
int peoples;
int finish;
}argument_t;

argument_t arguments;

pthread_mutex_t client_mtx, train_mtx;
pthread_cond_t full, riding, train;
bool exiting = false;

void *train_ride();
void *client();

int main(int argc, char const *argv[]){
    pthread_t *clients, train;
    
    if(argc != 3){
        printf("WRONG ARGUMENTS!\n");
        exit(1);
    }
    arguments.peoples = atoi(argv[1]);
    arguments.seats = atoi(argv[2]);
    arguments.reservations = atoi(argv[2]);
    arguments.taken = 0;
    arguments.finish = 0;

    clients = (pthread_t *) malloc(arguments.peoples * sizeof(pthread_t));
    if(!clients)
        return -1;

    //Create Train
    if(pthread_create(&train, NULL, train_ride, NULL))
        return -1;

    for(int i = 0; i < arguments.peoples; i++){
        if(pthread_create(clients + i, NULL, client, NULL))
            return -1;
    }

    for(int i = 0; i < arguments.peoples; i++){
        if(pthread_join(clients[i], NULL))
            return -1;
    }

    return 0;
}


void *client(){
    pthread_mutex_lock(&client_mtx);

    arguments.reservations--;
    if( arguments.reservations < 0){
        pthread_cond_wait(&full, &client_mtx);  //Wait In Queue
    }
    arguments.finish++;
    arguments.taken++;
    if( arguments.taken >=  arguments.seats){
        pthread_cond_signal(&train);
    }

    printf("\033[0;31mTaken Seats: %d\033[0m\n",  arguments.taken);
    
    if(arguments.finish != arguments.peoples || arguments.taken == arguments.seats)
        pthread_cond_wait(&riding, &client_mtx); //Wait for Train Ride

    printf("\033[0;31mExiting Client: %d\033[0m\n",  arguments.taken);
    arguments.taken--;
    if( arguments.taken > 0){
        pthread_cond_signal(&riding);
    }
    else {
        for(int i = 0; i <  arguments.seats; i++){
            pthread_cond_signal(&full);
        }
    }
    pthread_mutex_unlock(&client_mtx);
    return NULL;
}

void *train_ride(void *arguments){
    while (1){
        pthread_mutex_lock(&train_mtx);
        pthread_cond_wait(&train, &train_mtx);
        
        printf("\033[0;32mStarting Train Ride\033[0m\n");
        sleep(1);
        printf("\033[0;32mEnd of Train Ride\033[0m\n");


        pthread_cond_signal(&riding);
        pthread_mutex_unlock(&train_mtx);
    }

    return NULL;
}





