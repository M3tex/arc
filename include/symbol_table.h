#ifndef _SYMBOL_TABLE_HEADER
#define _SYMBOL_TABLE_HEADER


#include "ast.h"
#include <stdio.h>


/* Les types disponibles */
typedef enum {integer, pointer, array, func} type_symb;


/*
 * Représente un symbole:
 * id: l'identificateur
 * type: le type de la donnée stockée (entier, pointeur, fonction, etc.)
 * size: la taille en mémoire (entier=1, pointeur=1, tableau=n, etc.)
 * adr: l'adresse de la donnée stockée
 * next: le symbole suivant dans le contexte
 */
typedef struct _symbol {
    char id[ID_MAX_SIZE];
    type_symb type;
    int size;
    int adr;
    struct _symbol *next;
} symbol;


/*
 * Représente un contexte (par exemple dans une fonction).
 * name: le nom du contexte
 * symb_list: la liste des symboles locaux au contexte
 * next: le contexte suivant.
 */
typedef struct _context {
    char name[ID_MAX_SIZE];
    symbol *symb_list;
    struct _context *next;
} context, *symb_table;


symb_table init_symb_table(const char *c_name);
void free_table(symb_table table);

context *search_context(symb_table table, const char *c_name);
symbol *search_symbol(symb_table table, const char *c_name, const char *id);

context *add_context(symb_table table, const char *c_name);
symbol *add_symbol(symb_table table, const char *c_name, 
                     const char *id, int size, type_symb type);

void table_to_dot(symb_table table);
void symb_to_dot(FILE *fp, symbol s, char *c_name, char *previous);
void symb_to_img(symb_table table, char *filename, char *fmt);

#endif