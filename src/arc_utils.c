#include "arc_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * @brief Affiche l'erreur voulue et quitte le programme avec le code
 * d'erreur passé en paramètre s'il est différent de 0.
 * 
 * L'affichage se fera dans stderr.
 * 
 * Pour la partie format:
 * https://sourceware.org/git?p=glibc.git;a=blob_plain;f=stdio-common/printf.c;hb=HEAD
 * 
 * @param exit_code 
 * @param fmt 
 * @param ... 
 */
void print_error(int exit_code, char *fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    if (exit_code) exit(exit_code);
}



/**
 * @brief Affiche l'erreur avec la couleur voulue et quitte le programme
 * avec le code d'erreur passé en paramètre s'il est différent de 0.
 * 
 * L'affichage se fera dans stderr.
 * 
 * @param esc_seq La séquence d'échappement ANSI déterminant la couleur
 * et le mode d'affichage.
 * Doit être égal à CLR | mode, avec CLR une couleur définie dans le 
 * header et mode valant BOLD ou UNDERLINE.
 * @param exit_code 
 * @param fmt 
 * @param ... 
 */
void colored_error(int esc_seq, int exit_code, char *fmt, ...)
{
    char buff[16];
    int clr = esc_seq & ~(BOLD | UNDERLINE);
    
    /*
     * Séquence d'échappement ANSI.
     * \x1b[ puis code couleur (entre 30 et 37)
     */
    sprintf(buff, "\033[%d", clr);
    
    /* 1 pour gras, 4 pour souligné */
    if (esc_seq & BOLD) strcat(buff, ";1");
    if (esc_seq & UNDERLINE) strcat(buff, ";4");
    strcat(buff, "m");

    fprintf(stderr, "%s", buff);
    va_list arglist;
    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    /* Reset des attributs */
    fprintf(stderr, "\033[0m");

    if (exit_code) exit(exit_code);
}



/**
 * @brief À appeler à la suite d'un malloc/calloc/realloc.
 * Quitte le programme si le pointeur est NULL.
 * 
 * @param ptr 
 */
void check_alloc(void *ptr)
{
    if (ptr == NULL)
    {
        colored_error(RED | BOLD, 0, "Erreur d'allocation mémoire");
        perror("");     /* Pour afficher + d'infos */
        exit(ALLOC_FAILED);
    }
}



/**
 * @brief Convertit un opérateur qui ne rentre pas sur 1 char en string
 * (comme OU, NON et ET par exemple)
 * 
 * @param dest 
 * @param op 
 */
void op_to_str(char *dest, int op)
{
    switch (op)
    {
    case AND_OP:
        strcpy(dest, " ET");
        break;
    case OR_OP:
        strcpy(dest, "OU");
        break;
    case NOT_OP:
        strcpy(dest, "NON");
        break;
    case LE_OP:
        strcpy(dest, "<=");
        break;
    case GE_OP:
        strcpy(dest, ">=");
        break;
    case NE_OP:
        strcpy(dest, "!=");
        break;
    default:
        dest[0] = op;
        dest[1] = '\0';
        break;
    }
}