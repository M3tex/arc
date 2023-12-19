#ifndef _AST_HEADER
#define _AST_HEADER


#include <stdlib.h>
#include <stdio.h>

/* 
 * Contient toutes les fonctions / types nécessaires à la gestion de
 * l'arbre syntaxique abstrait (Abstract Syntax Tree).
 */

#define ID_MAX_SIZE 32


extern char *src;
extern int line_offset;

/* Besoin pour les noeuds */
typedef struct ast ast;


typedef enum {
    nb_type, b_op_type, u_op_type, id_type, affect_type, instr_type,
    decla_type, var_decla_type, prog_type, func_decla_type, while_type,
    if_type, io_type, func_call_type, exp_list_type, do_while_type,
    return_type, for_type
} node_type;


typedef struct {
    int val;
} nb_leaf;


typedef struct {
    char name[ID_MAX_SIZE];
} id_leaf;


typedef struct {
    int ope;
    ast *l_memb;
    ast *r_memb;
} b_op_node;


typedef struct {
    int ope;
    ast *child;
} u_op_node;


typedef struct {
    ast *instr;
    ast *next;
} instr_node;


typedef struct {
    ast *id;
    ast *expr;
} affect_node;


typedef struct {
    ast *exp;
    ast *next;
} exp_list_node;

/*
 * Un noeud de déclaration contient une déclaration et la déclaration
 * suivante. Une déclaration peut-être une fonction ou bien une
 * déclaration (ou initialisation, i.e VAR x <- 1) de variable.
 */
typedef struct {
    ast *decla;
    ast *next;
} decla_node;


typedef struct {
    ast *id;
    ast *expr;
    ast *next;
} var_decla_node;


typedef struct {
    ast *list_decl;
    ast *main_prog;
} prog_root;


typedef struct {
    ast *id;
    ast *params;
    ast *list_decl;
    ast *list_instr;
    size_t nb_params;
    size_t nb_decla;
} func_decla_node;


typedef struct {
    ast *expr;
    ast *list_instr;
} while_node;


typedef struct {
    ast *list_instr;
    ast *expr;
} do_while_node;


typedef struct {
    ast *expr;
    ast *list_instr1;
    ast *list_instr2;
} if_node;


typedef struct {
    ast *expr;
    char mode;
} io_node;


typedef struct {
    ast *func_id;
    ast *params;
} func_call_node;


typedef struct {
    ast *expr;
} return_node;


typedef struct {
    ast *id;
    ast *end_exp;
    ast *list_instr;
    ast *affect_init;
} for_node;


typedef struct ast {
    node_type type;
    int mem_adr;
    size_t codelen;
    int lig;
    int col;
    union {
        nb_leaf nb;
        id_leaf id;
        b_op_node b_op;
        u_op_node u_op;
        affect_node affect;
        instr_node list_instr;
        decla_node decla_list;
        exp_list_node exp_list;
        var_decla_node var_decla;
        prog_root root;
        func_decla_node func_decla;
        func_call_node func_call;
        return_node return_n;
        while_node while_n;
        do_while_node do_while;
        if_node if_n;
        io_node io;
        for_node for_n;
    };
} ast;


ast *init_ast(node_type type);
void free_ast(ast *t);

ast *create_nb_leaf(int value);
ast *create_id_leaf(const char *id);

ast *create_b_op_node(int op, ast *m1, ast *m2);
ast *create_u_op_node(int op, ast *c);
ast *create_instr_node(ast *instr, ast *l);
ast *create_affect_node(char *id, ast *expr);
ast *create_decla_node(ast *decla, ast *l_decla);
ast *create_var_decla_node(char *id, ast *expr, ast *next);
ast *create_function_node(char *id, ast *params, ast *l_decl, ast *l_i);
ast *create_function_call_node(const char *id, ast *params);
ast *create_exp_list_node(ast *expr, ast *next);
ast *create_return_node(ast *expr);

ast *create_io_node(ast *expr, char m);

ast *create_prog_root(ast *list_decl, ast *m_prog);

ast *create_while_node(ast *expr, ast *l_instr);
ast *create_do_while_node(ast *l_instr, ast *expr);
ast *create_if_node(ast *expr, ast *l_instr1, ast *l_instr2);
ast *create_for_node(char *id, ast *start_exp, ast *end_exp, ast *l_instr);


int ast_to_dot(ast *t, FILE *fp);
void ast_to_img(ast *t, char *filename, char *fmt);


#endif