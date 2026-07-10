#pragma once

#define INVALID_OFFSET "Invalid offset"
#define INVALID_SIZE "Invalid size"

#define TOKEN_PASTE(x, y) x ## y
#define CAT(x, y) TOKEN_PASTE(x, y)
#define PAD(size) char CAT(_pad_, __LINE__)[size]

#define FINLINE __attribute__((always_inline)) inline