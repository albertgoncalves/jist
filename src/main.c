#include "asm.h"

// NOTE: See `https://www.cs.cmu.edu/~rjsimmon/15411-f15/lec/10-ssa.pdf`.
// NOTE: See `http://troubles.md/wasm-is-not-a-stack-machine/`.

u32 JUMPS[CAP_INSTS] = {0};
u32 LOOPS[CAP_INSTS] = {0};

const char* ESCAPES[CAP_ESCAPES];
u32         LEN_ESCAPES = 0;

const Expr* LIST[CAP_LIST];
u32         LEN_LIST = 0;

#define INST_EMPTY(inst_type) ((Inst){.type = inst_type})
#define INST_I64(inst_type, inst_arg) \
    ((Inst){.type = inst_type, .value = {.as_i64 = inst_arg}})
#define INST_CHARS(inst_type, inst_arg) \
    ((Inst){.type = inst_type, .value = {.as_chars = inst_arg}})

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

i32 main(void) {
    insts_setup(INSTS, LEN_INSTS);
    insts_run(INSTS);
    insts_show();

    for (u32 i = 0; i < LEN_INSTS; ++i) {
        if (LOOPS[i] == 0) {
            continue;
        }

        const u32 start = i;
        const u32 end = LOOPS[i] + 2;
        EXIT_IF(LEN_INSTS < end);

        printf("\n%u -> %u\n", start, end);

        exprs_parse(INSTS, start, end);
        exprs_show();

        asm_emit();
        asm_show();

        void* func = asm_jit();

        i64 x = 0;
        ((void (*)(i64*))func)(&x);
        printf("%ld\n", x);
    }

    return OK;
}
