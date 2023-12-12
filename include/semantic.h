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
void semantic_var_decla(ast *t);
void semantic_function(ast *t);


#endif