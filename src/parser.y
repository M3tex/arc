%{

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include "ast.h"
#include "arc_utils.h"
#include "codegen.h"
#include "symbol_table.h"
#include "semantic.h"


extern int yylex();
void yyerror(const char *s);

// ToDo: Table symboles
ast *abstract_tree = NULL;
symb_table table = NULL;


char *src = NULL;
char *exename = "a.out";
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
%token PROGRAMME DEBUT FIN VAR
%token AFFECT_SYMB "<-"
%token LE "<="
%token GE ">="
%token DIFF "!="
%token TQ
%token FAIRE
%token FTQ
%token SI
%token ALORS
%token SINON
%token FSI
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
%type <tree> DECLA_VAR
%type <tree> LIST_INIT
%type <tree> STRUCT_TQ
%type <tree> STRUCT_SI
%type <tree> LIST_DECLA_VAR

%%

SEP : '\n' 
| SEP '\n'
;

END_FILE: SEP
| %empty
;

SEP_STRUCT: '\n'
| %empty
;


LIST_INIT: ID "<-" EXP          {$$ = create_var_init_node($1, $3, NULL);}
| ID                            {$$ = create_var_init_node($1, NULL, NULL);}
| LIST_INIT ',' ID              {$$ = create_var_init_node($3, NULL, $1);}
| LIST_INIT ',' ID "<-" EXP     {$$ = create_var_init_node($3, $5, $1);}
;



DECLA_VAR:VAR LIST_INIT SEP     {$$ = create_var_decla_node($2, NULL);}
;


LIST_DECLA_VAR: %empty          {$$ = NULL;}
| DECLA_VAR LIST_DECLA_VAR      {$$ = create_var_decla_node($1, $2);}
;

AFFECTATION: ID "<-" EXP   {$$ = create_affect_node(create_id_leaf($1), $3);}   
;



PROGRAMME_ALGO: LIST_DECLA_VAR PROG_MAIN END_FILE {$$ = create_prog_root($1, $2);
                                               abstract_tree = $$;
                                             }
;


PROG_MAIN: PROGRAMME '(' ')' SEP LIST_DECLA_VAR DEBUT SEP CORPS_FUNC FIN
                        {$$ = $8;}
;


CORPS_FUNC: LISTE_INSTR {$$ = $1;}
;



EXP: '(' EXP ')'        {$$ = $2;}
| ID                    {$$ = create_id_leaf($1);}
| NB                    {$$ = create_nb_leaf(yylval.nb);}
| NON EXP               {$$ = create_u_op_node(NOT_OP, $2);}
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
;


INSTR: EXP SEP          {$$ = $1;}
| AFFECTATION SEP       {$$ = $1;}
| STRUCT_TQ SEP
| STRUCT_SI SEP
;

LISTE_INSTR: INSTR      {$$ = create_instr_node($1, NULL);}
| LISTE_INSTR INSTR     {$$ = create_instr_node($2, $1);}
;


// Autres structures
STRUCT_TQ: TQ EXP FAIRE SEP LISTE_INSTR FTQ {}
;


STRUCT_SI: SI EXP ALORS SEP_STRUCT LISTE_INSTR SINON SEP_STRUCT LISTE_INSTR FSI    {}
| SI EXP ALORS SEP_STRUCT LISTE_INSTR FSI                               {}
;

%%


int main(int argc, char **argv)
{
    extern FILE *yyin;

    int opt;
    char *options = "o:";
    while ((opt = getopt(argc, argv, options)) != -1)
    {
        switch (opt)
        {
        case 'o':
            exename = (char *) malloc(sizeof(char) * (strlen(optarg) + 1));
            check_alloc(exename);
            strcpy(exename, optarg);
            break;
        default:
            break;
        }
    }

    /* Le seul argument sans option doit être le fichier.algo */
    if (optind >= argc)
    {
        colored_error(RED|BOLD, 0, "erreur fatale:");
        print_error(F_INPUT_ERROR, " pas de fichier en entrée.\n");
    }

    src = (char *) malloc(sizeof(char) * (strlen(argv[optind]) + 1));
    check_alloc(src);
    strcpy(src, argv[optind]);
    
    /* On essaye d'ouvrir le fichier */
    yyin = fopen(src, "r");
    if (yyin == NULL)
    {
        colored_error(RED|BOLD, 0, "erreur fatale:");
        print_error(0, " impossible d'ouvrir ");
        colored_error(UNDERLINE, 0, "%s", src);
        print_error(F_INPUT_ERROR, "\n");
    }

    /* Initialisation de la table des symboles */
    table = init_symb_table("global");

    yyparse();
    ast_to_img(abstract_tree, "ast", "png");
    semantic(abstract_tree);
    symb_to_img(table, "table", "png");


    fp_out = fopen(exename, "w");
    if (fp_out == NULL)
    {
        colored_error(RED|BOLD, 0, "erreur fatale:");
        print_error(0, " impossible d'ouvrir ");
        colored_error(UNDERLINE, 0, "%s", exename);
        print_error(F_INPUT_ERROR, "\n");
    }
    init_ram_os();
    codegen(abstract_tree);
    add_instr(STOP, ' ', 0);

    return 0;
}


void yyerror(const char *s)
{
    colored_error(RED|BOLD, 0, "%s:%d:%d:", src, yylloc.first_line, yylloc.first_column);
    print_error(BISON_ERROR, " %s\n", s);
}