%{

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include <linux/limits.h>
#include "ast.h"
#include "arc_utils.h"
#include "codegen.h"
#include "symbol_table.h"
#include "semantic.h"
#include "preprocessor.h"


extern int yylex();
extern int yylex_destroy();
void yyerror(const char *s);

// ToDo: Table symboles
ast *abstract_tree = NULL;
symb_table table = NULL;


char *src = NULL;
char *exename = NULL;
char *include_path = NULL;

int line_offset = 0;

char PROJECT_PATH[PATH_MAX];
FILE *fp_out;

char current_ctx[32] = "global";

%}


%union {
    int nb;
    char id[32];
    struct ast *tree;
};


%define parse.error verbose
%locations

%token <nb> NB
%token <id> ID
%token PROGRAMME DEBUT FIN VAR ALGO
%token AFFECT_SYMB "<-"
%token LE "<="
%token GE ">="
%token DIFF "!="
%token LIRE
%token ECRIRE
%token TQ
%token FAIRE
%token FTQ
%token SI
%token ALORS
%token SINON
%token FSI
%token POUR
%token DANS
%token FPOUR
%token RANGE_SYMB "..."
%token RETOURNER
%token '\n'

/* Priorités (du - prioritaire au + prioritaire) */
%right AFFECT_SYMB

%left OU
%left ET
%left NON
%nonassoc '<' '>' '=' "<=" ">=" "!="

%left '+' '-'
%left '*' '/' '%'
%token '(' ')'

/* Axiome */
%start PROGRAMME_ALGO

%type <tree> EXP
%type <tree> PROGRAMME_ALGO
%type <tree> CORPS_FUNC
%type <tree> LISTE_INSTR
%type <tree> INSTR
%type <tree> PROG_MAIN
%type <tree> AFFECTATION
%type <tree> DECLARATION
%type <tree> LIST_INIT
%type <tree> STRUCT_SI
%type <tree> STRUCT_TQ
%type <tree> STRUCT_FAIRE_TQ
%type <tree> STRUCT_POUR
%type <tree> LIST_DECLA
%type <tree> FONCTION
%type <tree> LIST_ARGS
%type <tree> LIST_EXPR
%type <tree> APPEL_FUNC
%type <tree> LIRE_INSTR
%type <tree> ECRIRE_INSTR

%%

SEP : '\n'
| ';'
| SEP '\n'
| SEP ';'
;

END_FILE: SEP
| %empty
;

SEP_STRUCT: SEP
| %empty
;


LIST_INIT: ID "<-" EXP          {$$ = create_var_decla_node($1, $3, NULL);}
| ID                            {$$ = create_var_decla_node($1, NULL, NULL);}
| LIST_INIT ',' ID              {$$ = create_var_decla_node($3, NULL, $1);}
| LIST_INIT ',' ID "<-" EXP     {$$ = create_var_decla_node($3, $5, $1);}
;



DECLARATION: VAR LIST_INIT SEP  {$$ = create_decla_node($2, NULL);}
| FONCTION                      {$$ = create_decla_node($1, NULL);}
;


LIST_DECLA: %empty              {$$ = NULL;}
| DECLARATION LIST_DECLA        {$1->decla_list.next = $2; $$ = $1;}
;


AFFECTATION: ID "<-" EXP   {$$ = create_affect_node($1, $3);}
;



PROGRAMME_ALGO: SEP_STRUCT LIST_DECLA PROG_MAIN END_FILE
                   {$$ = create_prog_root($2, $3); abstract_tree = $$;}
; 


PROG_MAIN: PROGRAMME '(' ')' SEP        
LIST_DECLA 
DEBUT SEP 
CORPS_FUNC
FIN             {$$ = create_function_node("PROGRAMME", NULL, $5, $8);}
;


LIST_ARGS: %empty       {$$ = NULL;}
| ID                    {$$ = create_var_decla_node($1, NULL, NULL);}
| LIST_ARGS ',' ID      {$$ = create_var_decla_node($3, NULL, $1);}
;


LIST_EXPR: %empty       {$$ = NULL;}
| EXP                   {$$ = create_exp_list_node($1, NULL);}
| LIST_EXPR ',' EXP     {$$ = create_exp_list_node($3, $1);}
;


FONCTION: ALGO ID '(' LIST_ARGS ')' SEP
LIST_DECLA
DEBUT SEP
CORPS_FUNC
FIN                 
SEP                     {$$ = create_function_node($2, $4, $7, $10);}
;

CORPS_FUNC: LISTE_INSTR {$$ = $1;}
;

APPEL_FUNC: ID '(' LIST_EXPR ')'    {$$ = create_function_call_node($1, $3);}
;


EXP: '(' EXP ')'        {$$ = $2;}
| ID                    {$$ = create_id_leaf($1);}
| NB                    {$$ = create_nb_leaf(yylval.nb);}
| NON EXP               {$$ = create_u_op_node(NOT_OP, $2);}
| '-' EXP               {$$ = create_u_op_node('-', $2);}
| EXP '+' EXP           {$$ = create_b_op_node('+', $1, $3);}
| EXP '-' EXP           {$$ = create_b_op_node('-', $1, $3);}
| EXP '*' EXP           {$$ = create_b_op_node('*', $1, $3);}
| EXP '/' EXP           {$$ = create_b_op_node('/', $1, $3);}
| EXP '%' EXP           {$$ = create_b_op_node('%', $1, $3);}
| EXP '<' EXP           {$$ = create_b_op_node('<', $1, $3);}
| EXP '>' EXP           {$$ = create_b_op_node('>', $1, $3);}
| EXP '=' EXP           {$$ = create_b_op_node('=', $1, $3);}
| EXP "<=" EXP          {$$ = create_b_op_node(LE_OP, $1, $3);}
| EXP ">=" EXP          {$$ = create_b_op_node(GE_OP, $1, $3);}
| EXP "!=" EXP          {$$ = create_b_op_node(NE_OP, $1, $3);}
| EXP OU EXP            {$$ = create_b_op_node(OR_OP, $1, $3);}
| EXP ET EXP            {$$ = create_b_op_node(AND_OP, $1, $3);}
| APPEL_FUNC            {$$ = $1;}
| LIRE_INSTR            {$$ = $1;}
;


INSTR: EXP SEP          {$$ = $1;}
| AFFECTATION SEP       {$$ = $1;}
| ECRIRE_INSTR SEP      {$$ = $1;}
| STRUCT_TQ SEP         {$$ = $1;}
| STRUCT_SI SEP         {$$ = $1;}
| STRUCT_FAIRE_TQ SEP   {$$ = $1;}
| STRUCT_POUR SEP       {$$ = $1;}
| RETOURNER EXP SEP     {$$ = create_return_node($2);}
| RETOURNER SEP         {$$ = create_return_node(NULL);}
;

LISTE_INSTR: INSTR      {$$ = create_instr_node($1, NULL);}
| LISTE_INSTR INSTR     {$$ = create_instr_node($2, $1);}
;


// Autres structures
LIRE_INSTR: LIRE '(' ')'            {$$ = create_io_node(NULL, 'r');}
;

ECRIRE_INSTR: ECRIRE '(' EXP ')'    {$$ = create_io_node($3, 'w');}
;

STRUCT_TQ: TQ EXP FAIRE SEP_STRUCT LISTE_INSTR FTQ {$$ = create_while_node($2, $5);}
;

STRUCT_FAIRE_TQ: FAIRE SEP_STRUCT
LISTE_INSTR TQ EXP                  {$$ = create_do_while_node($3, $5);}
;


STRUCT_POUR: POUR ID DANS EXP "..." EXP FAIRE SEP_STRUCT
LISTE_INSTR FPOUR       {$$ = create_for_node($2, $4, $6, $9);}
;


STRUCT_SI: 
 SI EXP ALORS SEP_STRUCT 
 LISTE_INSTR 
 SINON SEP_STRUCT 
 LISTE_INSTR 
 FSI    {$$ = create_if_node($2, $5, $8);}
| SI EXP ALORS SEP_STRUCT 
  LISTE_INSTR 
  FSI  {$$ = create_if_node($2, $5, NULL);}
;

%%


int main(int argc, char **argv)
{
    extern FILE *yyin;

    /*
     * Bricolage mais fonctionne:
     * Permet d'obtenir le chemin vers le projet.
     * Utilisé pour le chemin vers la librairie standard dans preprocessor.c
     *
     * __FILE__ contient le chemin du fichier passé en paramètre à gcc.
     * C'est pour cette raison que l'on donne le chemin complet de parser.y
     * dans le makefile.
     * 
     * La 2ème ligne permet de supprimer le bout de chemin en trop.
     */
    strcpy(PROJECT_PATH, __FILE__);
    PROJECT_PATH[strlen(PROJECT_PATH) - strlen("/src/parser.y")] = '\0';

    int opt, dbg = 0;
    char *options = "o:dI:";
    while ((opt = getopt(argc, argv, options)) != -1)
    {
        switch (opt)
        {
        case 'o':
            exename = (char *) malloc(sizeof(char) * (strlen(optarg) + 1));
            check_alloc(exename);
            strcpy(exename, optarg);
            break;
        case 'd':
            dbg = 1;
            break;
        case 'I':
            include_path = (char *) malloc(sizeof(char) * (strlen(optarg) + 1));
            check_alloc(include_path);
            strcpy(include_path, optarg);
            break;
        default:
            break;
        }
    }

    /* Le seul argument sans option doit être le fichier.algo */
    if (optind >= argc)
    {
        fatal_error("pas de fichier en entrée");
        exit(F_INPUT_ERROR);
    }

    src = (char *) malloc(sizeof(char) * (strlen(argv[optind]) + 1));
    check_alloc(src);
    strcpy(src, argv[optind]);

    /* Phase préprocesseur */
    yyin = preprocessor(src, &line_offset);

    /* Initialisation de la table des symboles */
    table = init_symb_table("global");
    abstract_tree = NULL;

    yyparse();
    fclose(yyin);

    /* Supprime le fichier intermédiaire utilisé */
    system("rm __arc_PP.algo_pp");

    /* Analyse sémantique */
    semantic(abstract_tree);

    
    if (dbg)
    {
        ast_to_img(abstract_tree, "ast", "png");
        symb_to_img(table, "table", "png");
    }


    if (exename == NULL)
    {
        exename = (char *) malloc(sizeof(char) * (strlen("a.out") + 1));
        check_alloc(exename);
        strcpy(exename, "a.out");
    }

    fp_out = fopen(exename, "w");
    if (fp_out == NULL)
    {
        fatal_error("impossible d'ouvrir ~U%s~E", exename);
        exit(F_INPUT_ERROR);
    }
    init_ram_os();
    codegen(abstract_tree);

    free_table(table);
    fclose(fp_out);

    if (dbg)
    {
        /* Temporaire, pour vérifier que semantic marche bien */
        char buff[256];
        size_t codelen_total = abstract_tree->codelen + 5;  // + 5 pour ramOS
        sprintf(buff, "echo \"Codelen total: %ld\nNombre de lignes "\
        "dans le fichier produit: \" && wc -l %s", codelen_total, exename);
        system(buff);
    }



    free(src);
    free(exename);
    free_ast(abstract_tree);
    
    if (include_path != NULL) free(include_path);

    /* Libère la mémoire non-libérée par bison */
    yylex_destroy();    

    return 0;
}


void yyerror(const char *s)
{
    int lig = yylloc.first_line - line_offset;
    colored_error(RED|BOLD, 0, "%s:%d:%d:", src, lig, yylloc.first_column);
    print_error(BISON_ERROR, " %s\n", s);
}