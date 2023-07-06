#ifndef PRELUDE_H
#define PRELUDE_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t i32;
typedef int64_t i64;

#define OK    0
#define ERROR 1

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

#define EXIT()                                                       \
    do {                                                             \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(ERROR);                                                \
    } while (FALSE)

#define EXIT_IF(condition)             \
    do {                               \
        if (condition) {               \
            fprintf(stderr,            \
                    "%s:%s:%d `%s`\n", \
                    __FILE__,          \
                    __func__,          \
                    __LINE__,          \
                    #condition);       \
            _exit(ERROR);              \
        }                              \
    } while (FALSE)

u32  len(const char*);
Bool eq(const char*, const char*);

#endif
