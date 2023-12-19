#include "arc_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct error_info ERROR_INFO;


/**
 * @brief Retourne un string qui contiendra les séquences d'échappement ANSI
 * correspondant aux formats spécifiés:
 * ~r pour afficher en rouge (~g, ~b, ~y, etc. pour les autres couleurs)
 * ~B et ~U pour afficher en gras et souligné.
 * ~E pour stopper la séquence.
 * 
 * @param fmt 
 * @return char* 
 */
static char *ansi_fmt(const char *fmt)
{
    size_t size = 1024;
    char *result = (char *) calloc((size + 1), sizeof(char));
    check_alloc(result);

    char buff[16] = "";
    size_t i = 0, c_size = 0;
    while (fmt[i] != '\0')
    {
        /* Gestion des formats gérant les séquences d'échappement */
        if (fmt[i] == '~' && fmt[i + 1] != '\0')
        {
            /* "\033[...m" -> 3 char et au + 2 char (0m ou 31, 32, 33, etc.) */
            if (c_size + 3 + 2 >= size)
            {
                size = size << 1;
                result = (char *) realloc(result, sizeof(char) * (size + 1));
                check_alloc(result);
            }

            c_size += 3;
            i++;
            switch (fmt[i])
            {
            case 'r':
                c_size += 2;
                sprintf(buff, "\033[%dm", RED);
                break;
            case 'g':
                c_size += 2;
                sprintf(buff, "\033[%dm", GREEN);
                break;
            case 'y':
                c_size += 2;
                sprintf(buff, "\033[%dm", YELLOW);
                break;
            case 'b':
                c_size += 2;
                sprintf(buff, "\033[%dm", BLUE);
                break;
            case 'm':
                c_size += 2;
                sprintf(buff, "\033[%dm", MAGENTA);
                break;
            case 'c':
                c_size += 2;
                sprintf(buff, "\033[%dm", CYAN);
                break;
            case 'B':
                c_size++;
                sprintf(buff, "\033[1m");
                break;
            case 'U':
                c_size++;
                sprintf(buff, "\033[4m");
                break;
            case 'E':
                c_size++;
                sprintf(buff, "\033[0m");
                break;
            default:
                c_size++;
                buff[0] = fmt[i];
                buff[1] = '\0';
                break;
            }
            strcat(result, buff);
        }
        else
        {
            if (c_size + 1 >= size)
            {
                size = size << 1;
                result = (char *) realloc(result, sizeof(char) * (size + 1));
                check_alloc(result);
            }

            result[c_size] = fmt[i];
            c_size++;
        }
        i++;
    }

    return result;
}



void fatal_error(char *fmt, ...)
{
    if (ERROR_INFO.has_info)
    {
        fprintf(stderr, "\033[1m%s:%d:%d:\033[0m ", src, ERROR_INFO.lig,
                ERROR_INFO.col);
    }

    fprintf(stderr, "\033[31;1merreur fatale:\033[0m ");
    
    /* On se le permet car fmt n'est pas réutilisé après */
    fmt = ansi_fmt(fmt);

    va_list arglist;
    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    fprintf(stderr, "\n");
    free(fmt);
}


void warning(char *fmt, ...)
{
    if (ERROR_INFO.has_info)
    {
        fprintf(stderr, "\033[1m%s:%d:%d:\033[0m ", src, ERROR_INFO.lig,
                ERROR_INFO.col);
    }

    fprintf(stderr, "\033[35;1mwarning:\033[0m ");

    /* On se le permet car fmt n'est pas réutilisé après */
    fmt = ansi_fmt(fmt);

    va_list arglist;
    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    fprintf(stderr, "\n");
    free(fmt);
}


void set_error_info(int l, int c)
{
    ERROR_INFO.has_info = 1;
    ERROR_INFO.lig = l;
    ERROR_INFO.col = c;
}

void unset_error_info()
{
    ERROR_INFO.has_info = 0;
    ERROR_INFO.lig = 0;
    ERROR_INFO.col = 0;
}



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
     * \x1b[ puis code couleur (entre 31 et 37 ou 91 et 97)
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
inline void check_alloc(void *ptr)
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