%{
    #include <string.h>
    #include "parser.h"
    #include "arc_utils.h"


    /* https://stackoverflow.com/a/19149193 */
    static void update_loc() {
        static int curr_line = 1;
        static int curr_col  = 1;

        yylloc.first_line   = curr_line;
        yylloc.first_column = curr_col;

        {char * s; for(s = yytext; *s != '\0'; s++) {
            if(*s == '\n') {
                curr_line++;
                curr_col = 1;
            } else {
                curr_col++;
            }
        }}

        yylloc.last_line   = curr_line;
        yylloc.last_column = curr_col;

        // printf(" first_line: %d, last_line: %d, firt_col: %d, last_col: %d\n\n", yylloc.first_line,
        // yylloc.last_line, yylloc.first_column, yylloc.last_column);

    }

    #define YY_USER_ACTION update_loc();


    /*
     * MACRO pour mettre à jour la variable yylloc avec la position du
     * token. 
     */ 
    /*#define YY_USER_ACTION                                             \
    yylloc.first_line = yylloc.last_line;                              \
    yylloc.first_column = yylloc.last_column;                            \
    if (yylloc.last_line == yylineno) yylloc.last_column += yyleng;    \
    else {                                                             \
        yylloc.last_line = yylineno;                                   \
        yylloc.last_column = 1;                                        \
        yylloc.first_column = 1;                                       \
    }   */

    extern char *src;

%}

%option nounput
%option noinput
%option yylineno

%x IN_MULTILINE_COMMENT

/* CHIFFRE  [0-9] */
NOMBRE   [0-9]+
IDENT    [_a-zA-Z]+[_a-zA-Z0-9]*

/* Commentaire sur une ligne (débutée par # ou //) */
COMM     ([#]+.*|\/\/.*)


%%
{COMM}          {/* Ignore les commentaires */}
"/*"            BEGIN(IN_MULTILINE_COMMENT);
<IN_MULTILINE_COMMENT>{
    /* http://westes.github.io/flex/manual/How-can-I-match-C_002dstyle-comments_003f.html */
    "*/"      BEGIN(INITIAL);
    [^*\n]+   // eat comment in chunks
    "*"       // eat the lone star
    \n
}


{NOMBRE}        {yylval.nb = atoi(yytext); return NB;}


"["             {return '[';}
"]"             {return ']';}

[-*+/=%<>)(;,@] {return yytext[0];}
[\n]            {return yytext[0];}
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

"LIRE"          {return LIRE;}
"ECRIRE"        {return ECRIRE;}

"TQ"            {return TQ;}
"FAIRE"         {return FAIRE;}
"FTQ"           {return FTQ;}

"SI"            {return SI;}
"ALORS"         {return ALORS;}
"SINON"         {return SINON;}
"FSI"           {return FSI;}

"POUR"          {return POUR;}
"DANS"          {return DANS;}
"..."           {return RANGE_SYMB;}
"FPOUR"         {return FPOUR;}

"RETOURNER"     {return RETOURNER;}
"RENVOYER"      {return RETOURNER;}

"ALLOUER"       {return ALLOUER;}

"VRAI"          {return VRAI;}
"FAUX"          {return FAUX;}

"PROTO"         {return PROTO;}

"!="            {return DIFF;}
"<="            {return LE;}
">="            {return GE;}

{IDENT}         {strcpy(yylval.id, yytext); return ID;}

.               {
                    set_error_info(yylloc);
                    fatal_error("erreur lexicale, caractère ‘~m~B%s~E‘ inattendu", yytext);
                    exit(LEX_ERROR);
                }
%%