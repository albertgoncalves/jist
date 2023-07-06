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
    ASM_ARG_LABEL = 1 << 0,
    ASM_ARG_REG = 1 << 1,
    ASM_ARG_ADDR = 1 << 2,
    ASM_ARG_I32 = 1 << 3,
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

typedef struct {
    const char* label;
    u8*         bytes;
} Label;

typedef struct {
    const char* label;
    u8*         bytes;
} Patch;

void  asm_emit(void);
void* asm_jit(void);
void  asm_show(void);

extern const char* ESCAPES[CAP_ESCAPES];
extern u32         LEN_ESCAPES;

extern const Expr* LIST[CAP_LIST];
extern u32         LEN_LIST;

#endif
