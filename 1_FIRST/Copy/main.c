#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "header.h"
#define LENGTH 64


void *WritingFIFO(void *arguments);
void *ReadingFIFO(void *arguments);


int main(int argc, const char *argv[]){
    pthread_t write, read;

    //Get PIPE Size
    if(argc == 2)
        properties.size = atoi(argv[1]);
    else
        properties.size = LENGTH;


    //Initialize PIPE
    pipe_init(properties.size);
    properties.position = 0;
    properties.condition = writing;
    
    //File Management
    files.ReadFile = fopen("image.jpeg", "rb");
    files.WriteFile = fopen("OUTPUT", "w+");

    pthread_create(&write, NULL, WritingFIFO, NULL);
    pthread_create(&read, NULL, ReadingFIFO, NULL);

    while(properties.condition != JOIN);

    return 0;
}


void *WritingFIFO(void *arguments){
    char letter;

    //Exit After WriteDone
    while(properties.condition != complete){
        //If I Can Write Read File and Write to FIFO
        if(properties.condition == writing){
            while(true){
                if(properties.position != properties.size){
                    //IF NOT EOF -> pipe_write ----[ELSE]---- IF EOF -> Write Done + Break
                    if(fread(&letter, sizeof(char), 1, files.ReadFile)){
                        pipe_write(letter);
                        properties.position++;
                    } else {
                        pipe_writeDone();
                        properties.position--;
                        break;
                    }
                } else {
                    properties.condition = reading;
                    properties.position--;
                    break;
                }
            }
        }
    }
    printf("Thread End: [WritingFIFO]\n");
    return NULL;
}

void *ReadingFIFO(void *arguments){
    while(true){
        if(properties.condition == reading || properties.condition == complete){
            if(pipe_read(PIPE) == 0){
                break;
            }
            else {
                if(properties.condition == complete){
                    break;
                }
                properties.condition = writing;
                properties.position = 0;
            }
                  
        }
    }
    printf("Thread End: [ReadingFIFO]\n");
    properties.condition = JOIN;
    return NULL;
}