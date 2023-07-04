#ifndef ASM_H
#define ASM_H

#include "expr.h"

typedef enum {
    ASM_REG_RDI = 0,
    ASM_REG_R8,
} AsmArgReg;

typedef struct {
    AsmArgReg reg;
    i32       offset;
} AsmArgAddr;

typedef union {
    const char* as_chars;
    AsmArgReg   as_reg;
    AsmArgAddr  as_addr;
    i32         as_i32;
} AsmArgValue;

typedef enum {
    ASM_ARG_NONE = 0,
    ASM_ARG_LABEL,
    ASM_ARG_REG,
    ASM_ARG_ADDR,
    ASM_ARG_I32,
} AsmArgType;

typedef struct {
    AsmArgValue value;
    AsmArgType  type;
} AsmArg;

typedef struct {
    const char* name;
    AsmArgReg   reg;
} Local;

typedef enum {
    ASM_NOP = 0,

    ASM_RET,

    ASM_LABEL,

    ASM_MOV,

    ASM_JMP,
    ASM_JNZ,
    ASM_JGE,

    ASM_TEST,
    ASM_CMP,

    ASM_AND,

    ASM_ADD,
} AsmType;

typedef struct {
    AsmArg  args[2];
    AsmType type;
} Asm;

typedef struct {
    const char* label;
    AsmArgReg   reg;
} Pointer;

#define CAP_ASMS (1 << 5)
static Asm ASMS[CAP_ASMS];
static u32 LEN_ASMS = 0;

static AsmArgReg REGS[] = {
    ASM_REG_RDI,
    ASM_REG_R8,
};

#define CAP_REGS (sizeof(REGS) / sizeof(REGS[0]))

static u32 LEN_REGS = 0;

#define CAP_PTRS (1 << 3)
static Pointer PTRS[CAP_PTRS];
static u32     LEN_PTRS = 0;

static Asm* asm_alloc(void) {
    EXIT_IF(CAP_ASMS <= LEN_ASMS);
    return &ASMS[LEN_ASMS++];
}

static AsmArgReg reg_alloc(void) {
    EXIT_IF(CAP_REGS <= LEN_REGS);
    return REGS[LEN_REGS++];
}

static Pointer* ptr_alloc(void) {
    EXIT_IF(CAP_PTRS <= LEN_PTRS);
    return &PTRS[LEN_PTRS++];
}

static void asm_arg_reg_print(AsmArgReg reg) {
    switch (reg) {
    case ASM_REG_RDI: {
        printf("rdi");
        break;
    }
    case ASM_REG_R8: {
        printf("r8");
        break;
    }
    default: {
        EXIT();
    }
    }
}

static void asm_arg_print(AsmArg arg) {
    switch (arg.type) {
    case ASM_ARG_NONE: {
        EXIT();
    }
    case ASM_ARG_LABEL: {
        printf("%s", arg.value.as_chars);
        break;
    }
    case ASM_ARG_REG: {
        asm_arg_reg_print(arg.value.as_reg);
        break;
    }
    case ASM_ARG_ADDR: {
        putchar('[');
        asm_arg_reg_print(arg.value.as_addr.reg);
        if (arg.value.as_addr.offset < 0) {
            printf(" - %d", -arg.value.as_addr.offset);
        } else if (0 < arg.value.as_addr.offset) {
            printf(" + %d", arg.value.as_addr.offset);
        }
        putchar(']');
        break;
    }
    case ASM_ARG_I32: {
        printf("%d", arg.value.as_i32);
        break;
    }
    default: {
        EXIT();
    }
    }
}

static void asm_println(Asm* asm) {
    switch (asm->type) {
    case ASM_NOP: {
        EXIT();
    }
    case ASM_RET: {
        printf("        ret\n");
        break;
    }
    case ASM_LABEL: {
        printf("    %s:\n", asm->args[0].value.as_chars);
        break;
    }
    case ASM_MOV: {
        printf("        mov ");
        asm_arg_print(asm->args[0]);
        printf(", ");
        asm_arg_print(asm->args[1]);
        putchar('\n');
        break;
    }
    case ASM_JMP: {
        printf("        jmp %s\n", asm->args[0].value.as_chars);
        break;
    }
    case ASM_JNZ: {
        printf("        jnz %s\n", asm->args[0].value.as_chars);
        break;
    }
    case ASM_JGE: {
        printf("        jge %s\n", asm->args[0].value.as_chars);
        break;
    }
    case ASM_TEST: {
        printf("        test ");
        asm_arg_print(asm->args[0]);
        printf(", ");
        asm_arg_print(asm->args[1]);
        putchar('\n');
        break;
    }
    case ASM_CMP: {
        printf("        cmp ");
        asm_arg_print(asm->args[0]);
        printf(", ");
        asm_arg_print(asm->args[1]);
        putchar('\n');
        break;
    }
    case ASM_AND: {
        printf("        and ");
        asm_arg_print(asm->args[0]);
        printf(", ");
        asm_arg_print(asm->args[1]);
        putchar('\n');
        break;
    }
    case ASM_ADD: {
        printf("        add ");
        asm_arg_print(asm->args[0]);
        printf(", ");
        asm_arg_print(asm->args[1]);
        putchar('\n');
        break;
    }
    default: {
        EXIT();
    }
    }
}

static void expr_to_asm_arg(Expr* expr, AsmArg* arg) {
    switch (expr->type) {
    case EXPR_I64: {
        EXIT_IF(0xFFFFFFFF < expr->values[0].as_i64);
        arg->value.as_i32 = expr->values[0].as_i32;
        arg->type = ASM_ARG_I32;
        break;
    }
    case EXPR_LOAD: {
        for (u32 i = 0; i < LEN_PTRS; ++i) {
            if (eq(expr->values[0].as_chars, PTRS[i].label)) {
                arg->value.as_addr = (AsmArgAddr){
                    .reg = PTRS[i].reg,
                    .offset = 0,
                };
                arg->type = ASM_ARG_ADDR;
                return;
            }
        }
        EXIT();
    }
    case EXPR_AND: {
        AsmArgReg reg = reg_alloc();
        {
            Asm* asm = asm_alloc();
            asm->args[0].value.as_reg = reg;
            asm->args[0].type = ASM_ARG_REG;
            expr_to_asm_arg(expr->values[0].as_expr, &asm->args[1]);
            asm->type = ASM_MOV;
        }
        {
            Asm* asm = asm_alloc();
            asm->args[0].value.as_reg = reg;
            asm->args[0].type = ASM_ARG_REG;
            expr_to_asm_arg(expr->values[1].as_expr, &asm->args[1]);
            asm->type = ASM_AND;
        }
        arg->value.as_reg = reg;
        arg->type = ASM_ARG_REG;
        return;
    }
    case EXPR_IDENT:
    case EXPR_RET:
    case EXPR_LABEL:
    case EXPR_STORE:
    case EXPR_JMP:
    case EXPR_JZ:
    case EXPR_LT:
    case EXPR_EQ:
    case EXPR_ADD:
    default: {
        EXIT();
    }
    }
}

static void expr_to_asm(Expr* expr) {
    switch (expr->type) {
    case EXPR_IDENT: {
        EXIT();
    }
    case EXPR_I64: {
        EXIT();
    }
    case EXPR_RET: {
        Asm* asm = asm_alloc();
        asm->type = ASM_RET;
        break;
    }
    case EXPR_LABEL: {
        Asm* asm = asm_alloc();
        asm->args[0] = (AsmArg){
            .value = {.as_chars = expr->values[0].as_chars},
            .type = ASM_ARG_LABEL,
        };
        asm->type = ASM_LABEL;
        break;
    }
    case EXPR_LOAD: {
        EXIT();
    }
    case EXPR_STORE: {
        const char* var = expr->values[0].as_chars;
        const Expr* child = expr->values[1].as_expr;

        if (child->type == EXPR_ADD) {
            const Expr* grandchild = child->values[0].as_expr;
            if ((grandchild->type == EXPR_LOAD) &&
                eq(var, grandchild->values[0].as_chars))
            {
                Asm* asm = asm_alloc();
                expr_to_asm_arg(child->values[0].as_expr, &asm->args[0]);
                expr_to_asm_arg(child->values[1].as_expr, &asm->args[1]);
                asm->type = ASM_ADD;
                break;
            }
        }
        EXIT();
    }
    case EXPR_JMP: {
        Asm* asm = asm_alloc();
        asm->args[0] = (AsmArg){
            .value = {.as_chars = expr->values[0].as_chars},
            .type = ASM_ARG_LABEL,
        };
        asm->type = ASM_JMP;
        break;
    }
    case EXPR_JZ: {
        const Expr* child = expr->values[1].as_expr;
        switch (child->type) {
        case EXPR_LT: {
            {
                Asm* asm = asm_alloc();
                expr_to_asm_arg(child->values[0].as_expr, &asm->args[0]);
                expr_to_asm_arg(child->values[1].as_expr, &asm->args[1]);
                asm->type = ASM_CMP;
            }
            {
                Asm* asm = asm_alloc();
                asm->args[0] = (AsmArg){
                    .value = {.as_chars = expr->values[0].as_chars},
                    .type = ASM_ARG_LABEL,
                };
                asm->type = ASM_JGE;
            }
            break;
        }
        case EXPR_EQ: {
            const Expr* grandchild = child->values[1].as_expr;
            if ((grandchild->type == EXPR_I64) &&
                (grandchild->values[0].as_i64) == 0)
            {
                {
                    AsmArg arg = {0};
                    expr_to_asm_arg(child->values[0].as_expr, &arg);

                    Asm* asm = asm_alloc();
                    asm->args[0] = arg;
                    asm->args[1] = arg;
                    asm->type = ASM_TEST;
                }
                {
                    Asm* asm = asm_alloc();
                    asm->args[0] = (AsmArg){
                        .value = {.as_chars = expr->values[0].as_chars},
                        .type = ASM_ARG_LABEL,
                    };
                    asm->type = ASM_JNZ;
                }
                break;
            }
            EXIT();
        }
        case EXPR_IDENT:
        case EXPR_I64:
        case EXPR_RET:
        case EXPR_LABEL:
        case EXPR_LOAD:
        case EXPR_STORE:
        case EXPR_JMP:
        case EXPR_JZ:
        case EXPR_AND:
        case EXPR_ADD:
        default: {
            EXIT();
        }
        }
        break;
    }
    case EXPR_LT: {
        EXIT();
    }
    case EXPR_EQ: {
        EXIT();
    }
    case EXPR_AND: {
        EXIT();
    }
    case EXPR_ADD: {
        EXIT();
    }
    default: {
        EXIT();
    }
    }
}

static void asm_emit(void) {
    LEN_ASMS = 0;
    LEN_REGS = 0;
    LEN_PTRS = 0;

    for (u32 i = 0; i < LEN_ESCAPES; ++i) {
        Pointer* pointer = ptr_alloc();
        pointer->label = ESCAPES[i];
        pointer->reg = reg_alloc();
    }

    const u32 len_regs = LEN_REGS;
    for (u32 i = LEN_LIST; i != 0;) {
        LEN_REGS = len_regs;
        expr_to_asm(LIST[--i]);
    }
}

static void asm_show(void) {
    putchar('\n');
    for (u32 i = 0; i < LEN_ASMS; ++i) {
        asm_println(&ASMS[i]);
    }
}

#endif
