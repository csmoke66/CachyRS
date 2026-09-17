#pragma once
#include <cstddef>
#include <cstdint>

#define INVALID_OFFSET "Invalid offset"
#define INVALID_SIZE "Invalid size"

#define TOKEN_PASTE(x, y) x##y
#define CAT(x, y) TOKEN_PASTE(x, y)
#define PAD(size) char CAT(_pad_, __LINE__)[size]
#define PAD_VT() virtual void CAT(_pad_, __LINE__)() = 0

#ifndef FINLINE
#define FINLINE __attribute__((always_inline)) inline
#endif

#ifndef off
#define off(t, f) offsetof(t, f)
#endif
