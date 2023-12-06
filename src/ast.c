#include "ast.h"
#include "arc_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



/**
 * @brief Initialise les champs commun à tous les noeuds/feuilles de
 * l'ASA.
 * 
 * @param type Le type du futur noeud/feuille.
 * @return ast* 
 */
ast *init_ast(node_type type)
{
    ast *t = (ast *) malloc(sizeof(ast));
    check_alloc(t);

    t->type = type;
    t->mem_adr = -1;
    t->codelen = -1;

    return t; 
}


/**
 * @brief Créer la feuille nombre contenant la valeur passée en 
 * paramètre.
 * 
 * @param value 
 * @return ast* 
 */
ast *create_nb_leaf(int value)
{
    ast *t = init_ast(nb_type);
    t->nb.val = value;

    return t;
} 


/**
 * @brief Créer la feuille ID contenant l'identificateur passé en 
 * paramètre.
 * 
 * @param id 
 * @return ast* 
 */
ast *create_id_leaf(char *id)
{
    ast *t = init_ast(id_type);
    strcpy(t->id.name, id);

    return t;
}



/**
 * @brief Créer le noeud pour l'opérateur binaire passé en paramètre.
 * 
 * @param op 
 * @param m1 Le membre de gauche
 * @param m2 Le membre de droite
 * @return ast* 
 */
ast *create_b_op_node(int op, ast *m1, ast *m2)
{
    ast *t = init_ast(b_op_type);
    t->b_op.l_memb = m1;
    t->b_op.r_memb = m2;
    t->b_op.ope = op;

    return t;
}


/**
 * @brief Créer le noeud pour l'opérateur unaire passé en paramètre.
 * 
 * @param op 
 * @param c 
 * @return ast* 
 */
ast *create_u_op_node(int op, ast *c)
{
    ast *t = init_ast(u_op_type);
    t->u_op.child = c;
    t->u_op.ope = op;

    return t;
}


/**
 * @brief Créer le noeud pour l'instruction passé en paramètre.
 * Cette instruction sera empilée à la liste des instructions `l`
 * 
 * @param instr 
 * @param next 
 * @return ast* 
 */
ast *create_instr_node(ast *instr, ast *l)
{
    ast *t = init_ast(instr_type);
    t->list_instr.instr = instr;
    t->list_instr.next = l;

    return t;
}


/**
 * @brief Créer le noeud d'affectation.
 * 
 * @param id 
 * @param expr 
 * @return ast* 
 */
ast *create_affect_node(ast *id, ast *expr)
{
    ast *t = init_ast(affect_type);
    t->affect.expr = expr;
    t->affect.id = id;

    return t;
}



ast *create_var_decla_node(ast *var_init, ast *l)
{
    ast *t = init_ast(var_decla_type);
    t->list_var_decla.init_var = var_init;
    t->list_var_decla.next = l;

    return t;
}



ast *create_var_init_node(char *id, ast *expr, ast *next)
{
    ast *t = init_ast(var_init_type);
    t->var_init.expr = expr;
    t->var_init.id = create_id_leaf(id);
    t->var_init.next = next;

    return t;
}


ast *create_prog_root(ast *list_decl, ast *m_prog)
{
    ast *t = init_ast(prog_type);
    t->root.list_decl = list_decl;
    t->root.main_prog = m_prog;

    return t;
}


/**
 * @brief Libère la mémoire utilisée par l'ASA.
 * 
 * @param t 
 */
void free_ast(ast *t)
{
    if (t == NULL) return;

    switch (t->type)
    {
    case b_op_type:
        free_ast(t->b_op.l_memb);
        free_ast(t->b_op.r_memb);
        break;
    case u_op_type:
        free_ast(t->u_op.child);
        break;
    case instr_type:
        free_ast(t->list_instr.instr);
        free_ast(t->list_instr.next);
        break;
    case affect_type:
        free_ast(t->affect.id);
        free_ast(t->affect.expr);
    default:
        break;
    }

    free(t);
}



/**
 * @brief Convertit l'ASA en fichier utilisable avec dot
 * https://graphviz.org/doc/info/shapes.html#record
 * 
 * @param t 
 * @param fp
 * @return int L'id du noeud ajouté
 */
int ast_to_dot(ast *t, FILE *fp)
{
    /* Pour éviter la réallocation à chaque appel récursif */
    static char buff[64];
    static char op_str[16];

    /* id du noeud dessiné */
    static int node_id = 0;

    if (t == NULL) return node_id;
    node_id++;

    int tmp;
    int c_id = node_id;
    switch (t->type)
    {
    case b_op_type:
        // ! Convertir ope en string pour les <= etc.
        op_to_str(op_str, t->b_op.ope);
        sprintf(buff, "\"{\\%s|{<l%dl> | <r%dr>}}\"", op_str, c_id, c_id);
        fprintf(fp, "    %d [label=%s];\n", c_id, buff);

        tmp = ast_to_dot(t->b_op.l_memb, fp);
        fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

        tmp = ast_to_dot(t->b_op.r_memb, fp);
        fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
        break;
    case u_op_type:
        op_to_str(op_str, t->u_op.ope);
        sprintf(buff, "\"{%s|{<c%dc>}}\"", op_str, c_id);
        fprintf(fp, "    %d [label=%s];\n", c_id, buff);

        tmp = ast_to_dot(t->u_op.child, fp);
        fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
        break;
    case affect_type:
        sprintf(buff, "\"{ \\<- |{<id%did> | <exp%dexp>}}\"", c_id, c_id);
        fprintf(fp, "    %d [label=%s];\n", c_id, buff);

        tmp = ast_to_dot(t->affect.id, fp);
        fprintf(fp, "    %d:id%did -- %d;\n", c_id, c_id, tmp);

        tmp = ast_to_dot(t->affect.expr, fp);
        fprintf(fp, "    %d:exp%dexp -- %d;\n", c_id, c_id, tmp);
        break;
    case nb_type:

        fprintf(fp, "    %d [label=\"%d\"];\n", c_id, t->nb.val);
        break;
    case id_type:

        fprintf(fp, "    %d [label=\"%s\"];\n", c_id, t->id.name);
        break;
    case instr_type:
        ast_to_dot(t->list_instr.instr, fp);
        ast_to_dot(t->list_instr.next, fp);
        break;
    case var_decla_type:
        ast_to_dot(t->list_var_decla.init_var, fp);
        ast_to_dot(t->list_var_decla.next, fp);
        break;
    case var_init_type:
        ast_to_dot(t->var_init.id, fp);
        ast_to_dot(t->var_init.expr, fp);
        ast_to_dot(t->var_init.next, fp);
        break;
    case prog_type:
        ast_to_dot(t->root.list_decl, fp);
        ast_to_dot(t->root.main_prog, fp);
        break;
    default:
        break;
    }

    return c_id;
}



/**
 * @brief Dessine l'ASA à l'aide de l'utilitaire dot.
 * 
 * @param t L'ASA
 * @param filename Le nom du fichier de sortie 
 * @param fmt Le format du fichier de sortie (png, svg, etc.)
 */
void ast_to_img(ast *t, char *filename, char *fmt)
{
    FILE *fp = fopen("tmp.dot", "w");
    check_alloc(fp);

    fprintf(fp, "graph G {\n");
    fprintf(fp, "    node [shape=record];\n    overlap=false;\n");
    ast_to_dot(t, fp);
    fprintf(fp, "}\n");
    fclose(fp);

    char cmd[128];
    sprintf(cmd, "dot -T%s tmp.dot -o %s.%s", fmt, filename, fmt);

    system(cmd);
    system("rm tmp.dot");
}

