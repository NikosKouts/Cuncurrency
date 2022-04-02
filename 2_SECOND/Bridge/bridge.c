#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "header.h"

#define INITIAL_VAL 0x01 //Semaphore Value

enum color {
    red = -1,
    neutral = 0,
    blue = 1
}; 

typedef struct bridge {
    int capacity;
    int crossing;
    enum color color;
    bool is_waiting;
} bridge_t;

typedef struct arguments {
    enum color car_color;
    bridge_t *bridge;
    int number;
    int semid;
} arguments_t;

/*
    {1, 1, 1, 0} ~~~> Check, ..., Color, Capacity
*/
void bridge_entry(arguments_t *arguments){
    mysem_down(arguments->semid, 0);

    if(arguments->bridge->color != arguments->car_color && arguments->bridge->color != neutral){
        mysem_up(arguments->semid, 0);
        mysem_down(arguments->semid, 2);
        mysem_up(arguments->semid, 2);
        mysem_down(arguments->semid, 0);
    }

    if(!arguments->bridge->crossing){
        arguments->bridge->color = arguments->car_color;
        mysem_down(arguments->semid, 2);
    }
    if(arguments->bridge->crossing >= arguments->bridge->capacity){
        mysem_up(arguments->semid, 0);
        mysem_down(arguments->semid, 3);
        mysem_down(arguments->semid, 0);
    }

    //Print
    if(arguments->car_color == red){
        fflush(stdout);
        printf("\033[1;31m[START] RED\033[0m\n");
    }
    else if(arguments->car_color == blue){
        fflush(stdout);
        printf("\033[0;34m[START] BLUE\033[0m\n");
    }

    arguments->bridge->crossing++;
    mysem_up(arguments->semid, 0);   
}

void bridge_exit(arguments_t *arguments){
    mysem_down(arguments->semid, 0);
    arguments->bridge->crossing--;
    
    
    //Print
    if(arguments->car_color == red){
        fflush(stdout);
        printf("\033[1;31m[EXIT] RED\033[0m\n");
    }
    else if(arguments->car_color == blue){
        fflush(stdout);
        printf("\033[0;34m[EXIT] BLUE\033[0m\n");
    }



    if(arguments->bridge->crossing == 0){
        mysem_up(arguments->semid, 2);
    }
    if(arguments->bridge->crossing >= arguments->bridge->capacity - 1){
        mysem_up(arguments->semid, 3);
    }
    

    mysem_up(arguments->semid, 0);
}

void *crossing_bridge(void *arguments){
    arguments_t *args = (arguments_t *) arguments;

    //Wait Random Arrival Time
    sleep(rand() % 4);
    
    //Enter Bridge
    bridge_entry(args);

    //Exit Bridge
    bridge_exit(args);

    return NULL;
}


//Create Threads for Cars
pthread_t *create_car(arguments_t *arguments){
    pthread_t *threads;

    threads = (pthread_t *) malloc(arguments->number * sizeof(pthread_t));
    if(!threads)
        return NULL;

    for(int i = 0; i < arguments->number; i++){
        if(pthread_create(threads + i, NULL, crossing_bridge, (void *) arguments) != 0)
            return NULL;
    }
    return threads;
}


//Create and Initialize a Bridge
bridge_t *bridge_init(){
    bridge_t *bridge;
    bridge = (bridge_t *) malloc(sizeof(bridge_t));
    if(!bridge)
        return NULL;

    //Initialize Bridge
    printf("Bridge Capacity: ");
    scanf("%d", &bridge->capacity);
    if(bridge->capacity <= 0)
        return NULL;
    printf("\n");
    bridge->crossing = 0;
    bridge->color = neutral;

    return bridge;
}


//Initialize Two Lanes of Cars (Create Cars)
arguments_t *lane_init(enum color color, bridge_t *bridge, int semid){
    arguments_t *lane;

    lane = (arguments_t *) malloc(sizeof(arguments_t));
    if(!lane)
        return NULL;

    //Assign Car Color
    lane->car_color = color;
    lane->bridge = bridge;
    scanf("%d", &lane->number);
    if(lane->number < 0)
        return NULL;
    //Semaphore to Control Traffic
    lane->semid  = semid;
    printf("\n");
    return lane;
}


int main(int argc, const char *argv[]){
    pthread_t *t1, *t2;
    bridge_t *bridge;
    arguments_t *red_lane, *blue_lane;
    int semaphores[] = {1, 1, 1, 0};
    int semid = mysem_create(4, semaphores);

    //Initialize Bridge Properties
    bridge = bridge_init();
    if(!bridge)
        return -1;

    //Red Lane
    printf("\033[1;31mRed Cars: \033[0m");
    if(!(red_lane = lane_init(red, bridge, semid))) //Semaphore[0]
        return -1;
    
    //Blue Lane
    printf("\033[0;34mBlue Cars: \033[0m");
    if(!(blue_lane = lane_init(blue, bridge, semid))) //Semaphore[1]
        return -1;
    

    
    if(!(t1 = create_car(red_lane)))
        return -1;
    if(!(t2 = create_car(blue_lane)))
        return -1;

    for(int i = 0; i < red_lane->number; i++){
        pthread_join(t1[i], NULL);
    }

    for(int i = 0; i < blue_lane->number; i++){
        pthread_join(t2[i], NULL);
    }
    

    return 0;
}