#pragma once
#include <string>
#include <vector>

#include "type.h"
#include "elf.h"
#include "pattern.h"

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

std::vector<Object> match_to_object(uint8_t *text, Elf64_Shdr text_hdr, std::vector<PatternObject> &patterns);

std::string compile_object_to_structure(Object object);
std::string compile_objects_to_header(const std::vector<Object> &objects);