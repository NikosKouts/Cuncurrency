#ifndef HEADER_H
#define HEADER_H

typedef struct coroutine{
    ucontext_t co;
    char STACK[8192];
}co_t;


int mycoroutines_init(co_t *main);
int mycoroutines_create(co_t *coroutine, void(body)(void *), void *arguments);
int mycoroutines_switchto(co_t *coroutine);
int mycoroutines_destroy(co_t *coroutine);

#endif