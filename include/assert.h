#ifndef ZK_ASSERT_H
#define ZK_ASSERT_H

#ifdef ZK_ENABLE_ASSERTS
#include <stdlib.h>
#define ZK_ASSERT(condition) do { if (!(condition)) { fprintf(stderr, "Assertion failed: %s:%d: %s", __FILE__, __LINE__, #condition);\
    exit(EXIT_FAILURE); } } while(0)
#else
#define ZK_ASSERT(condition)
#endif

#endif
