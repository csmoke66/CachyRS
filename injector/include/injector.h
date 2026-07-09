#pragma once
#include "executor.h"
#include <list>
#include <vector>
#include <string>

class Injector
{
private:
    pid_t pid;
    std::list<Executor *> executors;

public:
    ~Injector();

public:
    bool open(pid_t pid);
    int32_t inject(const std::string &library);
};
