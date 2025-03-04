#ifndef _SYMBOL_TABLE_HEADER
#define _SYMBOL_TABLE_HEADER



#include <stdio.h>
#include "ast.h"


/*
 * Représente un symbole:
 * id: l'identificateur
 * type: le type de la donnée stockée (entier, pointeur, fonction, etc.)
 * size: la taille en mémoire (entier=1, pointeur=1, tableau=n, etc.)
 * adr: l'adresse de la donnée stockée
 * mem_zone: 'h' si stocké dans le tas, 's' si dans la pile.
 * next: le symbole suivant dans le contexte
 * is_used: 1 si le symbole est utilisé, 0 sinon (pour warnings)
 * is_modified: prévu pour les optimisations (pas faites)
 * is_init: pour les erreurs
 * is_checked: pour ne pas afficher les erreurs plusieurs fois
 */
typedef struct _symbol {
    char id[ID_MAX_SIZE];
    type_symb type;
    int size;
    int adr;
    char mem_zone;
    struct _symbol *next;
    int is_used;
    int is_modified;
    int is_init;
    int is_checked;             // Pour le 2ème parcourt de l'analyse sémantique 
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

symbol *init_symbol(const char *id, int adr, char zone, type_symb t);

context *search_context(symb_table table, const char *c_name);
symbol *search_symbol(symb_table table, const char *c_name, const char *id);

context *add_context(symb_table table, const char *c_name);
symbol *add_symbol(symb_table table, const char *c_name, symbol *s);

symbol *get_symbol(symb_table table, const char *ctx, const char *id);

void table_to_dot(symb_table table);
void symb_to_dot(FILE *fp, symbol s, char *c_name, char *previous);
void symb_to_img(symb_table table, char *filename, char *fmt);

#endif