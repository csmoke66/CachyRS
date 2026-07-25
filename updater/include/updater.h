#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>

#include "type.h"
#include "elf.h"
#include "pattern.h"
#include "elf_interface.h"

#define LOG(LEVEL, ...) std::cout << "[" << __FUNCTION__ << "]" << "[" << #LEVEL << "] " << __VA_ARGS__ << std::dec << std::endl

enum class Status
{
    Success,
    NotFound,
    Duplicates
};

std::vector<int> compile_ida_pattern(const char *pattern);

Status pattern_scan(
    const uint8_t *data,
    size_t size,
    const std::vector<int> &pattern,
    const uint8_t** out);

std::vector<Object*> match_to_object(const ElfInterface& elf, uint8_t *text, Elf64_Shdr text_hdr, std::vector<PatternObject> &patterns);

std::string compile_object_to_structure(Object object);
std::string compile_objects_to_header(const std::vector<Object*> &objects);