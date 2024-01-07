#ifndef _SEMANTIC_HEADER
#define _SEMANTIC_HEADER


#include "ast.h"
#include "symbol_table.h"


/* Coût des opérations sur la pile (en nb d'instructions RAM) */
#define PUSH_COST 2
#define POP_COST 2
#define PEEK_COST 1


extern symb_table table;

void semantic(ast *t);
void second_turn_semantic(ast *t, ast *parent);

void semantic_nb(ast *t);
void semantic_id(ast *t);
void semantic_io(ast *t);
void semantic_if(ast *t);
void semantic_for(ast *t);
void semantic_u_op(ast *t);
void semantic_b_op(ast *t);
void semantic_instr(ast *);
void semantic_decla(ast *t);
void semantic_instr(ast *t);
void semantic_while(ast *t);
void semantic_alloc(ast *t);
void semantic_proto(ast *t);
void semantic_affect(ast *t);
void semantic_return(ast *t);
void semantic_do_while(ast *t);
void semantic_exp_list(ast *t);
void semantic_func_call(ast *t);
void semantic_var_decla(ast *t);
void semantic_arr_access(ast *t);
void semantic_func_decla(ast *t);

#endif