#include "preprocessor.h"
#include "arc_utils.h"
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>


extern char *include_path;
extern char PROJECT_PATH[PATH_MAX];


/**
 * @brief Copie le fichier src dans un nouveau fichier de nom `dest_name`.
 * Évite de modifier le fichier source directement.
 * https://stackoverflow.com/a/2180157
 * 
 * @param src 
 * @param dest_name 
 * @return FILE* Un pointeur sur le nouveau fichier
 */
FILE *cpy_file(FILE *src, const char *dest_name)
{
    FILE *result = fopen(dest_name, "w+");
    check_alloc(result);

    off_t tmp = 0;
    struct stat fileinfo = {0};
    fstat(src->_fileno, &fileinfo);
    if (sendfile(result->_fileno, src->_fileno, &tmp, fileinfo.st_size) == -1)
    {
        perror("Erreur lors de la copie du fichier");
        exit(F_INPUT_ERROR);
    }

    return result;
}


/**
 * @brief Renvoie le chemin vers le fichier fname s'il existe (dans le 
 * répértoire spécifié via -I ou dans le répertoire de la librairie standard).
 * 
 * @param fname 
 * @return char* 
 */
static char *search_file(char *fname)
{
    char std_include[PATH_MAX];
    strcpy(std_include, PROJECT_PATH);
    strcat(std_include, "/libstd/");

    /* On vérifie que la librairie standard existe */
    if (access(std_include, R_OK) != 0)
    {
        fatal_error("impossible de trouver la librairie standard");
        exit(1);
    }

    char *buff = (char *) malloc(sizeof(char) * PATH_MAX);
    check_alloc(buff);

    /* On cherche d'abord dans le répertoire spécifié */
    if (include_path != NULL)
    {
        strcpy(buff, include_path);
        strcat(buff, fname);

        if (access(buff, R_OK) == 0) return buff;
    }

    /* Puis dans le répertoire standard */
    strcpy(buff, std_include);
    strcat(buff, fname);

    if (access(buff, R_OK) == 0) return buff;

    return NULL;
}



static void do_preproc_action(FILE *dest, char *line, size_t line_nb, int *nb)
{
    /* On récupère le nom du fichier.algo */
    char buff[4096];
    YYLTYPE err;
    err.first_line = line_nb;
    err.first_column = 1;
    err.last_line = line_nb;
    err.last_column = strlen(line);

    set_error_info(err);
    if (sscanf(line, " $ INCLURE %s \n", buff) != 1)
    {
        fatal_error("instuction pré-processeur invalide: ~B%s~E", line);
        exit(1);
    }
 
    /*
     * On vérifie que le fichier.algo est dans le chemin de la librairie
     * standard ou dans le chemin spécifié par l'option -I
     */
    char *f_path;
    if ((f_path = search_file(buff)) == NULL)
    {
        fatal_error("le fichier ~U%s~E n'a pas été trouvé", buff);
        exit(1);
    }

    FILE *to_insert = fopen(f_path, "r");
    check_alloc(to_insert);
    free(f_path);

    /* -1 car on supprime la ligne contenant l'instruction préprocesseur */
    (*nb)--;

    /* On insère le fichier inclus */
    while (fgets(buff, 4095, to_insert) != NULL)
    {
        fwrite(buff, sizeof(char), strlen(buff), dest);
        (*nb)++;
    }
    fputc('\n', dest);

    fclose(to_insert);
}



FILE *preprocessor(char *src, int *nb_inserted)
{
    FILE *og_file = fopen(src, "r");
    if (og_file == NULL)
    {
        fatal_error("impossible d'ouvrir ~U%s~E", src);
        exit(F_INPUT_ERROR);
    }

    /* Fichier qui sera analysé etc. (écriture + lecture) */
    FILE *pp_f = fopen("./__arc_PP.algo_pp", "w+");
    check_alloc(pp_f);

    /* Parcours des lignes */
    *nb_inserted = 0;
    size_t num_lig = 1;
    char line[4096];
    int i;

    while (fgets(line, 4095, og_file) != NULL)
    {
        /* Si on lit un '$' (marqueur d'opérations préprocesseur) */
        for (i = 0; line[i] != '\0' && line[i] != '$'; i++);
        if (line[i] == '$') do_preproc_action(pp_f, line, num_lig, nb_inserted);
        else fwrite(line, sizeof(char), strlen(line), pp_f);
        num_lig++;
    }

    /* Car utilisé après pour l'analyse lexicale */
    rewind(pp_f);
    fclose(og_file);

    return pp_f;
}