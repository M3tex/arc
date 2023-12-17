#ifndef _RAM_OS_HEADER
#define _RAM_OS_HEADER

#include <limits.h>

/*
 * Constantes:
 * TMP_REG_1 -> 1er registre temporaire réservé à l'OS
 * TMP_REG_2 -> 2ème registre temporaire réservé à l'OS
 * STACK_REG -> Registre contenant l'adresse du sommet de la pile
 * HEAP_REG  -> Registre contenant l'adresse de la fin du tas
 * ...
 * 
 * 
 * Schéma de l'organisation de la mémoire:
 * 
 *    _______________   0
 *   |      ACC      |
 *   |_______________|  1
 *   |               |
 *   |     ramOS     |
 *   |               |
 *   |_______________|  ?
 *   |               |
 *   |      Tas      |             |
 *   |_______________|  HEAP_REG   v
 *   |               |
 *   |     libre     |
 *   |_______________|  STACK_REG  ^
 *   |               |             |
 *   |     pile      |
 *   |_______________|
 * 
 * 
 */



/* Registres temporaires */
#define TMP_REG_CMP 1
#define TMP_REG_STK_ADR 2
#define TMP_REG_ACC_SWP 3
#define TMP_REG_4 4
#define TMP_REG_SWP 5
#define REG_RETURN_VALUE 6
#define REG_RETURN_ADR 7

/*
 * Gestion de la pile.
 * Entiers sur 16 bits: les adresses traitées sont donc au + USHRT_MAX
 * (pour être + précis SHRT_MAX car les entiers sont signés).
 * La fin de la mémoire est donc à USHRT_MAX
 */
#define STACK_START USHRT_MAX
#define STACK_REG 10
#define STACK_REL_START 11


#define HEAP_REG  5

/* Début du tas: fin de ramOS */
#define HEAP_START 20


#endif