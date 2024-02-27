/** ====================================================================
**   Auteur  : FERRERE                | Date    : 10/01/2015
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : linux 2.6
**  --------------------------------------------------------------------
**   Nom fichier : filedyn.h          | Version : 1.0
**  --------------------------------------------------------------------
**   Description : En-tête gestion d'une file
** =====================================================================*/

#ifndef __FILEDYN_H__
#define __FILEDYN_H__

/* -------------------- déclaration des constantes -------------------- */
enum { FILE_OK,
       ERR_FILE_MEM,
       ERR_FILE_VIDE};

enum {FALSE, TRUE};

/* ----------------------- déclaration des types ---------------------- */
struct file {
       int          val;
       struct file  *psuiv;
};

typedef struct file file_t;

/* --------------------- prototypage des fonctions -------------------- */

/*======================================================================
**  Nom          : Enfiler
**  Description  : enfile un élément
** ---------------------------------------------------------------------
**  ValRetournée : FILE_OK si élément est dans la file
**                 ERR_FILE_MEM si problème allocation mémoire
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : la valeur à enfiler
**      1 [S]    : file d'attente
**======================================================================*/
int Enfiler(file_t **file, int nbre);

/*======================================================================
**  Nom          : Defiler
**  Description  : défile le premier élément
** ---------------------------------------------------------------------
**  ValRetournée : FILE_OK si l'élément est retiré de la file
**                 ERR_FILE_VIDE si la file est vide
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [ES]   : file d'attente
**      1 [S]    : élément défilé
**======================================================================*/
int Defiler(file_t **file, int *nbre);

/*======================================================================
**  Nom          : Lister
**  Description  : liste la file à l'écran
** ---------------------------------------------------------------------
**  ValRetournée : FILE_OK si les éléments de la file ont été listés
**                 ERR_FILE_VIDE si la file est vide
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : file d'attente
**======================================================================*/
int Lister(file_t *file);

/*======================================================================
**  Nom          : Vider
**  Description  : vide la file
** ---------------------------------------------------------------------
**  ValRetournée : FILE_OK si la file a été vidée
**                 ERR_FILE_VIDE si la file est vide
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [ES]   : file d'attente
**======================================================================*/
int Vider(file_t **file);

/*======================================================================
**  Nom          : EstPresent
**  Description  : vérifie si un nombre se trouve dans la file
** ---------------------------------------------------------------------
**  ValRetournée : TRUE si nbre se trouve déjà dans la file
**                 FALSE sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : file d'attente
**======================================================================*/
int EstPresent (file_t *file, int nbre);

#endif    // ifndef __FILEDYN_H__
