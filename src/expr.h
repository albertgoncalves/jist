#ifndef EXPR_H
#define EXPR_H

#include "inst.h"

typedef enum {
    EXPR_IDENT = 0,
    EXPR_I64,

    EXPR_RET,

    EXPR_LABEL,

    EXPR_LOAD,
    EXPR_STORE,

    EXPR_JMP,
    EXPR_JZ,

    EXPR_LT,
    EXPR_EQ,

    EXPR_AND,

    EXPR_ADD,
} ExprType;

typedef struct Expr Expr;

typedef union {
    const char* as_chars;
    Expr*       as_exprs[2];
    i64         as_i64;
} ExprValue;

struct Expr {
    ExprValue value;
    ExprType  type;
};

#define CAP_EXPRS (1 << 5)
static Expr EXPRS[CAP_EXPRS];
static u32  LEN_EXPRS = 0;

#define CAP_LIST (1 << 4)
static Expr* LIST[CAP_LIST];
static u32   LEN_LIST = 0;

static Expr* expr_alloc(void) {
    EXIT_IF(CAP_EXPRS <= LEN_EXPRS);
    return &EXPRS[LEN_EXPRS++];
}

static void list_push(Expr* expr) {
    EXIT_IF(CAP_LIST <= LEN_LIST);
    LIST[LEN_LIST++] = expr;
}

static void expr_print(Expr expr) {
    switch (expr.type) {
    case EXPR_IDENT: {
        printf("%s", expr.value.as_chars);
        break;
    }
    case EXPR_I64: {
        printf("%ld", expr.value.as_i64);
        break;
    }
    case EXPR_RET: {
        printf("ret()");
        break;
    }
    case EXPR_LABEL: {
        printf("label(%s)", expr.value.as_chars);
        break;
    }
    case EXPR_LOAD: {
        printf("load(%s)", expr.value.as_chars);
        break;
    }
    case EXPR_STORE: {
        printf("store(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    case EXPR_JMP: {
        printf("jmp(");
        expr_print(*expr.value.as_exprs[0]);
        putchar(')');
        break;
    }
    case EXPR_JZ: {
        printf("jz(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    case EXPR_LT: {
        printf("lt(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    case EXPR_EQ: {
        printf("eq(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    case EXPR_AND: {
        printf("and(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    case EXPR_ADD: {
        printf("add(");
        expr_print(*expr.value.as_exprs[0]);
        printf(", ");
        expr_print(*expr.value.as_exprs[1]);
        putchar(')');
        break;
    }
    default: {
        EXIT();
    }
    }
}

static Expr* insts_to_expr(Inst* insts, u32* i, u32 end) {
    EXIT_IF(*i <= end);
    const Inst inst = insts[--(*i)];
    switch (inst.type) {
    case INST_HALT: {
        EXIT();
    }
    case INST_LABEL: {
        Expr* expr = expr_alloc();
        expr->value.as_chars = inst.value.as_chars;
        expr->type = EXPR_LABEL;
        return expr;
    }
    case INST_ALLOC: {
        EXIT();
    }
    case INST_LOAD: {
        Expr* expr = expr_alloc();
        expr->value.as_chars = inst.value.as_chars;
        expr->type = EXPR_LOAD;
        return expr;
    }
    case INST_STORE: {
        Expr* arg = expr_alloc();
        arg->value.as_chars = inst.value.as_chars;
        arg->type = EXPR_IDENT;

        Expr* func = expr_alloc();
        func->value.as_exprs[1] = insts_to_expr(insts, i, end);
        func->value.as_exprs[0] = arg;
        func->type = EXPR_STORE;

        return func;
    }
    case INST_PUSH: {
        Expr* expr = expr_alloc();
        expr->value.as_i64 = inst.value.as_i64;
        expr->type = EXPR_I64;
        return expr;
    }
    case INST_JMP: {
        Expr* arg = expr_alloc();
        arg->value.as_chars = insts[inst.value.as_u32].value.as_chars;
        arg->type = EXPR_IDENT;

        Expr* func = expr_alloc();
        func->value.as_exprs[0] = arg;
        func->type = EXPR_JMP;

        return func;
    }
    case INST_JZ: {
        Expr* arg = expr_alloc();
        arg->value.as_chars = insts[inst.value.as_u32].value.as_chars;
        arg->type = EXPR_IDENT;

        Expr* func = expr_alloc();
        func->value.as_exprs[0] = arg;
        func->value.as_exprs[1] = insts_to_expr(insts, i, end);
        func->type = EXPR_JZ;

        return func;
    }
    case INST_LT: {
        Expr* expr = expr_alloc();
        expr->value.as_exprs[1] = insts_to_expr(insts, i, end);
        expr->value.as_exprs[0] = insts_to_expr(insts, i, end);
        expr->type = EXPR_LT;
        return expr;
    }
    case INST_EQ: {
        Expr* expr = expr_alloc();
        expr->value.as_exprs[1] = insts_to_expr(insts, i, end);
        expr->value.as_exprs[0] = insts_to_expr(insts, i, end);
        expr->type = EXPR_EQ;
        return expr;
    }
    case INST_AND: {
        Expr* expr = expr_alloc();
        expr->value.as_exprs[1] = insts_to_expr(insts, i, end);
        expr->value.as_exprs[0] = insts_to_expr(insts, i, end);
        expr->type = EXPR_AND;
        return expr;
    }
    case INST_ADD: {
        Expr* expr = expr_alloc();
        expr->value.as_exprs[1] = insts_to_expr(insts, i, end);
        expr->value.as_exprs[0] = insts_to_expr(insts, i, end);
        expr->type = EXPR_ADD;
        return expr;
    }
    case INST_PRINTLN_I64: {
        EXIT();
    }
    default: {
        EXIT();
    }
    }
}

static void exprs_parse(Inst* insts, u32 start, u32 end) {
    LEN_EXPRS = 0;
    LEN_LIST = 0;

    {
        Expr* expr = expr_alloc();
        expr->type = EXPR_RET;
        list_push(expr);
    }

    u32 i = end;
    while (start < i) {
        list_push(insts_to_expr(insts, &i, start));
    }
}

static void exprs_show(void) {
    putchar('\n');
    for (u32 i = LEN_LIST;;) {
        if (i == 0) {
            break;
        }
        expr_print(*LIST[--i]);
        putchar('\n');
    }
}

#endif
