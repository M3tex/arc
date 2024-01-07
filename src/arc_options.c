#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include "arc_utils.h"
#include "preprocessor.h"


extern char *include_path;
extern char *exename;
extern int is_dbg_mode;
extern int print_tree;
extern int print_table;
extern int mem_size;


static void print_help()
{
    fprintf(stderr, "Utilisation: arc [-o outfile] [-d | --debug] "\
            "[--print-tree] [--print-table] [-I dir] infile\n");
    fprintf(stderr, "Consultez le man pour plus d'informations\n");
}


void handle_options(int argc, char **argv)
{
    int opt;
    const struct option options[] = {
        {"draw-tree", no_argument, NULL, 1},
        {"draw-table", optional_argument, NULL, 2},
        {"debug", no_argument, NULL, 'd'},
        {"mem-size", required_argument, NULL, 3},
        {NULL, 0, NULL, '\0'}
    };

    while((opt = getopt_long(argc, argv, "I:o:d", options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'I':
            include_path = (char *) malloc(sizeof(char) * (strlen(optarg) + 2));
            check_alloc(include_path);
            strcpy(include_path, optarg);
            if (optarg[strlen(optarg) - 1] != '/') strcat(include_path, "/");
            break;
        case 'o':
            exename = (char *) malloc(sizeof(char) * (strlen(optarg) + 1));
            check_alloc(exename);
            strcpy(exename, optarg);
            break;
        case 'd':
            is_dbg_mode = 1;
            break;
        case 1:
            print_tree = 1;
            break;
        case 2:
            print_table = 1;
            break;
        case 3:
            mem_size = atoi(optarg);
            break;
        default:
            print_help();
            exit(1);
            break;
        }
    }

    /* Le seul argument sans option doit être le fichier.algo */
    if (optind >= argc)
    {
        fatal_error("pas de fichier en entrée");
        exit(F_INPUT_ERROR);
    }

    src = (char *) malloc(sizeof(char) * (strlen(argv[optind]) + 1));
    check_alloc(src);
    strcpy(src, argv[optind]);

    if (exename == NULL)
    {
        exename = (char *) malloc(sizeof(char) * (strlen("a.out") + 1));
        check_alloc(exename);
        strcpy(exename, "a.out");
    }

}