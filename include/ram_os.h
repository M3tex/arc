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
 * ┌───────────────────┐ <── 0
 * │        ACC        │
 * ├───────────────────┤ <── 1
 * │                   │
 * │       ramOS       │
 * │                   │
 * ├───────────────────┤ <── STATIC_START
 * │                   │
 * │ mémoire  statique │
 * │                   │
 * ├───────────────────┤ <── Début tas
 * │                   │
 * │                   │
 * │        tas        │
 * │                   │
 * │                   │
 * ├────────────────┬──┤ <── HEAP_REG
 * │                ▼  │
 * │                   │
 * │                   │
 * │    espace libre   │
 * │                   │
 * │                   │
 * │                ▲  │
 * ├────────────────┴──┤ <── STACK_REG
 * │                   │
 * │                   │
 * │        pile       │
 * │                   │
 * │                   │
 * └───────────────────┘ <── Début pile
 */


/*
 * Registres temporaires.
 * Ils ne doivent PAS être utilisés si un codegen est appelé entre le stockage
 * et l'accès au registre temporaire (car il pourrait être modifié entre temps).
 * 
 * Exemple de mauvaise utilisation:
 * ...
 * add_instr(STORE, ' ', TMP_REG_SWP);
 * ...
 * codegen(...);
 * add_instr(LOAD, ' ', TMP_REG_SWP);
 */
#define TMP_REG_STK_ADR 1
#define TMP_REG_ACC_SWP 2
#define TMP_REG_REL_STK_CPY 3
#define TMP_REG_SWP 4
#define REG_RETURN_VALUE 5
#define REG_RETURN_ADR 6

/*
 * Gestion de la pile.
 * Entiers sur 16 bits: les adresses traitées sont donc au + USHRT_MAX
 * (pour être + précis SHRT_MAX car les entiers sont signés).
 * La fin de la mémoire est donc à USHRT_MAX.
 * Attention: en fonction du simulateur utilisé, USHRT_MAX peut-être trop grand.
 */
#define STACK_START USHRT_MAX
#define STACK_REG 10
#define STACK_REL_START 11
#define HEAP_REG  12



/* Début de la mémoire statique: fin de ramOS */
#define STATIC_START 20


#endif