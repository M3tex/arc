#include "ast.h"
#include "arc_utils.h"
#include "parser.h"         /* Pour yyloc */
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

    t->pos_infos.first_line = yylloc.first_line - line_offset;
    t->pos_infos.last_line = yylloc.last_line - line_offset;
    t->pos_infos.first_column = yylloc.first_column;
    t->pos_infos.last_column = yylloc.last_column;


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
ast *create_id_leaf(const char *id)
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
ast *create_affect_node(char *id, ast *expr, int deref)
{
    ast *t = init_ast(affect_type);
    t->affect.id = create_id_leaf(id);
    t->affect.expr = expr;
    t->affect.is_deref = deref;

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



ast *create_var_decla_node(ast *var, ast *expr, ast *next, type_symb type)
{
    ast *t = init_ast(var_decla_type);
    t->var_decla.expr = expr;
    t->var_decla.type = type;
    t->var_decla.var = var;

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
    ast *t = init_ast(func_decla_type);
    t->func_decla.id = create_id_leaf(id);
    t->func_decla.params = params;
    t->func_decla.list_decl = l_decl;
    t->func_decla.list_instr = l_i;

    /* On compte le nombre de paramètres */
    t->func_decla.nb_params = 0;
    ast *aux = t->func_decla.params;
    while (aux != NULL)
    {
        t->func_decla.nb_params++;
        aux = aux->var_decla.next;
    }

    /* Idem pour le nombre de déclarations */
    t->func_decla.nb_decla = 0;
    aux = t->func_decla.list_decl;
    while (aux != NULL)
    {
        t->func_decla.nb_decla++;
        aux = aux->decla_list.next;
    } 

    return t;
}



ast *create_return_node(ast *expr)
{
    ast *t = init_ast(return_type);
    t->return_n.expr = expr;

    return t;
}




ast *create_while_node(ast *expr, ast *l_instr)
{
    ast *t = init_ast(while_type);
    t->while_n.expr = expr;
    t->while_n.list_instr = l_instr;

    return t;
}


ast *create_do_while_node(ast *l_instr, ast *expr)
{
    ast *t = init_ast(do_while_type);
    t->do_while.expr = expr;
    t->do_while.list_instr = l_instr;

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


ast *create_for_node(char *id, ast *start_exp, ast *end_exp, ast *l_instr)
{
    ast *t = init_ast(for_type);
    t->for_n.id = create_id_leaf(id);
    t->for_n.end_exp = create_b_op_node('<', t->for_n.id, end_exp);
    t->for_n.list_instr = l_instr;
    t->for_n.affect_init = create_affect_node(id, start_exp, 0);

    return t;
}



/**
 * @brief Créé un noeud d'entrée / sortie (LIRE ou ECRIRE).
 * m vaudra r pour LIRE et w pour ECRIRE
 * 
 * @param expr 
 * @param m 
 * @return ast* 
 */
ast *create_io_node(ast *expr, char m)
{
    ast *t = init_ast(io_type);
    t->io.expr = expr;
    t->io.mode = m;

    return t;
}


ast *create_exp_list_node(ast *expr, ast *next)
{
    ast *t = init_ast(exp_list_type);
    t->exp_list.exp = expr;

    t->pos_infos.last_column = expr->pos_infos.last_column;

    if (next != NULL)
    {
        t->pos_infos.first_column = next->pos_infos.first_column;
        next->pos_infos = t->pos_infos;
    }
    else t->pos_infos.first_column = expr->pos_infos.first_column;

    
    
    /* Il faut faire une opération d'enfilage pour garder le bon ordre */
    if (next == NULL)
    {
        t->exp_list.next = NULL;
        return t;
    }

    ast *aux = next;
    while (aux->exp_list.next != NULL)
    {
        aux = aux->exp_list.next;
    }
    aux->exp_list.next = t;

    

    return next;
}



ast *create_function_call_node(const char *id, ast *params)
{
    ast *t = init_ast(func_call_type);
    t->func_call.func_id = create_id_leaf(id);
    t->func_call.params = params;

    return t;
}




ast *create_arr_access_node(char *id, ast *ind_expr, ast *aff_expr)
{
    ast *t = init_ast(array_access_type);
    t->arr_access.id = create_id_leaf(id);
    t->arr_access.ind_expr = ind_expr;
    t->arr_access.affect_expr = aff_expr;

    return t;
}


/**
 * @brief Créé le noeud représentant une déclaration de tableau.
 * Soit: @tab (taille indéfinie)
 * ou tab[5]  (de taille 5 sans initialisation)
 * ou tab[3] <- [1, 2, 3] (de taille 3, initialisé)
 * 
 * @param id 
 * @param size 
 * @param l_exp 
 * @return ast* 
 */
ast *create_arr_decla_node(char *id, int size, ast *l_exp)
{
    ast *t = init_ast(array_decla_type);

    t->arr_decla.id = create_id_leaf(id);
    t->arr_decla.size = size;
    t->arr_decla.list_expr = l_exp;

    return t;
}



ast *create_alloc_node(char *id, ast *expr)
{
    ast *t = init_ast(alloc_type);

    t->alloc.id = create_id_leaf(id);
    t->alloc.expr = expr;

    return t;
}



ast *create_proto_node(char *id, ast *params)
{
    ast *t = init_ast(proto_type);

    t->proto.id = create_id_leaf(id);
    t->proto.params = params;

    /* On compte le nombre de paramètres */
    t->proto.nb_params = 0;
    ast *aux = t->proto.params;
    while (aux != NULL)
    {
        t->proto.nb_params++;
        aux = aux->var_decla.next;
    }

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
        free_ast(t->var_decla.var);
        free_ast(t->var_decla.next);
        break;
    case prog_type:
        free_ast(t->root.list_decl);
        free_ast(t->root.main_prog);
        break;
    case func_decla_type:
        free_ast(t->func_decla.id);
        free_ast(t->func_decla.list_decl);
        free_ast(t->func_decla.list_instr);
        free_ast(t->func_decla.params);
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
    case do_while_type:
        free_ast(t->do_while.expr);
        free_ast(t->do_while.list_instr);
        break;
    case exp_list_type:
        free_ast(t->exp_list.exp);
        free_ast(t->exp_list.next);
        break;
    case func_call_type:
        free_ast(t->func_call.func_id);
        free_ast(t->func_call.params);
        break;
    case return_type:
        free_ast(t->return_n.expr);
        break;
    case for_type:
        free_ast(t->for_n.affect_init);
        free_ast(t->for_n.list_instr);
        free_ast(t->for_n.end_exp);
        break;
    case io_type:
        free_ast(t->io.expr);
        break;
    case array_access_type:
        free_ast(t->arr_access.id);
        free_ast(t->arr_access.ind_expr);
        free_ast(t->arr_access.affect_expr);
        break;
    case array_decla_type:
        free_ast(t->arr_decla.id);
        free_ast(t->arr_decla.list_expr);
        break;
    case alloc_type:
        free_ast(t->alloc.id);
        free_ast(t->alloc.expr);
        break;
    case proto_type:
        free_ast(t->proto.id);
        free_ast(t->proto.params);
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


static void exp_list_to_dot(ast *t, int c_id, FILE *fp)
{
    exp_list_node node = t->exp_list;
    static char *fmt = "\"{L_EXPR|{<l%dl>|<r%dr> next}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.exp, fp);
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



static void arr_decla_to_dot(ast *t, int c_id, FILE *fp)
{
    array_decla_node node = t->arr_decla;
    printf("Taille %d\n", node.size);


    char pref[16] = "décla";
    char symb[16];
    
    if (node.list_expr != NULL) strcat(pref, " + init");
    (node.list_expr == NULL) ? strcpy(symb, " ") : strcpy(symb, "reçoit");

    static char *fmt = "\"{%s du tab %s (taille: %d)|{<l%dl> %s |<r%dr> next}}\"";
    sprintf(buff, fmt, pref, node.id->id.name, node.size, c_id, symb, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);


    if (node.list_expr == NULL) return;

    tmp = ast_to_dot(node.list_expr, fp);
    fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
}



static void var_init_to_dot(ast *t, int c_id, FILE *fp)
{
    var_decla_node node = t->var_decla;

    if (node.type == array)
    {
        arr_decla_to_dot(node.var, c_id, fp);
        if (node.next == NULL) return;
        tmp = ast_to_dot(node.next, fp);
        fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
        return;
    }

    char pref[16] = "décla";
    char symb[16];
    
    if (node.expr != NULL) strcat(pref, " + init");
    (node.expr == NULL) ? strcpy(symb, " ") : strcpy(symb, "reçoit");

    static char *fmt = "\"{%s de %s|{<l%dl> %s |<r%dr> next}}\"";
    sprintf(buff, fmt, pref, node.var->id.name, c_id, symb, c_id);
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



static void func_decla_to_dot(ast *t, int c_id, FILE *fp)
{
    func_decla_node node = t->func_decla;

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


static void do_while_to_dot(ast *t, int c_id, FILE *fp)
{
    do_while_node node = t->do_while;
    static char *fmt = "\"{FAIRE|{<l%dl>|<r%dr>TQ est vraie}}\"";

    sprintf(buff, fmt, c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.list_instr, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

    tmp = ast_to_dot(node.expr, fp);
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


static void for_to_dot(ast *t, int c_id, FILE *fp)
{
    for_node node = t->for_n;
    static char *fmt = "\"{pour chaque %s|{<s%ds>de|<e%de>à|<i%di>faire}}\"";

    sprintf(buff, fmt, node.id->id.name, c_id, c_id, c_id);
    fprintf(fp, "\t%d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.affect_init->affect.expr, fp);
    fprintf(fp, "\t%d:s%ds -- %d;\n", c_id, c_id, tmp);

    tmp = ast_to_dot(node.end_exp->b_op.r_memb, fp);
    fprintf(fp, "\t%d:e%de -- %d;\n", c_id, c_id, tmp);

    tmp = ast_to_dot(node.list_instr, fp);
    fprintf(fp, "\t%d:i%di -- %d;\n", c_id, c_id, tmp);
}



static void io_to_dot(ast *t, int c_id, FILE *fp)
{
    io_node node = t->io;
    static char *fmt = "\"{%s|{<c%dc>}}\"";

    char inst_name[32];
    if (node.mode == 'r') strcpy(inst_name, "Lire");
    else strcpy(inst_name, "Afficher");

    sprintf(buff, fmt, inst_name, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    if (node.expr == NULL) return;

    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
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


static void function_call_to_dot(ast *t, int c_id, FILE *fp)
{
    func_call_node node = t->func_call;
    static char *fmt = "\"{appel de %s|{<p%dp>params}}\"";

    /* Création du noeud */
    sprintf(buff, fmt, node.func_id->id.name, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);
    
    if (node.params == NULL) return;
    tmp = ast_to_dot(node.params, fp);
    fprintf(fp, "    %d:p%dp -- %d;\n", c_id, c_id, tmp);
}


static void return_to_dot(ast *t, int c_id, FILE *fp)
{
    return_node node = t->return_n;
    static char *fmt = "\"{RETOURNER|{<c%dc>}}\"";

    sprintf(buff, fmt, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    if (node.expr == NULL) return;

    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
}



static void arr_access_to_dot(ast *t, int c_id, FILE *fp)
{
    array_access_node node = t->arr_access;
    static char *fmt = "\"{Accès tableau %s|{<l%dl>indice|<r%dr>reçoit}}\"";

    sprintf(buff, fmt, node.id->id.name , c_id, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    if (node.ind_expr == NULL) return;

    tmp = ast_to_dot(node.ind_expr, fp);
    fprintf(fp, "    %d:l%dl -- %d;\n", c_id, c_id, tmp);

    if (node.affect_expr == NULL) return;
    tmp = ast_to_dot(node.affect_expr, fp);
    fprintf(fp, "    %d:r%dr -- %d;\n", c_id, c_id, tmp);
}


static void alloc_to_dot(ast *t, int c_id, FILE *fp)
{
    alloc_node node = t->alloc;
    static char *fmt = "\"{Allocation pour %s|{<c%dc>}}\"";

    sprintf(buff, fmt, node.id->id.name, c_id);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);

    tmp = ast_to_dot(node.expr, fp);
    fprintf(fp, "    %d:c%dc -- %d;\n", c_id, c_id, tmp);
}


static void proto_to_dot(ast *t, int c_id, FILE *fp)
{
    proto_node node = t->proto;

    static char *fmt = "\"{Prototype de %s|{<args%dargs>%d params}}\"";

    sprintf(buff, fmt, node.id->id.name, c_id, node.nb_params);
    fprintf(fp, "    %d [label=%s];\n", c_id, buff);
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
    case func_decla_type:
        func_decla_to_dot(t, c_id, fp);
        break;
    case func_call_type:
        function_call_to_dot(t, c_id, fp);
        break;
    case return_type:
        return_to_dot(t, c_id, fp);
        break;
    case while_type:
        while_to_dot(t, c_id, fp);
        break;
    case for_type:
        for_to_dot(t, c_id, fp);
        break;
    case do_while_type:
        do_while_to_dot(t, c_id, fp);
        break;
    case if_type:
        if_to_dot(t, c_id, fp);
        break;
    case io_type:
        io_to_dot(t, c_id, fp);
        break;
    case prog_type:
        prog_to_dot(t, c_id, fp);
        break;
    case exp_list_type:
        exp_list_to_dot(t, c_id, fp);
        break;
    case array_access_type:
        arr_access_to_dot(t, c_id, fp);
        break;
    case array_decla_type:
        arr_decla_to_dot(t, c_id, fp);
        break;
    case alloc_type:
        alloc_to_dot(t, c_id, fp);
        break;
    case proto_type:
        proto_to_dot(t, c_id, fp);
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
