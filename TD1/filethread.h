/** ====================================================================
**   Auteur  : FERRERE                | Date    : 10/01/2015
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 3.0
**  --------------------------------------------------------------------
**   Nom fichier : filethread.h       | Version : 1.0
**  --------------------------------------------------------------------
**   Description : fichier en-tête gestion file dynamique par des threads
** =====================================================================*/

#ifndef __FILETHREAD_H__
#define	__FILETHREAD_H__

/* ---------------- définition des nouveaux types ---------------- */
typedef unsigned int	bool;

/* ---------------- déclaration des constantes ---------------- */
#define	NB_THREADS		3

/* --------------------- prototypage des fonctions -------------------- */
void	initThreads (pthread_t threadID[]);
void*	TEnfiler (void* arg);
void*	TDefiler (void* arg);
void*	TCommande (void* arg);

#endif
