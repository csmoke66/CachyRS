#pragma once
#include <cstdint>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

uint64_t getlibcaddr(pid_t pid);
uint64_t get_function_addr(const std::string &name);
uint64_t get_function_addr_remote(pid_t pid);