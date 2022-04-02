#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <stdbool.h>
#include "header.h"

#define fifo_size 4

co_t reader, writer, cmain;

typedef struct arguments {
  char *FIFO;
  int position;
  FILE *read_file, *write_file;
  bool complete;
} arguments_t;

arguments_t arguments;


void reader_function() {
  
  while (1) {
    for(int i = arguments.position - 1; i > -1; i--){
        fputc(arguments.FIFO[i], arguments.write_file);
        arguments.FIFO[i] = '\0';
    }

    arguments.position = 0;
    //sleep(2);
    if(arguments.complete == true){
      printf("Reader switch to Main\n");
      break;
    }

    printf("Reader switch to Writer\n");
    mycoroutines_switchto(&writer);
    //swapcontext(&reader, &writer);
  }
}

void writer_function() {
  char letter, temp;
  while (fscanf(arguments.read_file, "%c", &letter) != EOF) {
    for(int i = arguments.position; i > -1; i--){
        temp = arguments.FIFO[i + 1];
        arguments.FIFO[i + 1] = arguments.FIFO[i];
        arguments.FIFO[i] = temp;
    }
    
    //Add On First Position
    arguments.FIFO[0] = letter;

    arguments.position++;

    if(arguments.position == fifo_size) {
      printf("Writer switch to Reader\n");
      mycoroutines_switchto(&reader);
      //swapcontext(&writer, &reader);
    }
  }
  arguments.complete = true;

  printf("Writer switch to Reader\n");
  mycoroutines_switchto(&reader);
  //swapcontext(&writer, &reader);
  printf("Writer switch to Main\n");
}

int compareFile(FILE * fPtr1, FILE * fPtr2, int * line, int * col)
{
    char ch1, ch2;

    *line = 1;
    *col  = 0;

    do
    {
        // Input character from both files
        ch1 = fgetc(fPtr1);
        ch2 = fgetc(fPtr2);
        
        // Increment line 
        if (ch1 == '\n')
        {
            *line += 1;
            *col = 0;
        }

        // If characters are not same then return -1
        if (ch1 != ch2)
            return -1;

        *col  += 1;

    } while (ch1 != EOF && ch2 != EOF);


    /* If both files have reached end */
    if (ch1 == EOF && ch2 == EOF)
        return 0;
    else
        return -1;
}


int main(int argc, char const *argv[]) {
  int line, col, diff;

  arguments.FIFO = (char *)calloc(sizeof(char), fifo_size);
  if(!arguments.FIFO)
    exit(1);

  if(argc != 2){
    printf("argv[0]: ./executable\nargv[1]: Input File\n");
    exit(EXIT_FAILURE);
  }

  arguments.position = 0;
  arguments.read_file = fopen(argv[1], "r");
  arguments.write_file = fopen("output.txt", "w+");
  arguments.complete = false;

  mycoroutines_init(&cmain);

  mycoroutines_create(&writer, writer_function, NULL);
  mycoroutines_create(&reader, reader_function, NULL);
  printf("Main switch to Writer\n");

  mycoroutines_switchto(&writer);
  printf("Main switch to Writer\n");
  mycoroutines_switchto(&writer);


  mycoroutines_destroy(&writer);
  mycoroutines_destroy(&reader);

  diff = compareFile(arguments.read_file, arguments.write_file, &line, &col);
  if(!diff){
    printf("\033[0;32mFiles Are Equal\033[0m\n");
  }
  else {
    printf("\033[0;31mFiles Are Differ\033[0m\n");
  }
  
  return 0;
}
