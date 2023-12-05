#include "symbol_table.h"
#include "arc_utils.h"

#include <string.h>



/**
 * @brief Initialise la table des symboles
 * 
 * @param c_name 
 * @return symb_table* 
 */
symb_table init_symb_table(const char *c_name)
{
    symb_table result = (symb_table ) malloc(sizeof(context));
    check_alloc(result);

    result->next = NULL;
    result->symb_list = NULL;
    strcpy(result->name, c_name);

    return result;
}



/**
 * @brief Cherche le contexte de nom `c_name` dans la table passée en
 * paramètre, NULL s'il n'existe pas.
 * 
 * @param table 
 * @param c_name 
 * @return context* 
 */
context *search_context(symb_table table, const char *c_name)
{
    if (table == NULL) return NULL;

    context *aux = table;
    while (aux != NULL)
    {
        if (strcmp(aux->name, c_name) == 0) return aux;

        aux = aux->next;
    }
    
    /* Pas trouvé */
    return NULL;
}



/**
 * @brief Cherche le symbole d'identificateur `id` dans le contexte
 * `c_name`.
 * Retourne NULL s'il n'existe pas.
 * 
 * @param table 
 * @param c_name 
 * @param id 
 * @return symbol* 
 */
symbol *search_symbol(symb_table table, const char *c_name, const char *id)
{
    context *c = search_context(table, c_name);
    if (c == NULL)
    {
        colored_error(RED|BOLD, 0, "erreur fatale: ");
        print_error(0, "le contexte `");
        colored_error(BLUE, 0, "%s", c_name);
        print_error(UNDEF_CTX, "` est inconnu\n");
    }

    symbol *aux = c->symb_list;
    while (aux != NULL)
    {
        if (strcmp(aux->id, id) == 0) return aux;

        aux = aux->next;
    }

    /* Pas trouvé */
    return NULL;
}



/**
 * @brief Ajoute le symbole au contexte passé en paramètre.
 * 
 * @param table 
 * @param c_name 
 * @param id 
 * @param size 
 * @param type 
 * @return symbol* 
 */
symbol *add_symbol(symb_table table, const char *c_name, 
                     const char *id, int size, type_symb type)
{
    context *c = search_context(table, c_name);

    /* Si le contexte n'existe pas on l'ajoute */
    if (c == NULL) c = add_context(table, c_name);


    /* On parcourt les symboles du contexte pour l'ajouter à la fin */
    symbol *aux = c->symb_list;
    symbol *previous = NULL;
    while (aux != NULL)
    {
        /* Warning si symbole existe déjà */
        if (strcmp(aux->id, id) == 0)
        {
            colored_error(MAGENTA|BOLD, 0, "warning:");
            print_error(0, " le symbole ");
            colored_error(BOLD, 0, "‘%s‘", id);
            print_error(0, " est déjà déclaré dans le contexte ");
            colored_error(GREEN|BOLD, 0, "%s", c_name);
            print_error(0, "\n");
            return aux;     // ToDo: simple warning ou erreur fatale ?
        }

        previous = aux;
        aux = aux->next;
    }

    /* Création du nouveau symbole */
    symbol *new_symb = (symbol *) malloc(sizeof(symbol));
    check_alloc(new_symb);
    new_symb->adr = -1;     // ToDo: quoi mettre ?
    new_symb->next = NULL;
    new_symb->size = size;
    new_symb->type = type;
    strcpy(new_symb->id, id);

    /* Si le contexte était vide */
    if (previous == NULL) c->symb_list = new_symb;
    else previous->next = new_symb;

    return new_symb;
}



context *add_context(symb_table table, const char *c_name)
{
    if (table == NULL) return NULL;
    
    context *aux = table;
    while (aux->next != NULL)
    {
        /* Le contexte existe déjà */
        if (strcmp(aux->name, c_name) == 0)
        {
            colored_error(MAGENTA|BOLD, 0, "warning:");
            print_error(0, " le contexte ");
            colored_error(GREEN, 0, "%s", c_name);
            print_error(0, " existe déjà\n");
            return aux;           // ToDo: simple warning ou erreur fatale ?
        }

        aux = aux->next;
    }

    
    aux->next = (context *) malloc(sizeof(context));
    check_alloc(aux->next);

    context *res = aux->next;
    res->next = NULL;
    res->symb_list = NULL;
    strcpy(res->name, c_name);

    return res;
}


void free_table(symb_table table)
{
    context *aux_c;
    context *c = table;
    while (c != NULL)
    {
        /* Libération des symboles du contexte */
        symbol *aux_s;
        symbol *s = c->symb_list;
        while (s != NULL)
        {
            aux_s = s->next;
            free(s);
            s = aux_s;
        }
        aux_c = c->next;
        free(c);
        c = aux_c;
    }
}



/**
 * @brief Convertit la table des symboles au format dot.
 * 
 * @param table 
 * @param filename 
 */
void table_to_dot(symb_table table)
{
    FILE *fp = fopen("tmp2.dot", "w");
    check_alloc(fp);

    fprintf(fp, "graph G {\n");
    fprintf(fp, "    node [shape=record];\n    overlap=false;\n");

    /* Parcours des contextes */
    context *c = table;
    while (c != NULL)
    {
        fprintf(fp, "    %s [label=\"{<%s_label> %s|<%s_symbs>}|<%s_next>\"];\n",
                c->name, c->name, c->name, c->name, c->name);
        
        /* Mise au même niveau que les autre contextes */
        if (c->next != NULL)
        {
            fprintf(fp, "    {rank=same; %s:%s_next -- %s:%s_label;}",
                c->name, c->name, c->next->name, c->next->name);
        }
        
        /* Pour chaque contexte, on affiche ses symboles */
        symbol *symbs = c->symb_list;
        char *prev = NULL;
        while (symbs != NULL)
        {
            symb_to_dot(fp, *symbs, c->name, prev);
            prev = symbs->id;
            symbs = symbs->next;
        }

        /* On ajoute le lien avec le contexte */
        if (prev != NULL)
        {
            fprintf(fp, "    %s:%s_symbs -- %s_%s;\n", c->name, c->name,
                    c->name, prev);
        }

        c = c->next;
    }

    fprintf(fp, "}");
    fclose(fp);
}



void symb_to_dot(FILE *fp, symbol s, char *c_name, char *previous)
{
    static char *type_to_str[4] = {
        "entier", "pointeur", "tableau", "fonction"
    };

    fprintf(fp, "    %s_%s [label=\"{%s|{type: %s|taille: %d}", 
                c_name, s.id, s.id, type_to_str[s.type], s.size);
    fprintf(fp, "|adr: %d|<%s_%s_next>}\"];\n", s.adr, c_name, s.id);

    /* Lien avec le symbole précedent, si non NULL */
    if (previous != NULL)
    {
        fprintf(fp, "    %s_%s:%s_%s_next -- %s_%s;\n", c_name, s.id,
                c_name, s.id, c_name, previous);
        // fprintf(fp, "    %s:%s_symbs -- %s_%s;\n", c_name, c_name,
        //         c_name, s.id);
    }
    // else
    // {
    //     fprintf(fp, "    %s_%s:%s_%s_next -- %s_%s;\n", c_name, s.id,
    //             c_name, s.id, c_name, previous);
    // }
}



void symb_to_img(symb_table table, char *filename, char *fmt)
{
    table_to_dot(table);

    char cmd[128];
    sprintf(cmd, "dot -T%s tmp2.dot -o %s.%s", fmt, filename, fmt);

    system(cmd);
    // system("rm tmp2.dot");
}