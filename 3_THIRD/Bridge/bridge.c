#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//****************************** GLOBAL ******************************//

//Mutex Locks
pthread_mutex_t mutex;


enum color {
  red_color = -1,
  blue_color = 0,
  neutral_color = 1
};

int capacity, crossing = 0, direction = neutral_color;

//********************************************************************//

typedef struct cars {
  int count;
  enum color color;
  int color_waiting;
  int capacity_waiting;
  pthread_cond_t queue;
  struct cars *opponent;
} cars_t;


void entry_check(cars_t *arguments){
  //Run Synchronously
  pthread_mutex_lock(&mutex);
  if(arguments->color != direction && direction != neutral_color){
    arguments->color_waiting++;
    pthread_cond_wait(&arguments->queue, &mutex);
    if(arguments->color_waiting > 0){
      arguments->color_waiting--;
      pthread_cond_signal(&arguments->queue);
    }
  }

  while (crossing >= capacity){
    arguments->capacity_waiting++;
    pthread_cond_wait(&arguments->queue, &mutex);
    if(arguments->capacity_waiting > 0){
      arguments->capacity_waiting--;
      pthread_cond_signal(&arguments->queue);
    }
  }
  
  //Set Direction
  direction = arguments->color;
  //Add Car on the Bridge
  crossing++;

  //Entry
  if(((cars_t *) arguments)->color == red_color)
    printf("\033[0;31m[Crossing] Red\033[0m\n");
  else
    printf("\033[0;34m[Crossing] Blue\033[0m\n");


  pthread_mutex_unlock(&mutex);
}

void exit_check(cars_t *arguments){
  //Run Synchronously
  pthread_mutex_lock(&mutex);

  //Exit
  if(arguments->color == red_color)
    printf("\033[0;31m[Exiting] Red\033[0m\n");
  else
    printf("\033[0;34m[Exiting] Blue\033[0m\n");

  crossing--;
  if(!crossing){
    if(arguments->opponent->color_waiting || arguments->opponent->capacity_waiting)
      pthread_cond_signal(&arguments->opponent->queue);
    else
      pthread_cond_signal(&arguments->queue);
  }
  else {
    if(!arguments->opponent->color_waiting)
      pthread_cond_signal(&arguments->queue);
  }

  pthread_mutex_unlock(&mutex);
}



void *cross_bridge(void *arguments){
  sleep(rand() % 2);  //Random Order

  //Check Before Entering
  entry_check((cars_t *) arguments);

  sleep(2); //Crossing Bridge

  //Exit and Handle Traffic
  exit_check((cars_t *) arguments);

  
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
    cars->color_waiting = 0;
    cars->capacity_waiting = 0;

    return cars;
}


int main(int argc, char const *argv[]){
  int red_count, blue_count;
  cars_t *red, *blue;
  pthread_t *red_lane, *blue_lane;

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
