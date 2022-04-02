#ifndef HEADER_H
#define HEADER_H

int mysem_create(int semaphores, int *init_values);
void mysem_down(int semid, int sem_num);
void mysem_up(int semid, int sem_num);
void mysem_destroy(int semid);

#endif