#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define SEM_MAX 0x01 //Binary


union semun  {
    int val;
    struct semid_ds *buf;
    ushort *array;
} argument;

int mysem_create(int semaphores, int *init_values){
    //Create One Semaphore
    int semid = semget(IPC_PRIVATE, semaphores, IPC_EXCL | IPC_CREAT | S_IRWXU);
    //Semaphore Already Exists
    if(semid == -1){
        perror("[ERROR]: Semaphore Creation");
        exit(EXIT_FAILURE);
    }

    //Initialize Semaphores from Semaphore Group (semid) with a Parameter Value
    for(int i = 0; i < semaphores; i++){
        if(semctl(semid, i, SETVAL, init_values[i]) != 0){
            perror("[ERROR]: Semaphore Initialization");
            exit(EXIT_FAILURE);
        }
    }

    return semid;
}

//Set Semaphore Value to Zero
void mysem_down(int semid, int sem_num){
    struct sembuf sops[2];

    //Decrement Semaphore by One
    sops[0].sem_num = sem_num;
    sops[0].sem_op = -1;    //Decrement Semaphore by One
    sops[0].sem_flg = 0;

    //Atomically Wait for Thread To Become 0
    sops[1].sem_num = sem_num;
    sops[1].sem_op = 0;    //Wait for Semaphore to be 0
    sops[1].sem_flg = 0;
        
    //Set Semaphore to Zero
    if(semop(semid, sops, 2) == -1){
        perror("[ERROR]: Semaphore Down");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "---- DOWN %d: [%d] ----\n", semid, sem_num);
}


//Set Semaphore Value to One
void mysem_up(int semid, int sem_num){
    if(semctl(semid, sem_num, GETVAL, argument) == 1){
        fprintf(stderr, "[MISSING] UP %d\n", sem_num);
        return;
    }

    argument.val = SEM_MAX; //Set to One (Binary)
    if(semctl(semid, sem_num, SETVAL, argument) == -1){
        perror("[ERROR]: Semaphore Up");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "---- UP %d: [%d] ----\n", semid, sem_num);
}