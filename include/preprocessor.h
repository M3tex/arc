#ifndef _PREPROCESSOR_HEADER
#define _PREPROCESSOR_HEADER

#include <stdio.h>



FILE *cpy_file(FILE *src, const char *dest_name);
FILE *preprocessor(char *src, int *nb_inserted);

#endif