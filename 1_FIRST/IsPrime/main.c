#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct arguments {
     int value;
     int available;
     int thread_print;
}argument_t;


void *func(void *number);
int isPrime(int val);


int main(int argc, char const *argv[]) {
    int num_of_threads, val = 0, complete = 0;
    argument_t *arguments;
    pthread_t thread;
    FILE *input_file;

    input_file = fopen("INPUT","r");

    //HOW MANY THREADS WE WILL HAVE
    if( argc != 2)
        exit(1);
    else
    num_of_threads = atoi(argv[1]);

    //CREATE STRUCTS FOR ALL THREADS
    arguments = (argument_t *)malloc(sizeof(argument_t) * num_of_threads);
    if(!arguments)
        exit(1);

    //CREATE THREADS AND INITIALIZE THE STRUCTS
    for(int i = 0; i <  num_of_threads; i++) {
        arguments[i].available = 1;
        arguments[i].value = -1;
        pthread_create(&thread, NULL, func, (void *)&arguments[i]);
    }

    //WHILE(JOB EXIST)
    while(fscanf(input_file, "%d", &val) != EOF) {
       
        //IF NUMBERS IS MORE FROM THREADS
        while(val != -1){

            //CHOICE THE THREAD WHO IS AVAILABLE
            for (int i = 0; i < num_of_threads; i++) {
                if (arguments[i].available == 1) {
                    arguments[i].value = val;
                    arguments[i].thread_print = i;
                    arguments[i].available = 0;
                    val = -1;
                    break;
                }          
            }
        }
    }

    fclose(input_file);

    //WE WAIT UNTIL ALL THREADS FINISH
    while(1){
        for (int i = 0; i < num_of_threads; i++) {
            if(arguments[i].available == 1)
                complete++;
        }
        if (complete == num_of_threads)
            break;
        else    
            complete = 0;
    }

    return 0;
}

void *func(void * arguments){
    argument_t *container = (argument_t *)arguments;

   while(1){
       if(container->available == 0){
            
            if(isPrime(container->value) == 0) {
                printf("Thread %d: %d is not a Prime.\n",container->thread_print, container->value);
                fflush(stdout);
            }
            else {
                printf("Thread %d: %d is a Prime.\n",container->thread_print, container->value);
                fflush(stdout);
            }
            container->available = 1;    
       }
    }
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
