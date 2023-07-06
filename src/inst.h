#ifndef INST_H
#define INST_H

#include "prelude.h"

typedef enum {
    INST_HALT = 0,

    INST_LABEL,

    INST_ALLOC,
    INST_LOAD,
    INST_STORE,

    INST_PUSH,

    INST_JMP,
    INST_JZ,

    INST_LT,
    INST_EQ,

    INST_AND,

    INST_ADD,

    INST_PRINTLN_I64,
} InstType;

typedef union {
    const char* as_chars;
    u32         as_u32;
    u64         as_u64;
    i64         as_i64;
} InstValue;

STATIC_ASSERT(sizeof(InstValue) == sizeof(i64));

typedef struct {
    InstValue value;
    InstType  type;
} Inst;

void insts_setup(Inst*, u32);
void insts_run(const Inst*);
void insts_show(void);

#define CAP_INSTS (1 << 5)

extern u32 JUMPS[CAP_INSTS];
extern u32 LOOPS[CAP_INSTS];

#endif
