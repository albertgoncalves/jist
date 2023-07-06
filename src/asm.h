#ifndef ASM_H
#define ASM_H

#include "expr.h"

void  asm_emit(void);
void* asm_jit(void);
void  asm_show(void);

extern const char* ESCAPES[CAP_ESCAPES];
extern u32         LEN_ESCAPES;

extern const Expr* LIST[CAP_LIST];
extern u32         LEN_LIST;

#endif
