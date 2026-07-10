#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Type
{
    std::string type;
    size_t size;
    bool arr = false;
    size_t arr_size = 0;
};

struct Field
{
    std::string name;
    uint64_t offset;
    Type type;
};

struct Object
{
    std::string name;
    std::vector<Field> fields;
};
