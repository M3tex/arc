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
        if(t_adr == ' ') sprintf(buff, "%s %%-%dd;\n", instr_str, offset + 1);
        else sprintf(buff, "%s %c%%-%dd;\n", instr_str, t_adr, offset);
        fprintf(fp_out, buff, adr);
        break;
    }
    
    nb_instr++;
}



/**
 * @brief Fonction permettant d'empiler.
 * Coûte 2 instructions (voir PUSH_COST)
 * 
 */
void push()
{
    add_instr(STORE, '@', STACK_REG);
    add_instr(DEC, ' ', STACK_REG);
}


/**
 * @brief Fonction permettant de dépiler.
 * Coûte 2 instruction (voir POP_COST)
 * 
 */
void pop()
{
    add_instr(INC, ' ', STACK_REG);
    add_instr(LOAD, '@', STACK_REG);
}



/**
 * @brief Fonction permettant d'obtenir la valeur au sommet de la pile.
 * Coûte 1 instruction
 * 
 */
void peek()
{
    add_instr(LOAD, '@', STACK_REG);
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
    add_instr(STORE, ' ', STACK_REL_START);
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
        codegen_instr(t);
        break;
    case affect_type:
        codegen_affect(t);
        break;
    case io_type:
        codegen_io(t);
        break;
    case while_type:
        codegen_while(t);
        break;
    case do_while_type:
        codegen_do_while(t);
        break;
    case for_type:
        codegen_for(t);
        break;
    case if_type:
        codegen_if(t);
        break;
    case prog_type:
        codegen(t->root.list_decl);
        codegen(t->root.main_prog);
        break;
    case func_decla_type:
        codegen_func_decla(t);
        break;
    case func_call_type:
        codegen_func_call(t);
        break;
    case return_type:
        codegen_return(t);
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
        push();
        codegen(t->b_op.l_memb);
        break;
    
    default:
        break;
    }


    /* On fait ensuite la bonne opération */
    switch (t->b_op.ope)
    {
    case '+':
        add_instr(INC, ' ', STACK_REG);
        add_instr(ADD, '@', STACK_REG);
        break;
    case '-':
        add_instr(INC, ' ', STACK_REG);
        add_instr(SUB, '@', STACK_REG);
        break;
    case '*':
        add_instr(INC, ' ', STACK_REG);
        add_instr(MUL, '@', STACK_REG);
        break;
    case '/':
        add_instr(INC, ' ', STACK_REG);
        add_instr(DIV, '@', STACK_REG);
        break;
    case '%':
        add_instr(INC, ' ', STACK_REG);
        add_instr(MOD, '@', STACK_REG);
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
    case '-':
        codegen(t->u_op.child);
        add_instr(MUL, '#', -1);
    default:
        break;
    }
}



void codegen_id(ast *t)
{
    symbol *tmp = get_symbol(table, c_context, t->id.name);

    char adr_type = tmp->type == pointer ? '@' : ' ';

    /* Si dans la pile il faut calculer l'adresse à l'exécution */
    int adr = tmp->adr;
    if (tmp->mem_zone == 's')
    {
        /* Calcul de l'adresse réelle et stockage dans TMP_REG_STK_ADR */
        add_instr(LOAD, ' ', STACK_REL_START);
        add_instr(SUB, '#', adr);
        add_instr(STORE, ' ', TMP_REG_STK_ADR);
        adr_type = '@';
        adr = TMP_REG_STK_ADR;
    }

    add_instr(LOAD, adr_type, adr);
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
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

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

    /* On empile */
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

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
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

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
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

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
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

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
    add_instr(STORE, ' ', TMP_REG_CMP);

    /* Génération du code de l'expression de gauche */
    codegen(t->b_op.l_memb);
    add_instr(SUB, ' ', TMP_REG_CMP);

    /* Si <= à 0: a <= b */
    add_instr(JUML, ' ', nb_instr + 4);

    /* Si JUML faux il faut quand même vérifier si c'est égal à 0 */
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(LOAD, '#', 0);
    add_instr(JUMP, ' ', nb_instr + 2);
    
    add_instr(LOAD, '#', 1);
}



void codegen_instr(ast *t)
{
    instr_node node = t->list_instr;
    codegen(node.instr);
    if (node.instr->type != return_type) codegen(node.next);
}


void codegen_affect(ast *t)
{
    affect_node node = t->affect;

    /* On évalue l'expression et on la stocke à la bonne adresse */
    codegen(node.expr);
    symbol *tmp = get_symbol(table, c_context, node.id->id.name);

    /* Si pointeur on utilise l'adressage indirect */
    char adr_type = tmp->type == pointer ? '@' : ' ';

    /* Si dans la pile il faut calculer l'adresse à l'exécution */
    int adr = tmp->adr;
    if (tmp->mem_zone == 's')
    {
        /* On stocke la valeur de l'ACC */
        add_instr(STORE, ' ', TMP_REG_ACC_SWP);
        add_instr(LOAD, ' ', STACK_REL_START);
        add_instr(SUB, '#', adr);
        add_instr(STORE, ' ', TMP_REG_STK_ADR);

        /* On remet l'ACC dans son état initial */
        add_instr(LOAD, ' ', TMP_REG_ACC_SWP);
        adr_type = '@';
        adr = TMP_REG_STK_ADR;
    }
    add_instr(STORE, adr_type, adr);
}


void codegen_io(ast *t)
{
    io_node node = t->io;
    if (node.mode == 'w')
    {
        codegen(node.expr);
        add_instr(WRITE, ' ', 0);
    }
    else add_instr(READ, ' ', 0);
}



void codegen_while(ast *t)
{
    while_node node = t->while_n;

    int jumb_back = nb_instr;
    codegen(node.expr);

    /* Si l'expression vaut 0 on sort de la boucle */
    int jumz_adr = nb_instr + node.list_instr->codelen + 2;
    add_instr(JUMZ, ' ', jumz_adr);
    codegen(node.list_instr);
    add_instr(JUMP, ' ', jumb_back);
}


void codegen_do_while(ast *t)
{
    do_while_node node = t->do_while;

    int jumb_back = nb_instr;
    codegen(node.list_instr);
    codegen(node.expr);

    /* Si l'expression vaut 0 on sort de la boucle */
    add_instr(JUMZ, ' ', nb_instr + 2);
    add_instr(JUMP, ' ', jumb_back);    /* Sinon retour aux instr */
}


void codegen_for(ast *t)
{
    for_node node = t->for_n;

    // ToDo: autoriser déclaration de variable dans le POUR ?
    symbol *tmp = get_symbol(table, c_context, node.id->id.name);

    /* Initialisation de la variable */
    codegen(node.affect_init);

    /* Vérification de la condition */
    int jump_back = nb_instr;
    codegen(node.end_exp);

    int tmp_jmp = tmp->mem_zone == 's' ? 6 : 3;
    int jumz_adr = nb_instr + node.list_instr->codelen + tmp_jmp;
    add_instr(JUMZ, ' ', jumz_adr);
    codegen(node.list_instr);

    /* Incrément de la variable controllant la boucle */
    int adr = tmp->adr;
    char adr_type = ' ';
    if (tmp->mem_zone == 's')
    {
        add_instr(LOAD, ' ', STACK_REL_START);
        add_instr(SUB, '#', adr);
        add_instr(STORE, ' ', TMP_REG_STK_ADR);
        adr = TMP_REG_STK_ADR;
        adr_type = '@';
    }
    add_instr(INC, adr_type, adr);

    add_instr(JUMP, ' ', jump_back);
}


void codegen_if(ast *t)
{
    if_node node = t->if_n;

    codegen(node.expr);

    /* Si expression fausse, on saute le 1er bloc d'instructions */
    int jumz_adr = nb_instr + node.list_instr1->codelen + 2;
    add_instr(JUMZ, ' ', jumz_adr);

    /* Si vraie, on l'exécute et on saute le 2ème bloc */
    codegen(node.list_instr1);

    int jump_adr = nb_instr + 1;
    if (node.list_instr2 != NULL) jump_adr += node.list_instr2->codelen;
    add_instr(JUMP, ' ', jump_adr);

    /* S'il y a un sinon */
    if (node.list_instr2 != NULL) codegen(node.list_instr2);
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

        /* Si dans la pile il faut calculer l'adresse à l'exécution */
        int adr = tmp->adr;
        if (tmp->mem_zone == 's')
        {
            /* On stocke la valeur de l'ACC */
            add_instr(STORE, ' ', TMP_REG_ACC_SWP);
            add_instr(LOAD, ' ', STACK_REL_START);
            add_instr(SUB, '#', adr);
            add_instr(STORE, ' ', TMP_REG_STK_ADR);

            /* On remet l'ACC dans son état initial */
            add_instr(LOAD, ' ', TMP_REG_ACC_SWP);
            adr_type = '@';
            adr = TMP_REG_STK_ADR;
        }
        add_instr(STORE, adr_type, adr);
    }

    /* MAJ du tas / pile */
    if (tmp->mem_zone == 'h') add_instr(INC, ' ', HEAP_REG);
    else if (tmp->mem_zone == 's') add_instr(DEC, ' ', STACK_REG);

    /* On génère le code pour la déclaration de variable suivante */
    codegen(node.next);
}




void codegen_func_decla(ast *t)
{
    func_decla_node node = t->func_decla;

    /* On change le contexte qui devient le nom de la fonction */
    strcpy(old_context, c_context);
    strcpy(c_context, node.id->id.name);


    /*
     * Si jamais la fonction est déclarée avant le programme principal,
     * il ne faut pas l'exécuter tant qu'elle n'est pas appelée. Le jump
     * permet de sauter directement à la fin de la déclaration de la
     * fonction lors de l'exécution.
     */
    if (strcmp(node.id->id.name, "PROGRAMME") != 0)
    {
        add_instr(JUMP, ' ', nb_instr + t->codelen);
    }
    

    /* MAJ de l'adresse de la fonction */ // ! attention si appel avant
    symbol *tmp = get_symbol(table, c_context, node.id->id.name);
    tmp->adr = nb_instr;
    t->mem_adr = nb_instr;

    codegen(node.list_decl);
    codegen(node.list_instr);

    /* Pas de retour pour la fonction principale */
    if (strcmp(node.id->id.name, "PROGRAMME") == 0)
    {
        add_instr(STOP, ' ', 0);
        return;
    }

    /* Si pas de "RETOURNER" spécifié */
    if (!tmp->has_return) add_instr(LOAD, '#', 0);

    /* On stocke le contenu de la valeur de retour */
    add_instr(STORE, ' ', REG_RETURN_VALUE);

    /*
     * On dépile jusqu'à atteindre le point de départ pour récupérer
     * l'adresse de retour.
     */
    int jumb_back = nb_instr;
    add_instr(LOAD, ' ', STACK_REL_START);
    add_instr(SUB, ' ', STACK_REG);
    add_instr(JUMZ, ' ', nb_instr + 3);
    add_instr(INC, ' ', STACK_REG);
    add_instr(JUMP, ' ', jumb_back);

    /* La pile pointe mtn sur l'adresse de retour */
    add_instr(LOAD, '@', STACK_REG);
    add_instr(STORE, ' ', REG_RETURN_ADR);

    /* On recharge la valeur de retour puis on jump */
    add_instr(LOAD, ' ', REG_RETURN_VALUE);
    push();
    add_instr(JUMP, '@', REG_RETURN_ADR);

    strcpy(c_context, old_context);
}




/**
 * @brief Génère le code permettant d'appeler une fonction.
 * 
 * Il faut empiler un certain nombre d'informations pour les transmettre à la
 * fonction appelée.
 * On empilera en 1er le début de pile relatif de la fonction appelante, afin
 * de pouvoir le remettre dans son état initial.
 * On empilera ensuite l'adresse de retour pour que la fonction appelée sache où
 * revenir à la fin de son exécution.
 * Enfin, on empilera les différents paramètres.
 * 
 * 
 * │                        │
 * │                        │
 * ├────────────────────────┤ <── Sommet de la pile
 * │        param n         │
 * ├────────────────────────┤
 * │          ...           │
 * ├────────────────────────┤
 * │        param 2         │
 * ├────────────────────────┤
 * │        param 1         │
 * ├────────────────────────┤
 * │   Adresse de retour    │
 * ├────────────────────────┤ <── Début de pile relatif de la fonction appelée
 * │   Début pile relatif   │
 * │   fonction appelante   │
 * ├────────────────────────┤
 * │                        │
 * │      pile fonction     │
 * │        appelante       │
 * │                        │
 * ├────────────────────────┤ <── Début de pile relatif de la fonction appelante
 * │                        │
 * │                        │
 * │                        │
 * │ pile avant la fonction │
 * │       appelante        │
 * │                        │
 * │                        │
 * │                        │
 * └────────────────────────┘ <── Début de la pile réel
 */
void codegen_func_call(ast *t)
{
    func_call_node node = t->func_call;

    /*
     * On empile le début relatif de la pile actuel pour le récupérer lorsque
     * l'on reviendra de la fonction appelée
     */
    add_instr(LOAD, ' ', STACK_REL_START);
    push();


    /*
     * Le nouveau début relatif de la pile sera au niveau du pointeur de
     * pile.
     * On ne le met pas à jour immédiatement car le début relatif est
     * utilisé + bas pour empiler les paramètres.
     */
    add_instr(LOAD, ' ', STACK_REG);
    add_instr(STORE, ' ', TMP_REG_4);


    /*
     * On empile l'adresse de retour.
     * -7 pour retirer les 7 dernières instructions (qui sont celles exécutées)
     * après la fonction appelée).
     * -6 pour ajouter le décalage des 5 premières instructions (dont celle-ci)
     */
    add_instr(LOAD, '#', nb_instr + t->codelen - 7 - 6 + 1);
    add_instr(STORE, '@', STACK_REG);
    add_instr(DEC, ' ', STACK_REG);

    /*
     * On empile les paramètres (on a fait en sorte de faire l'analyse
     * sémantique des paramètres en premier pour les noeuds de
     * déclaration de fonction de manière à ce que les adresses relatives soient
     * bonnes).
     */
    ast *aux = node.params;
    while (aux != NULL)
    {
        codegen(aux->exp_list.exp);
        push();
        aux = aux->exp_list.next;
    }

    /* On met à jour le début relatif de la pile maintenant */
    add_instr(LOAD, ' ', TMP_REG_4);
    add_instr(STORE, ' ', STACK_REL_START);


    /* On JUMP à l'adresse de la fonction */
    symbol *tmp = get_symbol(table, c_context, node.func_id->id.name);
    add_instr(JUMP, ' ', tmp->adr);

    /*
     * Retour de la fonction ici. La fonction aura tout dépilé avant de
     * quitter.
     * On dépile le retour de la fonction qui a été empilé en dernier
     */
    pop();
    add_instr(STORE, ' ', TMP_REG_SWP);

    /*
     * On remet à jour le début relatif de la pile en dépilant la valeur empilée
     * au tout début.
     */
    pop();
    add_instr(STORE, ' ', STACK_REL_START);

    /* Et on remet enfin la valeur de retour dans l'ACC */
    add_instr(LOAD, ' ', TMP_REG_SWP);
}


void codegen_return(ast *t)
{
    return_node node = t->return_n;

    /*
     * S'il y a une expression on la prend en compte, sinon on chargera
     * 0 dans l'ACC (on retourne donc 0 par défaut pour les fonctions
     * ne retournant aucune valeur)
     */
    if (node.expr != NULL) codegen(node.expr);
    else add_instr(LOAD, '#', 0);
}