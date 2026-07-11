#pragma once

#define FINLINE __attribute__((always_inline)) inline
#define UNSUPPORTED_OS() static_assert(false, "Unsupported operating system")