#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>

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
    uint64_t relative_offset;
    Type type;
};

struct Object
{
    std::string name;
    bool is_class;
    bool has_parent;
    std::string parent;
    std::map<std::string, Field> fields;
};
