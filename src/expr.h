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

struct Expr {
    union {
        const char* as_chars;
        Expr*       as_expr;
        i64         as_i64;
        i32         as_i32;
    } values[2];
    ExprType type;
};

void exprs_parse(const Inst*, u32, u32);
void exprs_show(void);

#define CAP_ESCAPES (1 << 3)
#define CAP_LIST    (1 << 4)

extern const char* ESCAPES[CAP_ESCAPES];
extern u32         LEN_ESCAPES;

extern const Expr* LIST[CAP_LIST];
extern u32         LEN_LIST;

#endif
