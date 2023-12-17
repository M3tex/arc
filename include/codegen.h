#ifndef _CODEGEN_HEADER
#define _CODEGEN_HEADER


#include "ast.h"
#include "codegen.h"
#include "ram_os.h"
#include <stdio.h>


/* Coût des opérations sur la pile (en nb d'instructions RAM) */
#define PUSH_COST 2
#define POP_COST 1
#define PEEK_COST 1


/* Utilisé dans `add_instr` */
typedef enum {
    READ,
    WRITE,
    LOAD,
    STORE,
    DEC,
    INC,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    JUMP,
    JUMZ,
    JUML,
    JUMG,
    STOP,
    NOP
} instr_ram;


extern FILE *fp_out;


void init_ram_os();
void codegen(ast *t);
void codegen_nb(ast *t);
void codegen_b_op(ast *t);
void codegen_u_op(ast *t);
void codegen_id(ast *t);

void codegen_or(ast *t);
void codegen_and(ast *t);
void codegen_not(ast *t);
void codegen_lt(ast *t);
void codegen_gt(ast *t);
void codegen_le(ast *t);
void codegen_ge(ast *t);
void codegen_eq(ast *t);
void codegen_ne(ast *t);

void codegen_affect(ast *t);
void codegen_instr(ast *t);
void codegen_io(ast *t);
void codegen_var_decla(ast *t);
void codegen_func_decla(ast *t);
void codegen_func_call(ast *t);
void codegen_return(ast *t);

void codegen_while(ast *t);
void codegen_do_while(ast *t);
void codegen_if(ast *t);

void add_instr(instr_ram instr, char t_adr, int adr);

#endif