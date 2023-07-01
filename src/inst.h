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

typedef struct {
    const char* key;
    InstValue   value;
} KeyValue;

typedef struct {
    Inst* insts;
    u32   len;
} Block;

#define CAP_STACK (1 << 3)
static InstValue STACK[CAP_STACK];
static u32       LEN_STACK = 0;

#define CAP_LOCALS (1 << 3)
static KeyValue LOCALS[CAP_LOCALS];
static u32      LEN_LOCALS = 0;

#define CAP_LABELS (1 << 3)
static KeyValue LABELS[CAP_LABELS];
static u32      LEN_LABELS = 0;

#define CAP_BLOCKS (1 << 3)
static Block BLOCKS[CAP_BLOCKS];
static u32   LEN_BLOCKS = 0;

#define CAP_ARGS (1 << 3)
static const char* ARGS[CAP_ARGS];
static u32         LEN_ARGS = 0;

#define CAP_INSTS (1 << 5)
STATIC_ASSERT(CAP_INSTS <= 0xFFFFFFFF);

static u32 JUMPS[CAP_INSTS] = {0};
static u32 LOOPS[CAP_INSTS] = {0};

#define INST_EMPTY(inst_type) ((Inst){.type = inst_type})
#define INST_I64(inst_type, inst_arg) \
    ((Inst){.type = inst_type, .value = {.as_i64 = inst_arg}})
#define INST_CHARS(inst_type, inst_arg) \
    ((Inst){.type = inst_type, .value = {.as_chars = inst_arg}})

static void stack_push(InstValue value) {
    EXIT_IF(CAP_STACK <= LEN_STACK);
    STACK[LEN_STACK++] = value;
}

static InstValue stack_pop(void) {
    EXIT_IF(LEN_STACK == 0);
    return STACK[--LEN_STACK];
}

static void local_push(const char* key, InstValue value) {
    EXIT_IF(CAP_LOCALS <= LEN_LOCALS);
    LOCALS[LEN_LOCALS++] = (KeyValue){
        .key = key,
        .value = value,
    };
}

static KeyValue* local_find(const char* key) {
    EXIT_IF(LEN_LOCALS == 0);
    for (u32 i = LEN_LOCALS;;) {
        KeyValue* local = &LOCALS[--i];
        if (eq(key, local->key)) {
            return local;
        }
        if (i == 0) {
            EXIT();
        }
    }
}

static void label_push(const char* key, InstValue value) {
    EXIT_IF(CAP_LABELS <= LEN_LABELS);
    LABELS[LEN_LABELS++] = (KeyValue){
        .key = key,
        .value = value,
    };
}

static KeyValue* label_find(const char* key) {
    EXIT_IF(LEN_LABELS == 0);
    for (u32 i = LEN_LABELS;;) {
        KeyValue* label = &LABELS[--i];
        if (eq(key, label->key)) {
            return label;
        }
        if (i == 0) {
            EXIT();
        }
    }
}

static Block* block_alloc(void) {
    EXIT_IF(CAP_BLOCKS <= LEN_BLOCKS);
    return &BLOCKS[LEN_BLOCKS++];
}

static void arg_push(const char* arg) {
    for (u32 j = 0; j < LEN_ARGS; ++j) {
        if (eq(arg, ARGS[j])) {
            return;
        }
    }
    EXIT_IF(CAP_ARGS <= LEN_ARGS);
    ARGS[LEN_ARGS++] = arg;
}

static void inst_println(Inst inst) {
    switch (inst.type) {
    case INST_HALT: {
        printf("        halt\n");
        break;
    }
    case INST_LABEL: {
        printf("    %s:\n", inst.value.as_chars);
        break;
    }
    case INST_ALLOC: {
        printf("        alloc       %s\n", inst.value.as_chars);
        break;
    }
    case INST_LOAD: {
        printf("        load        %s\n", inst.value.as_chars);
        break;
    }
    case INST_STORE: {
        printf("        store       %s\n", inst.value.as_chars);
        break;
    }
    case INST_PUSH: {
        printf("        push        %ld\n", inst.value.as_i64);
        break;
    }
    case INST_JMP: {
        printf("        jmp         %u\n", inst.value.as_u32);
        break;
    }
    case INST_JZ: {
        printf("        jz          %u\n", inst.value.as_u32);
        break;
    }
    case INST_LT: {
        printf("        lt\n");
        break;
    }
    case INST_EQ: {
        printf("        eq\n");
        break;
    }
    case INST_AND: {
        printf("        and\n");
        break;
    }
    case INST_ADD: {
        printf("        add\n");
        break;
    }
    case INST_PRINTLN_I64: {
        printf("        println_i64\n");
        break;
    }
    default: {
        EXIT();
    }
    }
}

static void insts_setup(Inst* insts, u32 len_insts) {
    for (u32 i = 0; i < len_insts;) {
        Block* block = block_alloc();
        block->insts = &insts[i];
        u32 j = i + 1;
        for (; j < len_insts; ++j) {
            const Inst inst = insts[j];
            if (inst.type == INST_LABEL) {
                break;
            }
            if ((inst.type == INST_JMP) || (inst.type == INST_JZ)) {
                ++j;
                break;
            }
        }
        block->len = j - i;
        i = j;
    }
    for (u32 i = 0; i < len_insts; ++i) {
        const Inst inst = insts[i];
        if (inst.type == INST_LABEL) {
            label_push(inst.value.as_chars, (InstValue){.as_u32 = i});
        }
    }
    for (u32 i = 0; i < len_insts; ++i) {
        const Inst inst = insts[i];
        if ((inst.type == INST_JMP) || (inst.type == INST_JZ)) {
            insts[i].value = label_find(inst.value.as_chars)->value;
        }
    }
}

static void insts_run(const Inst* insts) {
    u32 i = 0;
    for (;;) {
        const Inst inst = insts[i];
        switch (inst.type) {
        case INST_HALT: {
            return;
        }
        case INST_LABEL: {
            ++i;
            break;
        }
        case INST_ALLOC: {
            local_push(inst.value.as_chars, stack_pop());
            ++i;
            break;
        }
        case INST_LOAD: {
            const KeyValue* local = local_find(inst.value.as_chars);
            stack_push(local->value);
            ++i;
            break;
        }
        case INST_STORE: {
            KeyValue* local = local_find(inst.value.as_chars);
            local->value = stack_pop();
            ++i;
            break;
        }
        case INST_PUSH: {
            stack_push(inst.value);
            ++i;
            break;
        }
        case INST_JMP: {
            ++JUMPS[inst.value.as_u32];
            if (inst.value.as_u32 < i) {
                if (LOOPS[inst.value.as_u32] == 0) {
                    LOOPS[inst.value.as_u32] = i;
                } else {
                    EXIT_IF(LOOPS[inst.value.as_u32] != i);
                }
            }
            i = inst.value.as_u32;
            break;
        }
        case INST_JZ: {
            if (stack_pop().as_u64 == 0) {
                ++JUMPS[inst.value.as_u32];
                if (inst.value.as_u32 < i) {
                    if (LOOPS[inst.value.as_u32] == 0) {
                        LOOPS[inst.value.as_u32] = i;
                    } else {
                        EXIT_IF(LOOPS[inst.value.as_u32] != i);
                    }
                }
                i = inst.value.as_u32;
            } else {
                ++i;
            }
            break;
        }
        case INST_LT: {
            const i64 r = stack_pop().as_i64;
            const i64 l = stack_pop().as_i64;
            stack_push((InstValue){.as_u64 = l < r});
            ++i;
            break;
        }
        case INST_EQ: {
            const u64 r = stack_pop().as_u64;
            const u64 l = stack_pop().as_u64;
            stack_push((InstValue){.as_u64 = l == r});
            ++i;
            break;
        }
        case INST_AND: {
            const u64 r = stack_pop().as_u64;
            const u64 l = stack_pop().as_u64;
            stack_push((InstValue){.as_u64 = l & r});
            ++i;
            break;
        }
        case INST_ADD: {
            const i64 r = stack_pop().as_i64;
            const i64 l = stack_pop().as_i64;
            stack_push((InstValue){.as_i64 = l + r});
            ++i;
            break;
        }
        case INST_PRINTLN_I64: {
            printf("%lu\n", stack_pop().as_i64);
            ++i;
            break;
        }
        default: {
            EXIT();
        }
        }
    }
}

static void insts_trace(const Inst* insts, u32 start, u32 end) {
    LEN_ARGS = 0;

    printf("%u -> %u:\n", start, end);
    for (u32 i = start; i < end; ++i) {
        const Inst inst = insts[i];
        if ((inst.type == INST_LOAD) || (inst.type == INST_STORE)) {
            arg_push(inst.value.as_chars);
        }
        inst_println(inst);
    }
    printf("        [ ret ]\n\n");

    for (u32 i = 0; i < LEN_ARGS; ++i) {
        printf("%s\n", ARGS[i]);
    }
}

static void insts_show(const Inst* insts, u32 len_insts) {
    u32 l = 0;
    for (u32 i = 0; i < LEN_BLOCKS; ++i) {
        const Block block = BLOCKS[i];
        putchar('\n');
        u32 j = 0;
        for (; j < block.len; ++j, ++l) {
            printf("%3u - ", l);
            if (JUMPS[l] != 0) {
                printf("%4u --+", JUMPS[l]);
            } else {
                printf("       |");
            }
            inst_println(block.insts[j]);
        }
    }

    putchar('\n');
    for (u32 i = 0; i < len_insts; ++i) {
        if (LOOPS[i] != 0) {
            const u32 start = i;
            const u32 end = LOOPS[i] + 2;
            EXIT_IF(len_insts < end);
            insts_trace(insts, start, end);
        }
    }
}

#endif
