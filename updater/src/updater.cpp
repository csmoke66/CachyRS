
#include "updater.h"
#include "pattern.h"
#include <cstdlib>
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
      length = static_cast<uint64_t>(tmp - next - 1);
    }
    else
    {
      length = std::strlen(next);
    }

    char number[3] = { 0, 0, 0 };
    number[0] = next[0];
    if (length > 1)
    {
      number[1] = next[1];
    }

    if (std::strcmp(number, "?") == 0 || std::strcmp(number, "??") == 0)
    {
      compiled.push_back(-1);
    }
    else
    {
      compiled.push_back(std::strtol(number, nullptr, 16));
    }

    next = tmp;
  }
  while (next);

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

std::vector<Object *> match_to_object(const ElfInterface &elf, uint8_t *text, Elf64_Shdr text_hdr, std::vector<PatternObject> &patterns)
{
  std::unordered_map<std::string, Object *> matched;
  std::vector<Object *> linear;

  for (auto &pattern_obj : patterns)
  {
    auto obj = new Object{ pattern_obj.name, pattern_obj.is_class, pattern_obj.has_parent, pattern_obj.parent };
    LOG(INFO, "Matching " << pattern_obj.name);

    if (auto sizepattern = pattern_obj.size_pattern)
    {
      auto result = sizepattern->find_result(text, text_hdr);
      if (!result)
      {
        LOG(ERROR, "Failed to find size for '" << sizepattern->name << "'");
      }
      else
      {
        obj->size = sizepattern->extractor->extract_validated(elf, result);
      }
    }

    // Field offsets are relative to the end of the parent subobject (or 0).
    // A root class that declares virtuals also has a vptr before its fields.
    size_t field_base = 0;
    if (pattern_obj.has_parent)
    {
      auto parent_matched = matched.find(pattern_obj.parent);
      if (parent_matched != matched.end())
      {
        field_base = parent_matched->second->size;
      }
      else
      {
        LOG(ERROR, "Parent '" << pattern_obj.parent << "' not matched before '" << pattern_obj.name << "'");
      }
    }

    bool saw_vtf = false;
    for (auto pattern : pattern_obj.patterns)
    {
      auto result = pattern->find_result(text, text_hdr);
      auto extracted = pattern->extractor->extract_validated(elf, result);
      if (!extracted)
      {
        LOG(ERROR, "Failed to extract " << obj->name << "." << pattern->name);
        continue;
      }

      if (pattern->is_vtf)
      {
        obj->virtual_functions[pattern->name] = { pattern->name, extracted, extracted, pattern->type, true };

        // One vptr on the object that introduces the vtable — not on every derived class.
        if (!saw_vtf && !pattern_obj.has_parent)
        {
          field_base += sizeof(void *);
        }
        saw_vtf = true;
      }
      else
      {
        if (extracted < field_base)
        {
          LOG(ERROR, "Field " << obj->name << "." << pattern->name << " offset 0x" << std::hex << extracted
                              << " is before field base 0x" << field_base);
        }
        obj->fields[pattern->name] = { pattern->name, extracted, extracted - field_base, pattern->type };
      }
    }

    for (auto pattern : pattern_obj.patterns)
    {
      if (pattern->is_vtf)
      {
        continue;
      }

      auto it = obj->fields.find(pattern->name);
      if (it != obj->fields.end())
      {
        obj->size = std::max(obj->size, static_cast<size_t>(it->second.offset + pattern->type.size));
      }
    }

    if (saw_vtf && !pattern_obj.has_parent)
    {
      obj->size = std::max(obj->size, sizeof(void *));
    }

    // Derived / post-vptr payload size used for trailing PAD emission.
    if (obj->size < field_base)
    {
      LOG(ERROR, "Object '" << obj->name << "' size 0x" << std::hex << obj->size
                            << " is smaller than field base 0x" << field_base);
      obj->rel_size = 0;
    }
    else
    {
      obj->rel_size = obj->size - field_base;
    }

    matched[pattern_obj.name] = obj;
    linear.push_back(obj);
  }
  return linear;
}
