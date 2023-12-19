# Algo-RAM-Compiler (arc)

`arc` est un compilateur écrit dans le cadre du module d'I53 de la
licence d'informatique à l'Université de Toulon.
Il est utilisé pour compiler du langage algorithmique (voir + bas) vers
la [machine RAM][RAM].


# Langage algorithmique
## Lexique
### Mots réservés du langage
`PROGRAMME`, `ALGO`, `DEBUT`, `FIN`, `VAR`, `TQ`, `FAIRE`, `FTQ`, `SI`,
`ALORS`, `SINON`, `FSI`, `ET`, `OU`, `NON`, `VRAI`, `FAUX`, `LIRE`,
`ECRIRE`, `ALLOUER`, `RENVOYER`

### Ponctuateurs/opérateurs
`+`, `-`, `*`, `/`, `%`, `(`, `)`, `<-`, `=`, `!=`, `<=`, `>=`, `,`,
`[`, `]`, `@`, `\n`

### Blancs:
Espace et `\t`

### Nombres:
Entiers décimaux sans 0 superflus.

### Identificateurs
chaînes de caractères alpha-numériques ne commençant pas par un chiffre.

## Grammaire du langage
Un programme est une suite d'**algorithmes** (=fonctions) définis par le 
mot réservé `ALGO`, un identificateur et une liste de paramètres.
Les paramètres précédés d'un `@` sont de type pointeur, et les autres
de types entier.

Le corps d'une fonction débute par une suite de une, plusieurs ou aucune
déclaration de variables (avec ou sans affectation) suivie(s) du mot
réservé `DEBUT`, d'une série d'instruction et du mot réservé `FIN`.
Le ponctuateur `\n` sera utilisé comme séparateur d'instructions.

L'affectation se fait à l'aide de l'opérateur `<-`, on utilisera `=`
pour la comparaison.

Le point d'entrée du programme se fera via la fonction nommée
`PROGRAMME`, qui ne sera **pas** précédée du mot réservé `ALGO`.


Exemple de programme réunissant à peu près toutes les constructions
possibles:
```
// Échange les éléments des cases d'indices i et j
ALGO ECHANGER( @T,i,j)
VAR temp
DEBUT
	temp <- T[i]
	T[i] <- T[j]
	T[j] <- temp
FIN


// Renvoie l'indice de la plus petite valeur du tableau T entre i et n	
ALGO Selection( @T, n, i )
VAR imin <- i
DEBUT
	i <- i+1
	TQ i < n FAIRE
	   SI T[imin] > T[i] ALORS
	      imin <- i
	   FSI
	   i <- i+1
	FTQ
	RENVOYER imin
FIN


PROGRAMME()
VAR taille, @T
VAR i, imin
DEBUT
	// Stockage des données dans un tableau dynamique
	taille <- LIRE()
	ALLOUER( T, taille )
	i <- 0
	TQ i < taille FAIRE
	   T[i] <- LIRE()
	   i <- i+1
	FTQ

	// Tri selection
	i <- 0
	TQ i < taille FAIRE
	   ECHANGER( T, i, Selection(T, taille, i))
	   i <- i+1
	FTQ

	// Affichage du tableau trié
	i <- 0
	TQ i < taille FAIRE
	   ECRIRE(T[i])
	   i <- i+1
	FTQ
	
	RENVOYER 0
FIN
```

## Fonctionnalités ajoutées
### Inclure d'autres `fichier.algo`
Ajout d'une phase pré-processeur au compilateur. Les instruction pré-processeurs
sont préfixées de `%`.

Pour inclure un autre fichier: `%INCLURE fichier.algo`.\
Le `fichier.algo` sera d'abord cherché dans le chemin d'inclusion spécifié via
l'option `-I`, puis dans le chemin de la librairie standard.

[RAM]: https://zanotti.univ-tln.fr/ALGO/I31/MachineRAM.html