#include "codegen.h"
#include "arc_utils.h"
#include <string.h>



/* Compteur d'instruction. Utilisé pour les JUMP */
static int nb_instr = 0;


/**
 * @brief Ajoute l'instruction au fichier de sortie
 * 
 * @param instr L'instruction à ajouter
 * @param t_adr Le type d'adressage: @ pour indirect, # pour numérique
 * et ' ' pour direct.
 * @param adr  
 */
void add_instr(instr_ram instr, char t_adr, int adr)
{
    static char *instr_to_str[17] = {
        "READ",
        "WRITE",
        "LOAD",
        "STORE",
        "DEC",
        "INC",
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "MOD",
        "JUMP",
        "JUMZ",
        "JUML",
        "JUMG",
        "STOP",
        "NOP"
    };

    char *instr_str = instr_to_str[instr];
    char buff[32];
    int offset = 10 - (strlen(instr_str));

    switch (instr)
    {
    case READ:
    case WRITE:
    case STOP:
    case NOP:
        sprintf(buff, "%s %%-%dc;\n", instr_str, offset);
        fprintf(fp_out, buff, ' ');
        break;
    
    default:
        sprintf(buff, "%s %c%%-%dd;\n", instr_str, t_adr, offset);
        fprintf(fp_out, buff, adr);
        break;
    }
    
    nb_instr++;
}



/**
 * @brief Insère tout le code propre à ram_OS
 * 
 */
void init_ram_os()
{
    /* ToDo: Inclure proprement ram_OS */
    add_instr(LOAD, '#', 100);
    add_instr(STORE, ' ', STACK_REG);
}


void codegen(ast *t)
{
    if (t == NULL) return;
    switch (t->type)
    {
    case nb_type:
        codegen_nb(t);
        break;
    case b_op_type:
        codegen_b_op(t);
        break;
    case u_op_type:
        codegen_u_op(t);
        break;
    case instr_type:
        codegen(t->list_instr.instr);
        codegen(t->list_instr.next);
        break;
    default:
        break;
    }
}


/**
 * @brief Génère le code RAM correspondant au nombre passé en paramètre.
 * 
 * @param t 
 */
void codegen_nb(ast *t)
{
    add_instr(LOAD, '#', t->nb.val);
}



/**
 * @brief Génère le code RAM correspondant au noeud d'opérateur passé
 * en paramètre.
 * 
 * @param t 
 */
void codegen_b_op(ast *t)
{
    /* On gère l'associativité */
    switch (t->b_op.ope)
    {
    case OR_OP:
        codegen_or(t);
        break;
    case AND_OP:
        codegen_and(t);
        break;
    case '%':
    case '+':
    case '*':
    case '/':
    case '-':
        codegen(t->b_op.r_memb);
        add_instr(INC, ' ', STACK_REG);
        add_instr(STORE, '@', STACK_REG);
        codegen(t->b_op.l_memb);
        break;
    
    default:
        break;
    }


    /* On fait ensuite la bonne opération */
    switch (t->b_op.ope)
    {
    case '+':
        add_instr(ADD, '@', STACK_REG);
        add_instr(DEC, ' ', STACK_REG);
        break;
    case '-':
        add_instr(SUB, '@', STACK_REG);
        add_instr(DEC, ' ', STACK_REG);
        break;
    case '*':
        add_instr(MUL, '@', STACK_REG);
        add_instr(DEC, ' ', STACK_REG);
        break;
    case '/':
        add_instr(DIV, '@', STACK_REG);
        add_instr(DEC, ' ', STACK_REG);
        break;
    case '%':
        add_instr(MOD, '@', STACK_REG);
        add_instr(DEC, ' ', STACK_REG);
        break;
    default:
        break;
    }
}




void codegen_u_op(ast *t)
{
    switch (t->u_op.ope)
    {
    case NOT_OP:
        codegen_not(t);
        break;
    default:
        break;
    }
}



/**
 * @brief Génère le code RAM correspondant à un OU
 * 
 * @param t 
 */
void codegen_or(ast *t)
{
    /*
     * On évalue le terme de gauche en 1er.
     * S'il est différent de 0, il est inutile d'évaluer le terme de 
     * droite.
     */
    codegen(t->b_op.l_memb);
    int jumz_adr = t->b_op.r_memb->codelen + nb_instr + 1;
    add_instr(JUMZ, ' ', jumz_adr);

    int jump_adr = t->b_op.r_memb->codelen + nb_instr + 2;
    add_instr(JUMP, ' ', jump_adr);

    /* On évalue seulement si le JUMZ est vrai (i.e ACC = 0) */
    codegen(t->b_op.r_memb);
    add_instr(JUMZ, ' ', nb_instr + 3);

    /* A, cas où l'expr est vraie, on charge 1 et on saute après tout le code */
    add_instr(LOAD, '#', 1);     
    add_instr(JUMP, ' ', nb_instr + 2);

    /* Cas où les 2 sont faux: on charge 0 */
    add_instr(LOAD, '#', 0);           
}



/**
 * @brief Génère le code RAM correspondant à un ET
 * 
 * @param t 
 */
void codegen_and(ast *t)
{
    /*
     * On évalue le terme de gauche en 1er.
     * S'il vaut 0, il est inutile d'évaluer le terme de droite.
     */
    codegen(t->b_op.l_memb);
    int jumz_adr = t->b_op.r_memb->codelen + nb_instr + 4;
    add_instr(JUMZ, ' ', jumz_adr);

    /* On évalue seulement si le JUMZ est faux (i.e ACC != 0) */
    codegen(t->b_op.r_memb);
    add_instr(JUMZ, ' ', nb_instr + 3);

    /* A, cas où l'expr est vraie, on charge 1 et on saute après le LOAD #0 */
    add_instr(LOAD, '#', 1);     
    add_instr(JUMP, ' ', nb_instr + 2);

    /* Cas où les 2 sont faux: on charge 0 */
    add_instr(LOAD, '#', 0);
}


/**
 * @brief Génère le code RAM correspondant à un NON
 * 
 * @param t 
 */
void codegen_not(ast *t)
{
    codegen(t->u_op.child);
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    add_instr(LOAD, '#', 1);
}