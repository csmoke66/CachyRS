#include "cachy.h"
#include <thread>

void init_async() 
{
    RS.init();
}

__attribute__((constructor)) void entry()
{
    std::thread thr(init_async);
    thr.detach();
}