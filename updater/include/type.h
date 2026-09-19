#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
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
  uint64_t relative_offset;
  Type type;
  bool is_vt = false;
};

struct Object
{
  std::string name;
  bool is_class;
  bool has_parent;
  std::string parent;
  std::unordered_map<std::string, Field> virtual_functions{};
  std::unordered_map<std::string, Field> fields{};
  size_t rel_size = 0;
  size_t size = 0;
};
