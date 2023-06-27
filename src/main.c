#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

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

#define EXIT()                                              \
    do {                                                    \
        printf("%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(ERROR);                                       \
    } while (FALSE)

#define EXIT_WITH(x)                                                \
    do {                                                            \
        printf("%s:%s:%d `%s`\n", __FILE__, __func__, __LINE__, x); \
        _exit(ERROR);                                               \
    } while (FALSE)

#define EXIT_IF(condition)         \
    do {                           \
        if (condition) {           \
            EXIT_WITH(#condition); \
        }                          \
    } while (FALSE)

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
    u64         as_u64;
    i64         as_i64;
} Value;

typedef struct {
    Value    value;
    InstType type;
} Inst;

typedef struct {
    const char* key;
    Value       value;
} KeyValue;

STATIC_ASSERT(sizeof(void*) == sizeof(i64));

#define CAP_STACK (1 << 3)
static Value STACK[CAP_STACK];
static u32   LEN_STACK = 0;

#define CAP_LOCALS (1 << 3)
static KeyValue LOCALS[CAP_LOCALS];
static u32      LEN_LOCALS = 0;

#define CAP_LABELS (1 << 3)
static KeyValue LABELS[CAP_LABELS];
static u32      LEN_LABELS = 0;

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

static u32 HISTORY[LEN_INSTS] = {0};

static void println_inst(Inst inst) {
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
        printf("        jmp         %s\n",
               INSTS[inst.value.as_u64 - 1].value.as_chars);
        break;
    }
    case INST_JZ: {
        printf("        jz          %s\n",
               INSTS[inst.value.as_u64 - 1].value.as_chars);
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

static void push_stack(Value value) {
    EXIT_IF(CAP_STACK <= LEN_STACK);
    STACK[LEN_STACK++] = value;
}

static Value pop_stack(void) {
    EXIT_IF(LEN_STACK == 0);
    return STACK[--LEN_STACK];
}

static void push_local(const char* key, Value value) {
    EXIT_IF(CAP_LOCALS <= LEN_LOCALS);
    LOCALS[LEN_LOCALS++] = (KeyValue){
        .key = key,
        .value = value,
    };
}

static u32 len(const char* string) {
    u32 i = 0;
    for (; string[i] != '\0'; ++i) {
    }
    return i;
}

static Bool eq(const char* a, const char* b) {
    u32 n = len(a);
    if (n != len(b)) {
        return FALSE;
    }
    for (u32 i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

static KeyValue* find_local(const char* key) {
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

static void push_label(const char* key, Value value) {
    EXIT_IF(CAP_LABELS <= LEN_LABELS);
    LABELS[LEN_LABELS++] = (KeyValue){
        .key = key,
        .value = value,
    };
}

static KeyValue* find_label(const char* key) {
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

static void run(void) {
    for (u32 i = 0; i < LEN_INSTS;) {
        const Inst inst = INSTS[i++];
        if (inst.type == INST_LABEL) {
            push_label(inst.value.as_chars, (Value){.as_u64 = i});
        }
    }
    for (u32 i = 0; i < LEN_INSTS; ++i) {
        const Inst inst = INSTS[i];
        if ((inst.type == INST_JMP) || (inst.type == INST_JZ)) {
            INSTS[i].value = find_label(inst.value.as_chars)->value;
        }
    }

    u64 i = 0;
    for (;;) {
        const Inst inst = INSTS[i];

        switch (inst.type) {
        case INST_HALT: {
            return;
        }
        case INST_LABEL: {
            ++i;
            break;
        }
        case INST_ALLOC: {
            push_local(inst.value.as_chars, pop_stack());
            ++i;
            break;
        }
        case INST_LOAD: {
            const KeyValue* local = find_local(inst.value.as_chars);
            push_stack(local->value);
            ++i;
            break;
        }
        case INST_STORE: {
            KeyValue* local = find_local(inst.value.as_chars);
            local->value = pop_stack();
            ++i;
            break;
        }
        case INST_PUSH: {
            push_stack(inst.value);
            ++i;
            break;
        }
        case INST_JMP: {
            i = inst.value.as_u64;
            ++HISTORY[i];
            break;
        }
        case INST_JZ: {
            if (pop_stack().as_u64 == 0) {
                i = inst.value.as_u64;
                ++HISTORY[i];
            } else {
                ++i;
            }
            break;
        }
        case INST_LT: {
            const i64 r = pop_stack().as_i64;
            const i64 l = pop_stack().as_i64;
            push_stack((Value){.as_u64 = l < r});
            ++i;
            break;
        }
        case INST_EQ: {
            const u64 r = pop_stack().as_u64;
            const u64 l = pop_stack().as_u64;
            push_stack((Value){.as_u64 = l == r});
            ++i;
            break;
        }
        case INST_AND: {
            const u64 r = pop_stack().as_u64;
            const u64 l = pop_stack().as_u64;
            push_stack((Value){.as_u64 = l & r});
            ++i;
            break;
        }
        case INST_ADD: {
            const i64 r = pop_stack().as_i64;
            const i64 l = pop_stack().as_i64;
            push_stack((Value){.as_i64 = l + r});
            ++i;
            break;
        }
        case INST_PRINTLN_I64: {
            printf("%lu\n", pop_stack().as_i64);
            ++i;
            break;
        }
        default: {
            EXIT();
        }
        }
    }
}

/*
    i32 x = 0;
    while (x < 1000) {
        if ((x & 1) == 0) {
            x += 29;
        } else {
            x -= 3;
        }
    }
    printf("%d\n", x);
*/
/*
        push 0              [0]
        alloc x             []
    while_start:
        load x              [x]
        push 1000           [x, 1000]
        lt                  [x<1000]
        jz while_end        []

        load x              [x]
        push 1              [x, 1]
        and                 [x&1]
        push 0              [x&1, 0]
        eq                  [(x&1)==0]

        jz if_else          []

        load x              [x]
        push 29             [x, 29]
        add                 [x+29]
        store x             []
        jmp if_end

    if_else:
        load x              [x]
        push -3             [x, -3]
        add                 [x+(-3)]
        store x             []

    if_end:
        jmp while_start

    while_end:
        load x              [x]
        println_i64         []

        halt                []
*/

// NOTE: See `https://www.cs.cmu.edu/~rjsimmon/15411-f15/lec/10-ssa.pdf`.
// NOTE: See `http://troubles.md/wasm-is-not-a-stack-machine/`.

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

    run();

    putchar('\n');
    for (u32 i = 0; i < LEN_INSTS; ++i) {
        if (INSTS[i].type == INST_LABEL) {
            putchar('\n');
        } else if ((i != 0) && ((INSTS[i - 1].type == INST_JMP) ||
                                (INSTS[i - 1].type == INST_JZ)))
        {
            putchar('\n');
        }

        if (INSTS[i].type == INST_LABEL) {
            printf("        ");
        } else if (HISTORY[i] == 0) {
            printf("       |");
        } else {
            printf("%4u --+", HISTORY[i]);
        }

        println_inst(INSTS[i]);
    }

    return OK;
}
