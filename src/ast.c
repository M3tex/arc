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
    ast *t = (ast *) calloc(1, sizeof(ast));
    check_alloc(t);

    t->type = type;
    t->mem_adr = -1;
    t->codelen = 0;

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

    /* Il faut faire une opération d'enfilage pour garder le bon ordre */
    if (l == NULL)
    {
        t->list_instr.next = l;
        return t;
    }
    
    /* Enfilage */
    ast *aux = l;
    while (aux->list_instr.next != NULL)
    {
        aux = aux->list_instr.next;
    }
    aux->list_instr.next = t;

    return l;
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
    t->affect.id = id;
    t->affect.expr = expr;

    return t;
}



ast *create_decla_node(ast *decla, ast *l_decla)
{
    ast *t = init_ast(decla_type);
    t->decla_list.decla = decla;

    /* Il faut faire une opération d'enfilage pour garder le bon ordre */
    if (l_decla == NULL)
    {
        t->decla_list.next = l_decla;
        return t;
    }
    
    ast *aux = l_decla;
    while (aux->decla_list.next != NULL)
    {
        aux = aux->decla_list.next;
    }
    aux->decla_list.next = t;

    return l_decla;
}



ast *create_var_decla_node(char *id, ast *expr, ast *next)
{
    ast *t = init_ast(var_decla_type);
    t->var_decla.expr = expr;
    t->var_decla.id = create_id_leaf(id);

    /* Il faut faire une opération d'enfilage pour garder le bon ordre */
    if (next == NULL)
    {
        t->var_decla.next = next;
        return t;
    }
    
    ast *aux = next;
    while (aux->var_decla.next != NULL)
    {
        aux = aux->var_decla.next;
    }
    aux->var_decla.next = t;

    return next;
}


ast *create_prog_root(ast *list_decl, ast *m_prog)
{
    ast *t = init_ast(prog_type);
    t->root.list_decl = list_decl;
    t->root.main_prog = m_prog;

    return t;
}



ast *create_function_node(char *id, ast *params, ast *l_decl, ast *l_i)
{
    ast *t = init_ast(func_type);
    t->function.id = create_id_leaf(id);
    t->function.params = params;
    t->function.list_decl = l_decl;
    t->function.list_instr = l_i;

    return t;
}




ast *create_while_node(ast *expr, ast *l_instr)
{
    ast *t = init_ast(while_type);
    t->while_n.expr = expr;
    t->while_n.list_instr = l_instr;

    return t;
}



ast *create_if_node(ast *expr, ast *l_instr1, ast *l_instr2)
{
    ast *t = init_ast(if_type);
    t->if_n.expr = expr;
    t->if_n.list_instr1 = l_instr1;
    t->if_n.list_instr2 = l_instr2;

    return t;
}



ast *create_print_node(ast *expr)
{
    ast *t = init_ast(print_type);
    t->print.expr = expr;

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
        break;
    case decla_type:
        free_ast(t->decla_list.decla);
        free_ast(t->decla_list.next);
        break;
    case var_decla_type:
        free_ast(t->var_decla.expr);
        free_ast(t->var_decla.id);
        free_ast(t->var_decla.next);
        break;
    case prog_type:
        free_ast(t->root.list_decl);
        free_ast(t->root.main_prog);
        break;
    case func_type:
        free_ast(t->function.id);
        free_ast(t->function.list_decl);
        free_ast(t->function.list_instr);
        free_ast(t->function.params);
        break;
    case while_type:
        free_ast(t->while_n.expr);
        free_ast(t->while_n.list_instr);
        break;
    case if_type:
        free_ast(t->if_n.expr);
        free_ast(t->if_n.list_instr1);
        free_ast(t->if_n.list_instr2);
        break;
    default:
        break;
    }

    free(t);
}



/************************** Partie affichage **************************/


/* id du noeud dessiné */
static int node_id = 0;
static int tmp = 0;
static char buff[128];
static char op_str[16];


/*
 * Les fonctions suivantes génèrent la partie du fichier dot permettant
 * de visualiser les différents noeuds.
 * 
 * On utilise pour cela le noeud "record":
 * https://graphviz.org/doc/info/shapes.html#record
 * 
 * La variable globale `node_id` permet de générer un identifiant unique
 * pour chaque noeud (= un entier), et cet identifiant permet de créer
 * des balises.
 * Par exemple dans b_op_to_dot ci-dessous:
 * La balise <lXl> fait référence au fils de gauche du noeud caractérisé
 * par l'identifiant X.
 */

static void b_op_to_dot(ast* t, int c_id, FILE *fp)
{
    /*
     * Boite avec 2 "sous-boîtes" en dessous. 
     * <lXl> et <rXr> sont les balises marquant respectivement les sous
     * boîtes gauches et droites.
     */
    static char *fmt = "\"{\\%s|{<l%dl> | <r%dr>}}\"";

    b_op_node node = t->b_op;

    /* Création de la boîte de l'opérateur */
    op_to_str(op_str, node.ope);
    sprintf(buff, fmt, op_str, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    /* Création des "fils" de l'opérateur */
    tmp = ast_to_dot(node.l_memb, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

    tmp = ast_to_dot(node.r_memb, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}


static void u_op_to_dot(ast *t, int c_id, FILE *fp)
{
    u_op_node node = t->u_op;
    static char *fmt = "\"{%s|{<c%dc>}}\"";

    op_to_str(op_str, node.ope);
    sprintf(buff, fmt, op_str, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.child, fp);
    fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
}


static void affect_to_dot(ast *t, int c_id, FILE *fp)
{
    affect_node node = t->affect;
    static char *fmt = "\"{ \\<- |{<id%did> | <exp%dexp>}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.id, fp);
    fprintf(fp, "    %d:id%did -- %d;\n", c_id, c_id, tmp);

    if (node.expr == NULL) return;
    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:exp%dexp -- %d;\n", c_id, c_id, tmp);
}


static void instr_to_dot(ast *t, int c_id, FILE *fp)
{
    instr_node node = t->list_instr;
    static char *fmt = "\"{INSTRUCTION|{<l%dl>|<r%dr> next}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.instr, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

    if (node.next == NULL) return;

    tmp = ast_to_dot(node.next, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}


static void decla_to_dot(ast *t, int c_id, FILE *fp)
{
    decla_node node = t->decla_list;
    static char *fmt = "\"{DÉCLA|{<l%dl> | <r%dr>}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.decla, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

    if (node.next == NULL) return;

    tmp = ast_to_dot(node.next, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}



static void var_init_to_dot(ast *t, int c_id, FILE *fp)
{
    var_decla_node node = t->var_decla;
    char pref[16] = "décla";
    char symb[16];
    
    if (node.expr != NULL) strcat(pref, " + init");
    (node.expr == NULL) ? strcpy(symb, " ") : strcpy(symb, "reçoit");

    static char *fmt = "\"{%s de %s|{<l%dl> %s |<r%dr> next}}\"";
    sprintf(buff, fmt, pref, node.id->id.name, c_id, symb, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    if (node.expr != NULL)
    {
        tmp = ast_to_dot(node.expr, fp);
        fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);
    }

    if (node.next == NULL) return;
    tmp = ast_to_dot(node.next, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}



static void func_to_dot(ast *t, int c_id, FILE *fp)
{
    func_node node = t->function;

    static char *fmt = "\"{%s|{<args%dargs>params|<d%dd>déclarations|"\
                       "<i%di>instructions}}\"";

    sprintf(buff, fmt, node.id->id.name, c_id, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    if (node.params != NULL)
    {
        tmp = ast_to_dot(node.params, fp);
        fprintf(fp, "    %d:args%dargs -- %d;\n", c_id, c_id, tmp);
    }

    if (node.list_decl != NULL)
    {
        tmp = ast_to_dot(node.list_decl, fp);
        fprintf(fp, "    %d:d%dd -- %d;\n", c_id, c_id, tmp);
    }

    if (node.list_instr != NULL)
    {
        tmp = ast_to_dot(node.list_instr, fp);
        fprintf(fp, "    %d:i%di -- %d;\n", c_id, c_id, tmp);
    }
}



static void while_to_dot(ast *t, int c_id, FILE *fp)
{
    while_node node = t->while_n;
    static char *fmt = "\"{TQ|{<l%dl>est vrai|<r%dr>FAIRE}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);


    tmp = ast_to_dot(node.list_instr, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}



static void if_to_dot(ast *t, int c_id, FILE *fp)
{
    if_node node = t->if_n;
    static char *fmt = "\"{SI|{<l%dl>est vrai|<m%dm>ALORS|<r%dr>SINON}}\"";

    sprintf(buff, fmt, c_id, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);


    tmp = ast_to_dot(node.list_instr1, fp);
    fprintf(fp, "    %d:m%dm -- %d;\n", c_id, c_id, tmp);

    if (node.list_instr2 == NULL) return;
    tmp = ast_to_dot(node.list_instr2, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}



static void prog_to_dot(ast *t, int c_id, FILE *fp)
{
    prog_root node = t->root;
    static char *fmt = "\"{%s|{<l%dl>Déclarations|<r%dr> main}}\"";

    /* Création de la racine */
    sprintf(buff, fmt, src, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);
    
    tmp = ast_to_dot(node.main_prog, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);

    if (node.list_decl == NULL) return;

    /* Ajout des déclarations */
    tmp = ast_to_dot(node.list_decl, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);


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
    if (t == NULL) return node_id;
    node_id++;

    int c_id = node_id;
    switch (t->type)
    {
    case b_op_type:
        b_op_to_dot(t, c_id, fp);
        break;
    case u_op_type:
        u_op_to_dot(t, c_id, fp);
        break;
    case affect_type:
        affect_to_dot(t, c_id, fp);
        break;
    case nb_type:
        fprintf(fp, "    %d [label=\"%d\"];\n", c_id, t->nb.val);
        break;
    case id_type:
        fprintf(fp, "    %d [label=\"%s\"];\n", c_id, t->id.name);
        break;
    case instr_type:
        instr_to_dot(t, c_id, fp);
        break;
    case decla_type:
        decla_to_dot(t, c_id, fp);
        break;
    case var_decla_type:
        var_init_to_dot(t, c_id, fp);
        break;
    case func_type:
        /* Création de */
        func_to_dot(t, c_id, fp);
        break;
    case while_type:
        while_to_dot(t, c_id, fp);
        break;
    case if_type:
        if_to_dot(t, c_id, fp);
        break;
    case prog_type:
        prog_to_dot(t, c_id, fp);
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
