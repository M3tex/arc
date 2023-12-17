#include "semantic.h"
#include "arc_utils.h"
#include "ram_os.h"
#include <string.h>
#include <limits.h>


/**
 * Adresses relatives.
 * Par exemple, heap_rel_adr est l'adresse relative par rapport au tas
 * (0 pour la 1ère variable stockée dans le tas, 1 pour la deuxième,
 * etc.).
 * Idem pour la pile, à l'exception que l'adresse relative sera "reset"
 * à chaque fin de fonction.
 */
static int heap_rel_adr = 0;
static int stack_rel_adr = 0;


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
        colored_error(MAGENTA|BOLD, 0, "warning:");
        print_error(0, " le nombre ");
        colored_error(BOLD, 0, "%d", t->nb.val);
        print_error(0, " dépasse la valeur maximale d'un entier.\n");
    }

    /* 1 LOAD seulement */
    t->codelen = 1;
}



void semantic_id(ast *t)
{
    /*
     * Les actions dépendent du noeud parent et du contexte.
     * Par exemple, l'id  a dans l'expression 3 * a + 1 aura un codelen 
     * de 1 (il suffit simplement de faire un LOAD de la bonne adresse).
     * En revanche, lors d'une déclaration, par exemple VAR x en dehors
     * d'une fonction, il faudra en + augmenter le pointeur du tas (et
     * réciproquement si la variable est déclarée dans une fonction il
     * faudra mettre à jour la pile).
     * 
     * 
     * On ne met que 1 ici car ces diverses opérations seront prises en
     * compte dans le codelen des parents.
     * 
     * semantic_id ne doit être appelé que lorsqu'il faut charger la
     * valeur de l'identificateur.
     * 
     */
    
    /* On vérifie que l'identifiant existe */
    symbol * tmp = get_symbol(table, current_ctx, t->id.name);

    /* Un LOAD (les STORE seront gérés dans les affectations) */
    t->codelen = tmp->mem_zone == 's' ? 4 : 1;
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

    /* Tout ceux qui coûtent 6 instructions */
    case '<':
    case '>':
    case '=':
    case NE_OP:
    case OR_OP: 
        t->codelen += 6; 
        break;
    
    /* Ceux qui coûtent 7 instructions */
    case LE_OP:
    case GE_OP:
        t->codelen += 7;
        break;
    default:
        /* Tous les autres ajoutent 4 instructions */
        t->codelen += 4;
        break;
    }
}



void semantic_u_op(ast *t)
{
    semantic(t->u_op.child);
    t->codelen = t->u_op.child->codelen;
    switch (t->u_op.ope)
    {
    case NOT_OP:
        t->codelen += 4;
        break;
    case '-':
        t->codelen += 1;
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
            colored_error(MAGENTA|BOLD, 0, "warning:");
            print_error(0, " les instructions après le RETOURNER ne seront jamais exécutées\n");
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



void semantic_affect(ast *t)
{
    affect_node node = t->affect;

    semantic(node.expr);
    semantic(node.id);

    symbol * tmp = get_symbol(table, current_ctx, node.id->id.name);

    int cost = tmp->mem_zone == 's' ? 6 : 1;
    t->codelen = node.expr->codelen + cost;
}



void semantic_io(ast *t)
{
    io_node node = t->io;
    semantic(node.expr);

    t->codelen = 1;     /* READ ou WRITE */
    if (node.mode == 'r') return;

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



void semantic_var_decla(ast *t)
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
        adr = node.id->mem_adr = HEAP_START + heap_rel_adr++;
    }
    else if (zone == 's')
    {
        adr = node.id->mem_adr = stack_rel_adr++;
        stack_instr = 5;
    }


    symbol *new_symb = init_symbol(node.id->id.name, adr, zone, integer);
    add_symbol(table, current_ctx, new_symb);
    semantic(node.expr);
    semantic(node.next);
    semantic(node.id);


    /* + 2 pour le STORE et la MAJ du tas/pile */
    if (node.expr != NULL) t->codelen += node.expr->codelen + 1 + stack_instr;
    if (node.next != NULL) t->codelen += node.next->codelen;

    // printf("Déclaration de %s: %ld instructions RAM\n", node.id->id.name, t->codelen);
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

    /* On ajoute l'id de la fonction au contexte actuel */
    symbol *new_symb = init_symbol(node.id->id.name, 0, 'h', func);
    new_symb->size = n;
    add_symbol(table, current_ctx, new_symb);
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

    new_symb->has_return = has_return_instr;


    t->codelen = 0;

    // if (node.params != NULL) t->codelen += node.params->codelen;
    if (node.list_decl != NULL) t->codelen += node.list_decl->codelen;
    if (node.list_instr != NULL) t->codelen += node.list_instr->codelen;

    if (strcmp(node.id->id.name, "PROGRAMME") == 0) t->codelen += 1;
    else 
    {
        t->codelen += 1;
        t->codelen += 12 + 2;
        if (!new_symb->has_return) t->codelen += 1;
    }


    /* On revient au contexte précédent */
    strcpy(current_ctx, old_context);
    stack_rel_adr = old_stack_rel_adr;
}



void semantic_func_call(ast *t)
{
    func_call_node node = t->func_call;

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
        colored_error(RED|BOLD, 0, "erreur fatale:");
        print_error(1, " le nombre de paramètres est incorrect (%d donnés, %d attendus)\n", nb_params, tmp->size);
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
    t->codelen = 7 + node.params->codelen + 2 * nb_params + 3;
}


void semantic_return(ast *t)
{
    return_node node = t->return_n;
    has_return_instr = 1;

    /*
     * On ne peut retourner que depuis une fonction autre que la
     * fonction principale PROGRAMME
     */
    if (strcmp(current_ctx, "global") == 0 
        || strcmp(current_ctx, "PROGRAMME") == 0)
    {
        colored_error(RED|BOLD, 0, "erreur fatale:");
        print_error(1, " instruction 'RETOURNER' utilisée au mauvais endroit\n");
    }

    t->codelen = 1;

    if (node.expr != NULL)
    {
        semantic(node.expr);
        t->codelen = node.expr->codelen;
    }    
}


void semantic_exp_list(ast *t)
{
    exp_list_node node = t->exp_list;
    semantic(node.exp);
    semantic(node.next);

    t->codelen = node.exp->codelen;
    if (node.next != NULL) t->codelen += node.next->codelen;
}