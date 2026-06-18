#pragma once

// TODO: more detail assert options
#ifdef LUMEN_DEBUG
#define ASSERT(statement) assert(statement)
#define STATIC_ASSERT(statement, ...) static_assert(statement, __VA_ARGS__)
#else
#define ASSERT(statement)
#define STATIC_ASSERT(statement, ...)
#endif