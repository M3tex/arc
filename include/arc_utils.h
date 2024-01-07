#ifndef _ARC_UTILS_HEADER
#define _ARC_UTILS_HEADER


#include "parser.h"

/* Contient toutes les fonctions non liées à des "vrais" modules */


/* Codes d'erreur */
#define ALLOC_FAILED 1
#define LEX_ERROR 2
#define BISON_ERROR 3
#define F_INPUT_ERROR 4
#define S_TABLE_NOT_INIT 5
#define UNDEF_CTX 6
#define UNDEF_ID 7


/* Couleurs (utilisées dans print_color) */
#define RED 31
#define GREEN 92
#define YELLOW 93
#define BLUE 94
#define MAGENTA 35
#define CYAN 36

/* Autres séquences d'échappement */
#define BOLD 128
#define UNDERLINE 256


/* Codes pour les opérateurs ne tenant pas sur un char */
#define AND_OP 256
#define OR_OP 257
#define NOT_OP 258
#define LE_OP 259
#define GE_OP 260
#define NE_OP 261


/* Structure contenant les informations de l'erreur (un peu comme un errno) */
struct error_info {
    int has_info;
    YYLTYPE loc;
};


extern char *src;




void print_error(int exit_code, char *fmt, ...);
void colored_error(int clr, int exit_code, char *fmt, ...);

void fatal_error(char *fmt, ...);

void warning(char *fmt, ...);

void set_error_info(YYLTYPE infos);
void unset_error_info();

void check_alloc(void *ptr);
void op_to_str(char *dest, int op);

#endif