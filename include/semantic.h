#ifndef _SEMANTIC_HEADER
#define _SEMANTIC_HEADER


#include "ast.h"
#include "symbol_table.h"

extern symb_table table;
extern char current_ctx[32];


void semantic(ast *t);
void semantic_nb(ast *t);
void semantic_id(ast *t);
void semantic_b_op(ast *t);
void semantic_u_op(ast *t);
void semantic_affect(ast *t);
void semantic_instr(ast *);
void semantic_decla(ast *t);
void semantic_instr(ast *t);
void semantic_io(ast *t);
void semantic_var_decla(ast *t);
void semantic_func_decla(ast *t);
void semantic_func_call(ast *t);
void semantic_while(ast *t);
void semantic_do_while(ast *t);
void semantic_if(ast *t);
void semantic_for(ast *t);
void semantic_return(ast *t);
void semantic_exp_list(ast *t);

#endif