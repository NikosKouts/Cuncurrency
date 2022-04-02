#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"


void pipe_init(int size){
    PIPE = (char *) calloc(size, sizeof(char));
    if(!PIPE)
        return;
}


void pipe_write(char c){
    char temp;

    //Swap Values
    for(int i = properties.position; i > -1; i--){
        temp = PIPE[i + 1];
        PIPE[i + 1] = PIPE[i];
        PIPE[i] = temp;
    }
    //Add On First Position
    PIPE[0] = c;
}

void pipe_writeDone(){
    properties.condition = complete;
}


int pipe_read(char *c){
    if(strlen(c) == 0 && properties.condition == complete)
        return 0;
    
    for(int i = properties.position; i > -1; i--){
        fputc(c[i], files.WriteFile);
        c[i] = '\0';
    }
    return 1;
}