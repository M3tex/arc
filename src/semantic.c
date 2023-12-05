#include "semantic.h"



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
    /* ToDo: demander au prof */
    // ! pour l'instant tout en global
    add_symbol(table, "global", t->id.name, 1, integer);    // TEMPORAIRE
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