
#include "updater.h"
#include "pattern.h"
#include <cstring>

std::vector<int> compile_ida_pattern(const char *pattern)
{
    auto next = pattern;
    std::vector<int> compiled;
    do
    {

        auto tmp = strstr(next, " ");
        if (tmp)
            tmp += 1;

        uint64_t length = 0;
        if (tmp)
        {
            length = (uint64_t)tmp - (uint64_t)next - 1;
        }
        else
        {
            length = strlen(next);
        }

        char number[3] = {0, 0, 0};
        number[0] = next[0];
        if (length > 1)
        {
            number[1] = next[1];
        }

        if (!strcmp(number, "?") || !strcmp(number, "??"))
        {
            compiled.push_back(-1);
        }
        else
        {
            compiled.push_back(strtol(number, NULL, 16));
        }

        next = tmp;
    } while (next);

    return compiled;
}

Status pattern_scan(
    const uint8_t *data,
    size_t size,
    const std::vector<int> &pattern,
    const uint8_t **out)
{
    const uint8_t *found_at = nullptr;
    size_t found_count = 0;
    for (size_t i = 0; i <= size - pattern.size(); ++i)
    {
        bool found = true;

        for (size_t j = 0; j < pattern.size(); ++j)
        {
            if (pattern[j] != -1 && data[i + j] != pattern[j])
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            found_at = (data + i);
            found_count += 1;
        }
    }

    *out = found_at;
    if (found_count > 1)
    {
        return Status::Duplicates;
    }

    if (found_at)
    {
        return Status::Success;
    }

    return Status::NotFound;
}

std::vector<Object*> match_to_object(const ElfInterface& elf, uint8_t *text, Elf64_Shdr text_hdr, std::vector<PatternObject> &patterns)
{
    std::unordered_map<std::string, Object*> matched;
    std::vector<Object*> linear;

    for (auto &pattern_obj : patterns)
    {
        auto obj = new Object{pattern_obj.name, pattern_obj.is_class, pattern_obj.has_parent, pattern_obj.parent};
        LOG(info, "Matching " << pattern_obj.name);

        if (auto sizepattern = pattern_obj.size_pattern)
        {
            LOG(info, "Matching size...");
            auto result = sizepattern->find_result(text, text_hdr);

            obj->size = sizepattern->extractor->extract_validated(elf, result);
            obj->rel_size = obj->size;
            if (pattern_obj.has_parent)
            {
                obj->rel_size -= matched[pattern_obj.parent]->size;
            }
        }

        size_t size = 0;
        if (pattern_obj.has_parent)
        {
            size = matched[pattern_obj.parent]->size;
        }

        LOG(info, "Matching pattern...");
        for (auto pattern : pattern_obj.patterns)
        {
            auto result = pattern->find_result(text, text_hdr);
            auto extracted = pattern->extractor->extract_validated(elf, result);
            if (!extracted)
            {
                LOG(ERROR, "Failed to extract " << obj->name << "." << pattern->name);
                continue;
            }

            obj->fields[pattern->name] = {pattern->name, extracted, extracted - size, pattern->type};
        }
        matched[pattern_obj.name] = obj;
        linear.push_back(obj);
    }
    return linear;
}
