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


/*
 * Entiers sur 16 bits: les adresses traitées sont donc au + USHRT_MAX
 * (pour être + précis SHRT_MAX car les entiers sont signés).
 * La fin de la mémoire est donc à USHRT_MAX
 */
#define STACK_START USHRT_MAX

/* Début du tas: fin de ramOS */
#define HEAP_START 20

#define TMP_REG_1 1
#define TMP_REG_2 2
#define STACK_REG 3
#define HEAP_REG  4
#define ADR_REG   5


#endif