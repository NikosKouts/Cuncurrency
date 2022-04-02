#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "header.h"
#include <signal.h>

#define DEFAULT_STACK_SIZE 64
#define DEFAULT_ARGUMENTS 0x1


co_t *curr = NULL, *nxt = NULL;

int mycoroutines_init(co_t *main) {
  if (!getcontext(&main->co)){
    nxt = main;
    return 0;
  }
  else
    exit(0);
}


int mycoroutines_create(co_t *coroutine, void(body)(void *), void *arguments) {
    // Get Context
  if (!getcontext(&coroutine->co)) {
    // Build Context's Memory and Registers
    coroutine->co.uc_stack.ss_sp = coroutine->STACK;
    coroutine->co.uc_stack.ss_size = sizeof(coroutine->STACK);
    coroutine->co.uc_link = &nxt->co;
    
    //Make The Coroutine
    makecontext(&coroutine->co, (void (*)(void)) body, DEFAULT_ARGUMENTS, arguments);
  }
  else
    exit(0);
    
  return 0;
}

int mycoroutines_switchto(co_t *coroutine) {
  
  curr = nxt;
  nxt = coroutine;

  if(swapcontext(&curr->co, &nxt->co))
    exit(0);

  return 0;
}

int mycoroutines_destroy(co_t *coroutine){

  coroutine->co.uc_stack.ss_sp = NULL;

  return 0;
}