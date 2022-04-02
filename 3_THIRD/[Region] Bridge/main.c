#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "header.h"
//****************************** GLOBAL ******************************//

//Mutex Locks
pthread_mutex_t mutex, mtx;


enum color {
  red_color = -1,
  blue_color = 0,
  neutral_color = 1
};

volatile int capacity, crossing = 0, direction = neutral_color;

//CCR
CCR_DECLARE(R);

//********************************************************************//

typedef struct cars {
  int count;
  volatile enum color color;
  int waiting;
  pthread_cond_t queue;
  struct cars *opponent;
} cars_t;

void bridge_entry(cars_t *cars){
  //Entry
  if(cars->color == red_color)
    printf("\033[0;31m[Crossing] Red\033[0m\n");
  else
    printf("\033[0;34m[Crossing] Blue\033[0m\n");

  fflush(stdout);

  if(!crossing){
    //Set Direction
    direction = cars->color;
  }
  crossing++;
}

void bridge_exit(cars_t *cars){
  //Exit
  if(cars->color == red_color)
    printf("\033[0;31m[Exiting] Red\033[0m\n");
  else
    printf("\033[0;34m[Exiting] Blue\033[0m\n");

  fflush(stdout);

  crossing--;
  if(!crossing){
    direction = neutral_color;
  }
}

void debug_before(cars_t *cars){
  //Exit
  if(cars->color == red_color)
    fprintf(stderr, "\033[0;31m* [Blocked] -> Red\033[0m\n");
  else
    fprintf(stderr, "\033[0;34m* [Blocked] -> Blue\033[0m\n");
  
  fflush(stdout);
}

void debug_after(cars_t *cars){
  //Exit
  if(cars->color == red_color)
    fprintf(stderr, "\033[0;31m* [Unblocked] -> Red\033[0m\n");
  else
    fprintf(stderr, "\033[0;34m* [Unblocked] -> Blue\033[0m\n");
  
  fflush(stdout);
}

void *cross_bridge(void *arguments){
  cars_t *cars = (cars_t *) arguments;
  
  
  CCR_DO(R, (cars->color == direction || direction == neutral_color) && crossing < capacity, bridge_entry(cars));


  sleep(2); //Crossing Bridge

  //Exit and Handle Traffic
  CCR_DO(R, 1, bridge_exit(cars));

  return NULL;
}



pthread_t *car_generator(cars_t *cars){
  pthread_t *threads = (pthread_t *) malloc(cars->count * sizeof(pthread_t));
  if(!threads)
    return NULL;

  for(int i = 0; i < cars->count; i++){
    if(pthread_create(threads + i, NULL, cross_bridge, (void *) cars))
      return NULL;
  }

  

  return threads;
}


//Initialize Car Lane Properties
cars_t *cars_init(enum color color, int count){
    cars_t *cars = (cars_t *) malloc(sizeof(cars_t));
    if(!cars)
      return NULL;
    
    cars->color = color;
    cars->count = count;
    cars->waiting = 0;

    return cars;
}


int main(int argc, char const *argv[]){
  int red_count, blue_count;
  cars_t *red, *blue;
  pthread_t *red_lane, *blue_lane;
  CCR_INIT(R);
  printf("Capacity: ");
  scanf("%d", &capacity);

  printf("\033[0;31mRed: \033[0m");
  scanf("%d", &red_count);

  printf("\033[0;34mBlue: \033[0m");
  scanf("%d", &blue_count);

  //Create Cars
  red = cars_init(red_color, red_count);
  if(!red)
    exit(-1);

  blue = cars_init(blue_color, blue_count);
  if(!blue)
    exit(-1);

  //Mark Opponents
  red->opponent = blue;
  blue->opponent = red;

  //Create Threads
  red_lane = car_generator(red);
  if(!red_lane)
    exit(-1);
  blue_lane = car_generator(blue);
  if(!blue_lane)
    exit(-1);

  
  //Wait for Threads to Finish
  for(int i = 0; i < red->count; i++){
    if(pthread_join(red_lane[i], NULL))
      exit(-1);
  }

  for(int i = 0; i < blue->count; i++){
    if(pthread_join(blue_lane[i], NULL))
      exit(-1);
  }

  return 0;
}
