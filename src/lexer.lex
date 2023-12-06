%{
    #include <string.h>
    #include "parser.h"
    #include "arc_utils.h"

    /*
     * MACRO pour mettre à jour la variable yylloc avec la position du
     * token. 
     */ 
    #define YY_USER_ACTION                                             \
    yylloc.first_line = yylloc.last_line;                              \
    yylloc.first_column = yylloc.last_column;                            \
    if (yylloc.last_line == yylineno) yylloc.last_column += yyleng;    \
    else {                                                             \
        yylloc.last_line = yylineno;                                   \
        yylloc.last_column = 1;                                        \
    }                                                                  \


    extern char *src;

%}

%option nounput
%option noinput
%option yylineno

/* CHIFFRE  [0-9] */
NOMBRE   [0-9]+
IDENT    [_a-zA-Z]+[_a-zA-Z0-9]*

COMM     ([#]+.*|\/\/.*|\/\*.*\*\/)

%%
{COMM}          {/* Ignore les commentaires */}
{NOMBRE}        {yylval.nb = atoi(yytext); return NB;}


[-*+/=%<>)(;\n,] {return yytext[0];}
[ \t]           {/* Ignore les caractères blancs */}


"PROGRAMME"     {return PROGRAMME;}
"DEBUT"         {return DEBUT;}
"FIN"           {return FIN;}
"VAR"           {return VAR;}
"<-"            {return AFFECT_SYMB;}

"ALGO"          {return ALGO;}

"NON"           {return NON;}
"ET"            {return ET;}
"OU"            {return OU;}

"TQ"            {return TQ;}
"FAIRE"         {return FAIRE;}
"FTQ"           {return FTQ;}

"SI"            {return SI;}
"ALORS"         {return ALORS;}
"SINON"         {return SINON;}
"FSI"           {return FSI;}

"!="            {return DIFF;}
"<="            {return LE;}
">="            {return GE;}

{IDENT}         {strcpy(yylval.id, yytext); return ID;}

.               {
                    colored_error(RED|BOLD, 0, "[erreur lexicale] ");
                    colored_error(BOLD, 0, "%s:%d:%d:", src, yylloc.first_line,
                                  yylloc.first_column);
                    print_error(0, " caractère ‘");
                    colored_error(MAGENTA|BOLD, 0, "%s", yytext);
                    print_error(LEX_ERROR, "‘ inattendu\n");
                }
%%