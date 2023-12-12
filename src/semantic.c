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
    case func_type:
        semantic_function(t);
        break;
    case print_type:
        semantic(t->print.expr);
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
    get_symbol(table, current_ctx, t->id.name);

    /* Un LOAD (les STORE seront gérés dans les affectations) */
    t->codelen = 1;
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
    
    default:
        break;
    }
}



void semantic_instr(ast *t)
{
    instr_node node = t->list_instr;
    semantic(node.instr);
    semantic(node.next);

    t->codelen = node.instr->codelen;
    if (node.next != NULL) t->codelen += node.next->codelen;
}



void semantic_affect(ast *t)
{
    semantic(t->affect.expr);
    semantic(t->affect.id);

    t->codelen = t->affect.expr->codelen + 1;
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

    /* Temporaire: l'adresse est celle dans le tas */
    // ToDo: adresse en fonction du contexte (tas ou pile)
    int adr;
    if (zone == 'h')
    {
        adr = node.id->mem_adr = HEAP_START + heap_rel_adr++;
    }
    else if (zone == 's')
    {
        adr = node.id->mem_adr = STACK_START - stack_rel_adr++;
    }
    

    symbol *new_symb = init_symbol(node.id->id.name, adr, zone, integer);
    add_symbol(table, current_ctx, new_symb);
    semantic(node.expr);
    semantic(node.next);
    semantic(node.id);

    /* 1 pour la gestion de la modification du ptr de pile/tas */
    t->codelen = 1;
    /* + 1 pour le STORE */
    if (node.expr != NULL) t->codelen += node.expr->codelen + 1;
    if (node.next != NULL) t->codelen += node.next->codelen;

    // printf("Déclaration de %s: %ld instructions RAM\n", node.id->id.name, t->codelen);
}



/**
 * @brief 
 * 
 * @param t 
 */
void semantic_function(ast *t)
{
    func_node node = t->function;

    /* On ajoute l'id de la fonction au contexte actuel */
    symbol *new_symb = init_symbol(node.id->id.name, 0, 'h', func);
    add_symbol(table, current_ctx, new_symb);
    semantic(t->function.id);

    /* On change le contexte qui devient le nom de la fonction */
    strcpy(old_context, current_ctx);
    strcpy(current_ctx, node.id->id.name);
    add_context(table, current_ctx);

    /*
     * À la fin de son exécution, la fonction doit rendre la pile dans
     * l'état dans lequel elle était au début de son exécution
     */
    int old_stack_rel_adr = stack_rel_adr;


    semantic(node.params);
    semantic(node.list_decl);
    semantic(node.list_instr);

    t->codelen = 0;

    if (node.params != NULL) t->codelen += node.params->codelen;
    if (node.list_decl != NULL) t->codelen += node.list_decl->codelen;
    if (node.list_instr != NULL) t->codelen += node.list_instr->codelen;

    /* On revient au contexte précédent */
    strcpy(current_ctx, old_context);
    stack_rel_adr = old_stack_rel_adr;
}
