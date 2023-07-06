#include "expr.h"

#define CAP_EXPRS (1 << 5)
static Expr EXPRS[CAP_EXPRS];
static u32  LEN_EXPRS = 0;

static Expr* expr_alloc(void) {
    EXIT_IF(CAP_EXPRS <= LEN_EXPRS);
    return &EXPRS[LEN_EXPRS++];
}

static void escape_push(const char* escape) {
    for (u32 j = 0; j < LEN_ESCAPES; ++j) {
        if (eq(escape, ESCAPES[j])) {
            return;
        }
    }
    EXIT_IF(CAP_ESCAPES <= LEN_ESCAPES);
    ESCAPES[LEN_ESCAPES++] = escape;
}

static void list_push(const Expr* expr) {
    EXIT_IF(CAP_LIST <= LEN_LIST);
    LIST[LEN_LIST++] = expr;
}

static void expr_print(Expr expr) {
    switch (expr.type) {
    case EXPR_IDENT: {
        printf("%s", expr.values[0].as_chars);
        break;
    }
    case EXPR_I64: {
        printf("%ld", expr.values[0].as_i64);
        break;
    }
    case EXPR_RET: {
        printf("ret()");
        break;
    }
    case EXPR_LABEL: {
        printf("label(%s)", expr.values[0].as_chars);
        break;
    }
    case EXPR_LOAD: {
        printf("load(%s)", expr.values[0].as_chars);
        break;
    }
    case EXPR_STORE: {
        printf("store(%s, ", expr.values[0].as_chars);
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    case EXPR_JMP: {
        printf("jmp(%s)", expr.values[0].as_chars);
        break;
    }
    case EXPR_JZ: {
        printf("jz(%s, ", expr.values[0].as_chars);
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    case EXPR_LT: {
        printf("lt(");
        expr_print(*expr.values[0].as_expr);
        printf(", ");
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    case EXPR_EQ: {
        printf("eq(");
        expr_print(*expr.values[0].as_expr);
        printf(", ");
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    case EXPR_AND: {
        printf("and(");
        expr_print(*expr.values[0].as_expr);
        printf(", ");
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    case EXPR_ADD: {
        printf("add(");
        expr_print(*expr.values[0].as_expr);
        printf(", ");
        expr_print(*expr.values[1].as_expr);
        putchar(')');
        break;
    }
    default: {
        EXIT();
    }
    }
}

static Expr* insts_to_expr(const Inst* insts, u32* i, u32 end) {
    EXIT_IF(*i <= end);
    const Inst inst = insts[--(*i)];
    switch (inst.type) {
    case INST_HALT: {
        EXIT();
    }
    case INST_LABEL: {
        Expr* expr = expr_alloc();
        expr->values[0].as_chars = inst.value.as_chars;
        expr->type = EXPR_LABEL;
        return expr;
    }
    case INST_ALLOC: {
        EXIT();
    }
    case INST_LOAD: {
        escape_push(inst.value.as_chars);

        Expr* expr = expr_alloc();
        expr->values[0].as_chars = inst.value.as_chars;
        expr->type = EXPR_LOAD;
        return expr;
    }
    case INST_STORE: {
        escape_push(inst.value.as_chars);

        Expr* expr = expr_alloc();
        expr->values[0].as_chars = inst.value.as_chars;
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->type = EXPR_STORE;
        return expr;
    }
    case INST_PUSH: {
        Expr* expr = expr_alloc();
        expr->values[0].as_i64 = inst.value.as_i64;
        expr->type = EXPR_I64;
        return expr;
    }
    case INST_JMP: {
        Expr* expr = expr_alloc();
        expr->values[0].as_chars = insts[inst.value.as_u32].value.as_chars;
        expr->type = EXPR_JMP;
        return expr;
    }
    case INST_JZ: {
        Expr* expr = expr_alloc();
        expr->values[0].as_chars = insts[inst.value.as_u32].value.as_chars;
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->type = EXPR_JZ;
        return expr;
    }
    case INST_LT: {
        Expr* expr = expr_alloc();
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->values[0].as_expr = insts_to_expr(insts, i, end);
        expr->type = EXPR_LT;
        return expr;
    }
    case INST_EQ: {
        Expr* expr = expr_alloc();
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->values[0].as_expr = insts_to_expr(insts, i, end);
        expr->type = EXPR_EQ;
        return expr;
    }
    case INST_AND: {
        Expr* expr = expr_alloc();
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->values[0].as_expr = insts_to_expr(insts, i, end);
        expr->type = EXPR_AND;
        return expr;
    }
    case INST_ADD: {
        Expr* expr = expr_alloc();
        expr->values[1].as_expr = insts_to_expr(insts, i, end);
        expr->values[0].as_expr = insts_to_expr(insts, i, end);
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

void exprs_parse(const Inst* insts, u32 start, u32 end) {
    LEN_EXPRS = 0;
    LEN_LIST = 0;
    LEN_ESCAPES = 0;

    {
        Expr* expr = expr_alloc();
        expr->type = EXPR_RET;
        list_push(expr);
    }

    for (u32 i = end; start < i;) {
        list_push(insts_to_expr(insts, &i, start));
    }
}

void exprs_show(void) {
    putchar('\n');
    for (u32 i = LEN_LIST; i != 0;) {
        expr_print(*LIST[--i]);
        putchar('\n');
    }
}
