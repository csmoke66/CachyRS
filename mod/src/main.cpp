#include "cachy.h"

void* init_async(void* arg) 
{
    RS.init();
    return nullptr;
}

__attribute__((constructor)) void entry()
{
    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, init_async, nullptr);
}