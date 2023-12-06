#ifndef _AST_HEADER
#define _AST_HEADER


#include <stdlib.h>


/* 
 * Contient toutes les fonctions / types nécessaires à la gestion de
 * l'arbre syntaxique abstrait (Abstract Syntax Tree).
 */

#define ID_MAX_SIZE 32


/* Besoin pour les noeuds */
typedef struct ast ast;


typedef enum {
    nb_type, b_op_type, u_op_type, id_type, affect_type, instr_type,
    var_decla_type, var_init_type, prog_type, func_type
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
    ast *init_var;
    ast *next;
} var_decla_node;


typedef struct {
    ast *id;
    ast *expr;
    ast *next;
} var_init_node;


typedef struct {
    ast *list_decl;
    ast *main_prog;
} prog_root;


typedef struct {

} func_node;

// typedef struct {

// } decla_func_node;


typedef struct ast {
    node_type type;
    int mem_adr;
    size_t codelen;
    union {
        nb_leaf nb;
        id_leaf id;
        b_op_node b_op;
        u_op_node u_op;
        affect_node affect;
        instr_node list_instr;
        var_decla_node list_var_decla;
        var_init_node var_init;
        prog_root root;
    };
} ast;


ast *init_ast(node_type type);
ast *create_nb_leaf(int value);
ast *create_id_leaf(char *id);
ast *create_b_op_node(int op, ast *m1, ast *m2);
ast *create_u_op_node(int op, ast *c);
ast *create_instr_node(ast *instr, ast *l);
ast *create_affect_node(ast *id, ast *expr);
ast *create_var_decla_node(ast *var_init, ast *l);
ast *create_var_init_node(char *id, ast *expr, ast *next);

ast *create_prog_root(ast *list_decl, ast *m_prog);

void free_ast(ast *t);

void ast_to_img(ast *t, char *filename, char *fmt);


#endif