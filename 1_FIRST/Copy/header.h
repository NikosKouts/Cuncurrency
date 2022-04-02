#ifndef _HEADER_H
#define _HEADER_H

//Function Declarations
void pipe_init(int size);
void pipe_write(char c);
void pipe_writeDone();
int pipe_read(char *c);

//Enums
enum conditions {
    JOIN = -2,
    complete = -1,
    reading = 0,
    writing  = 1
};

//Struct Declarations
typedef struct {
    int size;
    int position;
    enum conditions condition;
} properties_t;

typedef struct {
    FILE *ReadFile;
    FILE *WriteFile;
} files_t;


//Global Declarations
char *PIPE;
files_t files;
properties_t properties;





#endif