#include "codegen.h"
#include "arc_utils.h"
#include "symbol_table.h"
#include <string.h>


extern symb_table table;
static char c_context[32] = "global";
static char old_context[32];


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
    int offset = 13 - (strlen(instr_str));

    switch (instr)
    {
    case READ:
    case WRITE:
    case STOP:
    case NOP:
        sprintf(buff, "%s %%-%dc;\n", instr_str, offset + 1);
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
    /* ToDo: Inclure + proprement ram_OS ? */
    add_instr(LOAD, '#', STACK_START);
    add_instr(STORE, ' ', STACK_REG);
    add_instr(LOAD, '#', HEAP_START);
    add_instr(STORE, ' ', HEAP_REG);
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
    case id_type:
        codegen_id(t);
        break;
    case instr_type:
        codegen(t->list_instr.instr);
        codegen(t->list_instr.next);
        break;
    case affect_type:
        codegen_affect(t);
        break;
    case prog_type:
        codegen(t->root.list_decl);
        codegen(t->root.main_prog);
        break;
    case func_type:
        /* On change le contexte qui devient le nom de la fonction */
        strcpy(old_context, c_context);
        strcpy(c_context, t->function.id->id.name);

        codegen(t->function.params);
        codegen(t->function.list_decl);
        codegen(t->function.list_instr);

        strcpy(c_context, old_context);
        break;
    case decla_type:
        codegen(t->decla_list.decla);
        codegen(t->decla_list.next);
        break;
    case var_decla_type:
        codegen_var_decla(t);
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
    case '>':
        codegen_gt(t);
        break;
    case '<':
        codegen_lt(t);
        break;
    case '=':
        codegen_eq(t);
        break;
    case NE_OP:
        codegen_ne(t);
        break;
    case LE_OP:
        codegen_le(t);
        break;
    case GE_OP:
        codegen_ge(t);
        break;
    case '%':
    case '+':
    case '*':
    case '/':
    case '-':
        codegen(t->b_op.r_memb);
        add_instr(DEC, ' ', STACK_REG);
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
        add_instr(INC, ' ', STACK_REG);
        break;
    case '-':
        add_instr(SUB, '@', STACK_REG);
        add_instr(INC, ' ', STACK_REG);
        break;
    case '*':
        add_instr(MUL, '@', STACK_REG);
        add_instr(INC, ' ', STACK_REG);
        break;
    case '/':
        add_instr(DIV, '@', STACK_REG);
        add_instr(INC, ' ', STACK_REG);
        break;
    case '%':
        add_instr(MOD, '@', STACK_REG);
        add_instr(INC, ' ', STACK_REG);
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



void codegen_id(ast *t)
{
    symbol *tmp = get_symbol(table, c_context, t->id.name);

    char adr_type = tmp->type == pointer ? '@' : ' ';
    add_instr(LOAD, adr_type, tmp->adr);
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
    add_instr(JUMZ, ' ', nb_instr + 2);

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


/**
 * @brief Génère le code RAM correspondant à l'opérateur <
 * 
 * @param t 
 */
void codegen_lt(ast *t)
{
    /*
     * Si on souhaite vérifier si a < b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si inférieur (strictement) à 0: a < b */
    add_instr(JUML, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    add_instr(LOAD, '#', 1);
}


/**
 * @brief Génère le code RAM correspondant à l'opérateur >
 * 
 * @param t 
 */
void codegen_gt(ast *t)
{
    /*
     * Si on souhaite vérifier si a > b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si supérieur (strictement) à 0: a > b */
    add_instr(JUMG, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    add_instr(LOAD, '#', 1);
}


/**
 * @brief Génère le code RAM correspondant à l'opérateur = (comparaison)
 * 
 * @param t 
 */
void codegen_eq(ast *t)
{
    /*
     * Si on souhaite vérifier si a = b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si égal à 0: a = b */
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    add_instr(LOAD, '#', 1);
}


/**
 * @brief Génère le code RAM correspondant à l'opérateur !=
 * 
 * @param t 
 */
void codegen_ne(ast *t)
{
    /*
     * Si on souhaite vérifier si a = b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si différent de 0: a != b */
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 1);
    add_instr(JUMP, ' ', nb_instr + 2);
    add_instr(LOAD, '#', 0);
}


/**
 * @brief Génère le code RAM correspondant à l'opérateur >=
 * 
 * @param t 
 */
void codegen_ge(ast *t)
{
    /*
     * Si on souhaite vérifier si a >= b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si >= à 0: a >= b */
    add_instr(JUMG, ' ', nb_instr + 4);

    /* Si JUMG faux il faut quand même vérifier si c'est égal à 0 */
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);

    add_instr(LOAD, '#', 1);
}



/**
 * @brief Génère le code RAM correspondant à l'opérateur <=
 * 
 * @param t 
 */
void codegen_le(ast *t)
{
    /*
     * Si on souhaite vérifier si a <= b il suffit de regarder le signe
     * de a - b.
     * On commence donc par générer le membre de droite, qu'on viendra
     * soustraire au membre de gauche.
     */
    codegen(t->b_op.r_memb);

    /* On stocke dans un registre temporaire */
    add_instr(STORE, ' ', TMP_REG_1);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_1);

    /* Si <= à 0: a <= b */
    add_instr(JUML, ' ', nb_instr + 4);

    /* Si JUML faux il faut quand même vérifier si c'est égal à 0 */
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    
    add_instr(LOAD, '#', 1);
}



void codegen_affect(ast *t)
{
    affect_node node = t->affect;

    /* On évalue l'expression et on la stocke à la bonne adresse */
    codegen(node.expr);
    symbol *tmp = get_symbol(table, c_context, node.id->id.name);

    /* Si pointeur on utilise l'adressage indirect */
    char adr_type = tmp->type == pointer ? '@' : ' ';
    add_instr(STORE, adr_type, tmp->adr);
}



void codegen_var_decla(ast *t)
{
    var_decla_node node = t->var_decla;

    symbol *tmp = get_symbol(table, c_context, node.id->id.name);
    /*
     * S'il y a une expression, il faut initialiser la variable.
     * On commence par générer le code pour l'expression puis on stocke
     * le résultat à la bonne adresse.
     */
    if (node.expr != NULL)
    {
        codegen(node.expr);

        /* Si pointeur on utilise l'adressage indirect */
        char adr_type = tmp->type == pointer ? '@' : ' ';
        add_instr(STORE, adr_type, tmp->adr);

        // if (tmp->mem_zone == 'h')
        // {
        //     add_instr(STORE, ' ', tmp->adr + HEAP_START);
        //     tmp->adr += HEAP_START;
        // }
        // else if (tmp->mem_zone == 's')
        // {
        //     add_instr(DEC, ' ', STACK_REG);
        //     add_instr(STORE, '@', STACK_REG);
        //     /* ToDo: Demander au prof pour l'adresse */
        // }
    }

    /* MAJ du tas / pile */
    if (tmp->mem_zone == 'h') add_instr(INC, ' ', HEAP_REG);
    else if (tmp->mem_zone == 's') add_instr(DEC, ' ', STACK_REG);

    /* On génère le code pour la déclaration de variable suivante */
    codegen(node.next);
}