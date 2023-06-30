#include "inst.h"

// NOTE: See `https://www.cs.cmu.edu/~rjsimmon/15411-f15/lec/10-ssa.pdf`.
// NOTE: See `http://troubles.md/wasm-is-not-a-stack-machine/`.

static Inst INSTS[] = {
    INST_I64(INST_PUSH, 0),
    INST_CHARS(INST_ALLOC, "x"),

    INST_CHARS(INST_LABEL, "while_start"),
    INST_CHARS(INST_LOAD, "x"),
    INST_I64(INST_PUSH, 1000),
    INST_EMPTY(INST_LT),
    INST_CHARS(INST_JZ, "while_end"),

    INST_CHARS(INST_LOAD, "x"),
    INST_I64(INST_PUSH, 1),
    INST_EMPTY(INST_AND),
    INST_I64(INST_PUSH, 0),
    INST_EMPTY(INST_EQ),

    INST_CHARS(INST_JZ, "if_else"),

    INST_CHARS(INST_LOAD, "x"),
    INST_I64(INST_PUSH, 29),
    INST_EMPTY(INST_ADD),
    INST_CHARS(INST_STORE, "x"),
    INST_CHARS(INST_JMP, "if_end"),

    INST_CHARS(INST_LABEL, "if_else"),
    INST_CHARS(INST_LOAD, "x"),
    INST_I64(INST_PUSH, -3),
    INST_EMPTY(INST_ADD),
    INST_CHARS(INST_STORE, "x"),

    INST_CHARS(INST_LABEL, "if_end"),
    INST_CHARS(INST_JMP, "while_start"),

    INST_CHARS(INST_LABEL, "while_end"),
    INST_CHARS(INST_LOAD, "x"),
    INST_EMPTY(INST_PRINTLN_I64),

    INST_EMPTY(INST_HALT),
};

#define LEN_INSTS (sizeof(INSTS) / sizeof(INSTS[0]))

STATIC_ASSERT(LEN_INSTS <= CAP_INSTS);

i32 main(void) {
    printf("\n"
           "sizeof(Bool)     : %zu\n"
           "sizeof(Value)    : %zu\n"
           "sizeof(KeyValue) : %zu\n"
           "sizeof(Inst)     : %zu\n"
           "sizeof(STACK)    : %zu\n"
           "sizeof(LOCALS)   : %zu\n"
           "sizeof(LABELS)   : %zu\n"
           "\n",
           sizeof(Bool),
           sizeof(Value),
           sizeof(KeyValue),
           sizeof(Inst),
           sizeof(STACK),
           sizeof(LOCALS),
           sizeof(LABELS));

    insts_setup(INSTS, LEN_INSTS);
    insts_run(INSTS);
    insts_show(INSTS, LEN_INSTS);

    return OK;
}
