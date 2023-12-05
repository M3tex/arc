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
%token AFFECT "<-"
%token LE "<="
%token GE ">="
%token DIFF "!="
%token '\n'

/* Priorités (du - prioritaire au + prioritaire) */
%right AFFECT

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

%%

SEP : '\n' 
| SEP '\n'
;

PROGRAMME_ALGO: PROGRAMME '(' ')' SEP VAR_LISTE DEBUT SEP CORPS_FUNC FIN
                        {$$ = $8; abstract_tree = $$;}
;

INSTR: EXP SEP          {$$ = $1;}
| ID "<-" EXP SEP       {$$ = create_affect_node(create_id_leaf($1), $3);}
;

LISTE_INSTR: INSTR      {$$ = create_instr_node($1, NULL);}
| LISTE_INSTR INSTR     {$$ = create_instr_node($2, $1);}
;

CORPS_FUNC: LISTE_INSTR {$$ = $1;}
;

LISTE_ID: ID
| LISTE_ID ',' ID;
;

VAR_LISTE: %empty
| VAR_LISTE VAR LISTE_ID SEP    {/* ToDo: Ajouter au contexte tous les id */}
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