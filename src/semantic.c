#include "semantic.h"
#include "arc_utils.h"
#include "ram_os.h"
#include <string.h>
#include <limits.h>


/*
 * Adresses relatives.
 * Par exemple, static_rel_adr est l'adresse relative par rapport à la mémoire
 * statique (0 pour la 1ère variable stockée, 1 pour la deuxième, etc.).
 * Idem pour la pile, à l'exception que l'adresse relative sera "reset"
 * à chaque fin de fonction.
 */
int static_rel_adr = 0;
static int stack_rel_adr = 0;

/* Contexte par défaut */
char current_ctx[32] = "global";

/* Pour swap les contextes */
static char old_context[32];

/* Pour savoir si une fonction a un "RETOURNER" */
static int has_return_instr = 0;




/**
 * @brief Réalise l'analyse sémantique de l'arbre syntaxique abstrait
 * 
 * @param t 
 */
void semantic(ast *t)
{

    if (t == NULL) return;

    switch (t->type)
    {
    case nb_type:
        semantic_nb(t);
        break;
    case id_type:
        semantic_id(t);
        break;
    case b_op_type:
        semantic_b_op(t);
        break;
    case u_op_type:
        semantic_u_op(t);
        break;
    case affect_type:
        semantic_affect(t);
        break;
    case instr_type:
        semantic_instr(t);
        break;
    case while_type:
        semantic_while(t);
        break;
    case do_while_type:
        semantic_do_while(t);
        break;
    case if_type:
        semantic_if(t);
        break;
    case for_type:
        semantic_for(t);
        break;
    case decla_type:
        semantic_decla(t);
        break;
    case var_decla_type:
        semantic_var_decla(t);
        break;
    case prog_type:
        semantic(t->root.list_decl);
        semantic(t->root.main_prog);

        t->codelen = t->root.main_prog->codelen;
        if (t->root.list_decl != NULL)
        {
            t->codelen += t->root.list_decl->codelen;
        }
        
        break;
    case func_decla_type:
        semantic_func_decla(t);
        break;
    case func_call_type:
        semantic_func_call(t);
        break;
    case return_type:
        semantic_return(t);
        break;
    case exp_list_type:
        semantic_exp_list(t);
        break;
    case io_type:
        semantic_io(t);
        break;
    case array_access_type:
        semantic_arr_access(t);
        break;
    case alloc_type:
        semantic_alloc(t);
        break;
    case proto_type:
        semantic_proto(t);
        break;
    default:
        break;
    }
}



/**
 * @brief 2ème parcourt de l'arbre, après l'analyse sémantique.
 * Permet de lever différents warning et de mettre à jour correctement l'adresse
 * des fonctions.
 * Les optimisations auraient aussi eu lieues ici.
 * 
 * @param t 
 * @param parent
 */
void second_turn_semantic(ast *t, ast *parent)
{
    static int is_param_decl = 0;
    static size_t offset_cdln = 0;
    symbol *tmp;

    char *id;

    if (t == NULL) return;

    switch (t->type)
    {
    case id_type:
        tmp = get_symbol(table, current_ctx, t->id.name);
        set_error_info(t->pos_infos);
        if (!tmp->is_used && !tmp->is_checked)
        {
            warning("l'identificateur ‘~m%s~E‘ n'est pas utilisé", t->id.name);
        }
        else if (!tmp->is_init && !tmp->is_checked && !is_param_decl)
        {
            warning("l'identificateur ‘~m%s~E‘ n'est pas initialisé", t->id.name);
        }
        tmp->is_checked = 1;
        break;
    case b_op_type:
        second_turn_semantic(t->b_op.l_memb, t);
        second_turn_semantic(t->b_op.r_memb, t);
        break;
    case u_op_type:
        second_turn_semantic(t->u_op.child, t);
        break;
    case affect_type:
        second_turn_semantic(t->affect.expr, t);
        second_turn_semantic(t->affect.id, t);
        break;
    case instr_type:
        second_turn_semantic(t->list_instr.instr, t);
        second_turn_semantic(t->list_instr.next, t);
        break;
    case while_type:
        second_turn_semantic(t->while_n.expr, t);
        second_turn_semantic(t->while_n.list_instr, t);
        break;
    case do_while_type:
        second_turn_semantic(t->while_n.expr, t);
        second_turn_semantic(t->while_n.list_instr, t);
        break;
    case if_type:
        second_turn_semantic(t->if_n.expr, t);
        second_turn_semantic(t->if_n.list_instr1, t);
        second_turn_semantic(t->if_n.list_instr2, t);
        break;
    case for_type:
        second_turn_semantic(t->for_n.affect_init, t);
        second_turn_semantic(t->for_n.end_exp, t);
        second_turn_semantic(t->for_n.id, t);
        second_turn_semantic(t->for_n.list_instr, t);
        break;
    case decla_type:
        second_turn_semantic(t->decla_list.decla, t);
        second_turn_semantic(t->decla_list.next, t);
        break;
    case var_decla_type:
        second_turn_semantic(t->var_decla.expr, t);
        second_turn_semantic(t->var_decla.next, t);
        second_turn_semantic(t->var_decla.var, t);
        break;
    case prog_type:
        if (t->root.list_decl != NULL) offset_cdln = t->root.list_decl->codelen;
        second_turn_semantic(t->root.list_decl, t);
        second_turn_semantic(t->root.main_prog, t);
        break;
    case func_decla_type:
        id = t->func_decla.id->id.name;
        strcpy(old_context, current_ctx);
        strcpy(current_ctx, id);
        
        
        /*
         * Calcul de l'adresse de la fonction.
         * 7 pour ramOs et 1 pour le JUMP avant la fonction
         */
        size_t adr = offset_cdln + 7 + 1;
        adr -= t->codelen;
        if (parent->decla_list.next != NULL)
        {
            adr -= parent->decla_list.next->codelen;
        }

        if (strcmp(id, "PROGRAMME") == 0) adr = offset_cdln + 7;

        tmp = get_symbol(table, current_ctx, id);
        tmp->adr = adr;
        t->mem_adr = adr;

        is_param_decl = 1;
        second_turn_semantic(t->func_decla.params, t);
        is_param_decl = 0;

        second_turn_semantic(t->func_decla.id, t);
        second_turn_semantic(t->func_decla.list_decl, t);
        second_turn_semantic(t->func_decla.list_instr, t);
        strcpy(current_ctx, old_context);
        break;
    case func_call_type:
        id = t->func_call.func_id->id.name;
        tmp = get_symbol(table, current_ctx, id);
        set_error_info(t->proto.id->pos_infos);
        if (!tmp->is_init)
        {
            fatal_error("la fonction ‘~r%s~E‘ n'est pas initialisée", id);
            exit(1);
        }

        second_turn_semantic(t->func_call.func_id, t);
        second_turn_semantic(t->func_call.params, t);
        break;
    case return_type:
        second_turn_semantic(t->return_n.expr, t);
        break;
    case exp_list_type:
        second_turn_semantic(t->exp_list.exp, t);
        second_turn_semantic(t->exp_list.next, t);
        break;
    case io_type:
        second_turn_semantic(t->io.expr, t);
        break;
    case array_access_type:
        second_turn_semantic(t->arr_access.affect_expr, t);
        second_turn_semantic(t->arr_access.id, t);
        second_turn_semantic(t->arr_access.ind_expr, t);
        break;
    case alloc_type:
        second_turn_semantic(t->alloc.expr, t);
        second_turn_semantic(t->alloc.id, t);
        break;
    case proto_type:
        id = t->proto.id->id.name;
        tmp = get_symbol(table, current_ctx, id);
        set_error_info(t->pos_infos);
        if (!tmp->is_init && !tmp->is_checked)
        {
            warning("la fonction ‘~m%s~E‘ n'est pas initialisée", id);
        }
        tmp->is_checked = 1;
        break;
    default:
        break;
    }
}



/**
 * @brief Réalise l'analyse sémantique pour les feuilles nombres.
 * Vérifie que l'entier soit bien sur 16 bits et afficher un warning
 * si ça n'est pas le cas.
 * 
 * @param t 
 */
void semantic_nb(ast *t)
{
    if (t->nb.val > SHRT_MAX || t->nb.val < SHRT_MIN)
    {
        set_error_info(t->pos_infos);
        warning("le nombre ~B%d~E dépasse la valeur maximale d'un entier",
                t->nb.val);
    }

    /* 1 LOAD seulement */
    t->codelen = 1;
}



void semantic_id(ast *t)
{    
    /* On vérifie que l'identifiant existe */
    set_error_info(t->pos_infos);
    symbol *tmp = get_symbol(table, current_ctx, t->id.name);
    tmp->is_used = 1;


    if (tmp->type == array) t->codelen = tmp->mem_zone == 's' ? 2 : 1;
    else t->codelen = tmp->mem_zone == 's' ? 4 : 1;
}


void semantic_b_op(ast *t)
{
    semantic(t->b_op.l_memb);
    semantic(t->b_op.r_memb);

    t->codelen = t->b_op.l_memb->codelen + t->b_op.r_memb->codelen;

    /* La taille dépend de l'opérateur */
    switch (t->b_op.ope)
    {
    
    case AND_OP: 
        t->codelen += 5; 
        break;

    /* Tout ceux qui coûtent 8 instructions */
    case '<':
    case '>':
    case '=':
    case NE_OP:
    case OR_OP: 
        t->codelen += 8; 
        break;
    
    /* Ceux qui coûtent 9 instructions */
    case LE_OP:
    case GE_OP:
        t->codelen += 9;
        break;
    default:
        /* Tous les autres ajoutent 4 instructions */
        t->codelen += 4;
        break;
    }
}



void semantic_u_op(ast *t)
{
    u_op_node node = t->u_op;
    semantic(node.child);
    t->codelen = node.child->codelen;

    symbol *tmp;
    switch (node.ope)
    {
    case NOT_OP:
        t->codelen += 4;
        break;
    case '-':
        t->codelen += 1;
        break;
    case '@':
        set_error_info(node.child->pos_infos);
        tmp = get_symbol(table, current_ctx, node.child->id.name);

        if (tmp->mem_zone == 's')
        {
            t->codelen = 2;
        }
        else t->codelen = 1;
        break;
    case '*':
        set_error_info(node.child->pos_infos);
        tmp = get_symbol(table, current_ctx, node.child->id.name);

        if (tmp->type != pointer)
        {
            warning("le symbole ‘~B%s~E‘ n'est pas un pointeur");
        }

        if (tmp->mem_zone == 's') t->codelen = 4;
        else t->codelen = 1;
        break;
    default:
        break;
    }
}



void semantic_instr(ast *t)
{
    instr_node node = t->list_instr;
    semantic(node.instr);
    t->codelen = node.instr->codelen;

    /* Si c'est un RETOURNER on s'arrête là */
    if (node.instr->type == return_type)
    {
        if (node.next != NULL)
        {
            set_error_info(node.instr->pos_infos);
            warning("les instructions situées apprès le ~BRETOURNER~E ne "\
                    "seront jamais exécutées");
        }
        return;
    }

    /* Sinon on continue normalement */
    semantic(node.next);
    if (node.next != NULL) t->codelen += node.next->codelen;
}


void semantic_while(ast *t)
{
    while_node node = t->while_n;

    semantic(node.expr);
    semantic(node.list_instr);

    t->codelen = node.expr->codelen + node.list_instr->codelen + 2;
}



void semantic_do_while(ast *t)
{
    do_while_node node = t->do_while;

    semantic(node.list_instr);
    semantic(node.expr);

    t->codelen = node.expr->codelen + node.list_instr->codelen + 2;
}



void semantic_if(ast *t)
{
    if_node node = t->if_n;

    semantic(node.expr);
    semantic(node.list_instr1);
    semantic(node.list_instr2);

    t->codelen = node.list_instr1->codelen + node.expr->codelen + 2;
    if (node.list_instr2 != NULL) t->codelen += node.list_instr2->codelen;
}


void semantic_for(ast *t)
{
    for_node node = t->for_n;
    
    // ToDo: rendre possible la déclaration dans la boucle ?
    semantic(node.affect_init);
    semantic(node.end_exp);
    semantic(node.list_instr);

    set_error_info(node.id->pos_infos);
    symbol *tmp = get_symbol(table, current_ctx, node.id->id.name);

    int cost = tmp->mem_zone == 's' ? 6 : 3;
    t->codelen = node.affect_init->codelen + node.end_exp->codelen\
                 + node.list_instr->codelen + cost;
}



void semantic_affect(ast *t)
{
    affect_node node = t->affect;

    semantic(node.expr);
    semantic(node.id);

    set_error_info(node.id->pos_infos);
    symbol *tmp = get_symbol(table, current_ctx, node.id->id.name);

    /* Si déjà init alors il est modifié */
    tmp->is_modified = tmp->is_init ? 1 : tmp->is_modified;
    tmp->is_init = 1;

    int cost = tmp->mem_zone == 's' ? 6 : 1;
    t->codelen = node.expr->codelen + cost;
    if (node.is_deref && tmp->mem_zone == 's') t->codelen += 1;
}



void semantic_io(ast *t)
{
    io_node node = t->io;
    semantic(node.expr);

    t->codelen = 1;                 /* READ ou WRITE */
    if (node.mode == 'r') return;   /* Rien d'autre à faire pour LIRE */

    t->codelen += node.expr->codelen;
}


void semantic_decla(ast *t)
{
    decla_node node = t->decla_list;
    semantic(node.decla);
    semantic(node.next);

    t->codelen = node.decla->codelen;
    if (node.next != NULL) t->codelen += node.next->codelen;
}


static void semantic_int_decla(ast *t)
{
    var_decla_node node = t->var_decla;

    /*
     * Pour les déclarations de variables, si le contexte n'est pas
     * global alors l'allocation se fait dans la pile (hors allocation
     * dynamique mais sera géré par ALLOUER).
     */
    char zone = strcmp(current_ctx, "global") == 0 ? 'h' : 's';

    t->codelen = 1;

    int adr;
    int stack_instr = 0;
    if (zone == 'h')
    {
        adr = node.var->mem_adr = STATIC_START + static_rel_adr++;
    }
    else if (zone == 's')
    {
        adr = node.var->mem_adr = stack_rel_adr++;
        stack_instr = 5;
    }
    
    char *id = node.var->id.name;

    set_error_info(node.var->pos_infos);
    symbol *new_symb = init_symbol(id, adr, zone, integer);
    add_symbol(table, current_ctx, new_symb);

    semantic(node.expr);
    semantic(node.next);
    semantic(node.var);

    new_symb->is_used = 0;

    if (node.expr != NULL) new_symb->is_init = 1;


    if (node.expr != NULL) t->codelen += node.expr->codelen + 1 + stack_instr;
    if (node.next != NULL) t->codelen += node.next->codelen;
}


static void semantic_ptr_decla(ast *t)
{
    var_decla_node node = t->var_decla;
    char zone = strcmp(current_ctx, "global") == 0 ? 'h' : 's';

    int adr;
    if (zone == 'h')
    {
        adr = node.var->mem_adr = STATIC_START + static_rel_adr++;
    }
    else if (zone == 's')
    {
        adr = node.var->mem_adr = stack_rel_adr++;
    }
    
    char *id = node.var->id.name;
    set_error_info(node.var->pos_infos);

    symbol *new_symb = init_symbol(id, adr, zone, pointer);
    add_symbol(table, current_ctx, new_symb);

    semantic(node.expr);
    semantic(node.next);
    semantic(node.var);

    new_symb->is_used = 0;

    t->codelen = 1;
    if (node.next != NULL) t->codelen += node.next->codelen;
}


static void semantic_arr_decla(ast *t)
{
    var_decla_node node = t->var_decla;
    array_decla_node arr_node = node.var->arr_decla;

    char zone = strcmp(current_ctx, "global") == 0 ? 'h' : 's';

    int adr;
    if (zone == 'h')
    {
        adr = node.var->mem_adr = STATIC_START + static_rel_adr;
        static_rel_adr += arr_node.size > 1 ? arr_node.size : 1;
    }
    else if (zone == 's')
    {
        stack_rel_adr += arr_node.size > 1 ? arr_node.size : 1;
        adr = node.var->mem_adr = stack_rel_adr;
    }
    
    char *id = arr_node.id->id.name;
    symbol *new_symb = init_symbol(id, adr, zone, node.type);
    new_symb->size = arr_node.size;
    add_symbol(table, current_ctx, new_symb);

    semantic(arr_node.list_expr);
    semantic(arr_node.id);
    semantic(node.next);

    new_symb->is_used = 0;


    /* On compte le nombre d'éléments qui initialisent le tableau */
    int size = 0;
    ast *aux = arr_node.list_expr;
    while (aux != NULL)
    {
        size++;
        aux = aux->exp_list.next;
    }

    if (arr_node.size != -1 && (arr_node.size != size) && arr_node.list_expr != NULL)
    {
        set_error_info(arr_node.list_expr->pos_infos);
        fatal_error("mauvaise taille lors de l'initialisation du tableau "\
        "~U%s~E: le tableau est de taille ~B%d~E (initialisé avec un tableau "\
        "de taille ~B%d~E)", id, arr_node.size, size);
        exit(1);
    }


    t->codelen = 0;
    if (new_symb->mem_zone == 's') t->codelen += 3;
    else if (arr_node.list_expr == NULL) t->codelen += 3;

    if (arr_node.list_expr != NULL)
    {
        new_symb->is_init = 1;
        t->codelen += arr_node.list_expr->codelen + 2 * size;      // !
    }


    if (node.next != NULL) t->codelen += node.next->codelen;
}



void semantic_var_decla(ast *t)
{    
    var_decla_node node = t->var_decla;

    switch (node.type)
    {
    case integer:
        semantic_int_decla(t);
        break;
    case array:
        semantic_arr_decla(t);
        break;
    case pointer:
        semantic_ptr_decla(t);
        break;
    default:
        break;
    }
}



/**
 * @brief 
 * 
 * @param t 
 */
void semantic_func_decla(ast *t)
{
    func_decla_node node = t->func_decla;

    /* Le champs adr contiendra le nombre de paramètres */
    int n = node.nb_params;
    

    

    set_error_info(node.id->pos_infos);
    /*
     * On ajoute le nom de la fonction en tant qu'identifiant si nécessaire.
     * En effet, il est peut-être déjà dans la table des symboles si un 
     * prototype a été mit.
     */
    symbol *tmp = search_symbol(table, current_ctx, node.id->id.name);
    if (tmp == NULL)
    {
        symbol *new_symb = init_symbol(node.id->id.name, 0, 'h', func);
        new_symb->size = n;
        new_symb->is_init = 1;
        tmp = add_symbol(table, current_ctx, new_symb);
    }
    tmp->is_init = 1;

    semantic(t->func_decla.id);

    /* On change le contexte qui devient le nom de la fonction */
    strcpy(old_context, current_ctx);
    strcpy(current_ctx, node.id->id.name);
    add_context(table, current_ctx);

    /*
     * À la fin de son exécution, la fonction doit rendre la pile dans
     * l'état dans lequel elle était au début de son exécution
     */
    int old_stack_rel_adr = stack_rel_adr;

    /*
     * 1 car l'adresse relative 0 est réservée à une éventuelle adresse
     * de retour dans les fonctions autres que la fonction principale.
     */
    stack_rel_adr = strcmp(node.id->id.name, "PROGRAMME") == 0 ? 0 : 1;

    has_return_instr = 0;
    
    semantic(node.params);
    semantic(node.list_decl);
    semantic(node.list_instr);

    /* Si pas de "RETOURNER" spécifié on créé un noeud vide qui renverra 0 */
    if (!has_return_instr && strcmp(node.id->id.name, "PROGRAMME") != 0)
    {
        ast *return_n = create_return_node(NULL);

        /* On l'ajoute à la fin des instructions et on réanalyse */
        node.list_instr = create_instr_node(return_n, node.list_instr);
        semantic(node.list_instr);
    }

    t->codelen = 0;

    if (node.list_decl != NULL) t->codelen += node.list_decl->codelen;
    if (node.list_instr != NULL) t->codelen += node.list_instr->codelen;

    /* Au cas où ça change + tard (STOP pour PROGRAMME, JUMP pour les autre) */
    if (strcmp(node.id->id.name, "PROGRAMME") == 0) t->codelen += 1;
    else t->codelen += 1;

    /* On revient au contexte précédent */
    strcpy(current_ctx, old_context);
    stack_rel_adr = old_stack_rel_adr;
}



void semantic_func_call(ast *t)
{
    func_call_node node = t->func_call;
    semantic(node.func_id);

    /* On vérifie que la fonction existe */
    symbol *tmp = get_symbol(table, current_ctx, node.func_id->id.name);

    /* On compte le nombre de paramètres */
    int nb_params = 0;
    ast *aux = node.params;
    while (aux != NULL)
    {
        nb_params++;
        aux = aux->exp_list.next;
    }

    /* On vérifie que le nombre de paramètre est le bon */
    if (nb_params != tmp->size)
    {
        if (node.params != NULL) set_error_info(node.params->pos_infos);
        else set_error_info(node.func_id->pos_infos);
        fatal_error("nombre de paramètres incorrect (%d passés, %d attendus)",
                    nb_params, tmp->size);
        exit(1);
    }
    
    /* Analyse sémantique des paramètres passés */
    semantic(node.params);

    /*
     * Codelen: 1 mise à jour de STACK_REL_START, 1 JUMP vers le code
     * de la fonction, et re mise en l'état initial de STACK_REL_START.
     * 
     * On ajoute également le coût de la copie des paramètres dans la
     * pile. On va empiler les paramètres à la suite dans la pile.
     */
    t->codelen = 24 + 2 * nb_params;
    if (node.params != NULL) t->codelen += node.params->codelen;
}


void semantic_return(ast *t)
{
    return_node node = t->return_n;
    has_return_instr = 1;

    /*
     * On ne peut retourner que depuis une fonction autre que la
     * fonction principale PROGRAMME
     */
    if (strcmp(current_ctx, "global") == 0)
    {
        set_error_info(t->pos_infos);
        fatal_error("~BRETOURNER~E utilisé en dehors d'une fonction");
        exit(1);
    }

    

    if (node.expr != NULL)
    {
        semantic(node.expr);
        t->codelen = node.expr->codelen;
    }
    else t->codelen = 1;   
    
    if (strcmp(current_ctx, "PROGRAMME") == 0) t->codelen += 1;
    else t->codelen += 10 + PUSH_COST;
}


void semantic_exp_list(ast *t)
{
    exp_list_node node = t->exp_list;
    semantic(node.exp);
    semantic(node.next);

    t->codelen = node.exp->codelen;
    if (node.next != NULL) t->codelen += node.next->codelen;
}



void semantic_arr_access(ast *t)
{
    array_access_node node = t->arr_access;
    semantic(node.id);
    semantic(node.ind_expr);
    semantic(node.affect_expr);

    set_error_info(node.id->pos_infos);
    symbol *tmp = get_symbol(table, current_ctx, node.id->id.name);
    if (tmp->type != array && tmp->type != pointer)
    {
        fatal_error("Impossible d'utiliser l'opérateur ~B[]~E");
        exit(1);
    }

    if (node.affect_expr != NULL) tmp->is_init = 1;

    t->codelen = node.ind_expr->codelen;
    if (tmp->type == array) t->codelen += (tmp->mem_zone == 's') ? 4 : 1;
    else t->codelen += (tmp->mem_zone == 's') ? 5 : 3;

    if (node.affect_expr != NULL) t->codelen += node.affect_expr->codelen + 8;
    else t->codelen += 1;
}


void semantic_alloc(ast *t)
{
    alloc_node node = t->alloc;
    semantic(node.id);
    semantic(node.expr);

    set_error_info(node.id->pos_infos);
    symbol *tmp = get_symbol(table, current_ctx, node.id->id.name);
    if (tmp->type != pointer)
    {
        symb_to_img(table, "table", "png");
        fatal_error("Impossible d'allouer de la mémoire à ~B%s~E "\
                    "car ça n'est pas un pointeur.", node.id->id.name);
        exit(1);
    }

    tmp->is_init = 1;

    t->codelen = node.expr->codelen + 4;
    if (tmp->mem_zone == 's') t->codelen += 5;
    else t->codelen += 2;
}


void semantic_proto(ast *t)
{
    proto_node node = t->proto;

    set_error_info(node.id->pos_infos);

    /* Si déjà déclaré, on ignore */
    symbol *tmp = search_symbol(table, current_ctx, node.id->id.name);
    if (tmp == NULL)
    {
        symbol *new_symb = init_symbol(node.id->id.name, 0, 'h', func);
        new_symb->size = node.nb_params;

        /* Ajout dans la table des symboles */
        add_symbol(table, current_ctx, new_symb);
    }
}

