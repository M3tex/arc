#ifndef _AST_HEADER
#define _AST_HEADER


#include <stdlib.h>
#include <stdio.h>

#include "parser.h"     /* Pour les infos de ligne/colonne */


/* 
 * Contient toutes les fonctions / types nécessaires à la gestion de
 * l'arbre syntaxique abstrait (Abstract Syntax Tree = AST).
 */

/* 32 caractères max pour un identifiant */
#define ID_MAX_SIZE 32


extern char *src;
extern int line_offset;

/* Besoin pour les noeuds */
typedef struct ast ast;


typedef enum {
    nb_type, b_op_type, u_op_type, id_type, affect_type, instr_type,
    decla_type, var_decla_type, prog_type, func_decla_type, while_type,
    if_type, io_type, func_call_type, exp_list_type, do_while_type,
    return_type, for_type, array_access_type, array_decla_type,
    alloc_type, proto_type
} node_type;


typedef enum {integer, pointer, array, func} type_symb;


typedef struct {
    int val;
} nb_leaf;


typedef struct {
    char name[ID_MAX_SIZE];
} id_leaf;

/* l_memb le membre de gauche et r_memb celui de droite */
typedef struct {
    int ope;
    ast *l_memb;
    ast *r_memb;
} b_op_node;


typedef struct {
    int ope;
    ast *child;
} u_op_node;


/*
 * instr contient l'instruction courante, next la suivante, ou NULL s'il n'y a
 * pas d'instruction suivante
 */
typedef struct {
    ast *instr;
    ast *next;
} instr_node;


typedef struct {
    ast *id;
    ast *expr;
    int is_deref;
} affect_node;


/* Idem que pour les instructions, avec exp l'expression courante */
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


/*
 * var: feuile id ou noeud tableau.
 * expr: l'expression d'initialisation si var est un id
 * next: la déclaration de variable suivante
 * type: le type de la variable (pointeur, int ou tableau)
 */
typedef struct {
    ast *var;
    ast *expr;
    ast *next;
    type_symb type;
} var_decla_node;


typedef struct {
    ast *list_decl;
    ast *main_prog;
} prog_root;


/*
 * id: la feuille contenant le nom de la fonction.
 * params: la liste des paramètres
 * list_decl: la liste des déclarations
 * list_instr: la liste des instructions
 */
typedef struct {
    ast *id;
    ast *params;
    ast *list_decl;
    ast *list_instr;
    size_t nb_params;
    size_t nb_decla;
} func_decla_node;


/*
 * expr: l'expression controllant la boucle
 * list_instr: la liste des instructions à exécuter tant que expr est vraie
 */
typedef struct {
    ast *expr;
    ast *list_instr;
} while_node;


/* idem */
typedef struct {
    ast *list_instr;
    ast *expr;
} do_while_node;


/*
 * expr: l'expression évaluée
 * list_instr1: la liste des instructions à exécuter si expr est vraie
 * list_intr2: la liste des instructions à exécuter si expr est fausse (=sinon).
 * Peut-être NULL si pas de sinon.
 */
typedef struct {
    ast *expr;
    ast *list_instr1;
    ast *list_instr2;
} if_node;


/*
 * mode: 'r' pour LIRE et 'w' pour ECRIRE
 * expr: l'expression à écrire
 */
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


/*
 * id: la variable qui prendra les différentes valeurs
 * affect_init: la 1 ère valeur que prendra la variable
 * end_exp: la 1 ère valeur que ne peut pas prendre la variable
 * list_instr: les instructions exécutées tant que la variable ne dépasse pas la
 * dernière valeur.
 * 
 * Par exemple dans: POUR i dans n...2 * n + 7, affect_init est n, end_exp est
 * 2 * n + 7.
 */
typedef struct {
    ast *id;
    ast *end_exp;
    ast *list_instr;
    ast *affect_init;
} for_node;


/*
 * ind_expr est l'exression contenue entre les crochets. 
 * affect_expr est l'éventuelle initialisation du tableau.
 */
typedef struct {
    ast *id;
    ast *ind_expr;
    ast *affect_expr;
} array_access_node;



typedef struct {
    ast *id;
    int size;
    ast *list_expr;
} array_decla_node;


typedef struct {
    ast *id;
    ast *expr;
} alloc_node;


typedef struct {
    ast *id;
    ast *params;
    size_t nb_params;
} proto_node;




typedef struct ast {
    node_type type;
    int mem_adr;
    size_t codelen;
    YYLTYPE pos_infos;
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
        array_access_node arr_access;
        array_decla_node arr_decla;
        alloc_node alloc;
        proto_node proto;
    };
} ast;


void free_ast(ast *t);
ast *init_ast(node_type type);

ast *create_nb_leaf(int value);
ast *create_return_node(ast *expr);
ast *create_id_leaf(const char *id);
ast *create_u_op_node(int op, ast *c);
ast *create_io_node(ast *expr, char m);
ast *create_instr_node(ast *instr, ast *l);
ast *create_alloc_node(char *id, ast *expr);
ast *create_proto_node(char *id, ast *params);
ast *create_b_op_node(int op, ast *m1, ast *m2);
ast *create_exp_list_node(ast *expr, ast *next);
ast *create_while_node(ast *expr, ast *l_instr);
ast *create_decla_node(ast *decla, ast *l_decla);
ast *create_do_while_node(ast *l_instr, ast *expr);
ast *create_prog_root(ast *list_decl, ast *m_prog);
ast *create_affect_node(char *id, ast *expr, int deref);
ast *create_arr_decla_node(char *id, int size, ast *l_exp);
ast *create_function_call_node(const char *id, ast *params);
ast *create_if_node(ast *expr, ast *l_instr1, ast *l_instr2);
ast *create_arr_access_node(char *id, ast *ind_expr, ast *aff_expr);
ast *create_function_node(char *id, ast *params, ast *l_decl, ast *l_i);
ast *create_var_decla_node(ast *var, ast *expr, ast *next, type_symb type);
ast *create_for_node(char *id, ast *start_exp, ast *end_exp, ast *l_instr);

int ast_to_dot(ast *t, FILE *fp);
void ast_to_img(ast *t, char *filename, char *fmt);

#endif