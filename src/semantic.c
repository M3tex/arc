#include "semantic.h"
#include "arc_utils.h"


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
    case affect_type:
        semantic_affect(t);
        break;
    case instr_type:
        semantic(t->list_instr.instr);
        semantic(t->list_instr.next);
        break;
    case var_decla_type:
        semantic(t->list_var_decla.init_var);
        semantic(t->list_var_decla.next);
        break;
    case var_init_type:
        add_symbol(table, current_ctx, t->var_init.id->id.name, 1, integer);
        semantic(t->var_init.next);
        break;
    case prog_type:
        semantic(t->root.list_decl);
        semantic(t->root.main_prog);
        break;
    default:
        break;
    }
}


void semantic_nb(ast *t)
{
    t->codelen = 1;
}


void semantic_id(ast *t)
{
    /* On cherche dans le context local, et sinon dans le contexte global */
    if (search_symbol(table, current_ctx, t->id.name) == NULL)
    {
        if (search_symbol(table, "global", t->id.name) == NULL)
        {
            colored_error(RED|BOLD, 0, "erreur fatale:");
            print_error(0, " l'identificateur ‘");
            colored_error(BOLD, 0, "%s", t->id.name);
            print_error(1, "‘ n'est pas déclaré\n");
        }
    }
}


void semantic_b_op(ast *t)
{
    semantic(t->b_op.l_memb);
    semantic(t->b_op.r_memb);
}


void semantic_affect(ast *t)
{
    semantic(t->affect.expr);
    semantic(t->affect.id);
}