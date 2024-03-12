/** ====================================================================
**   Auteur  : FERRERE                | Date    : 27/01/2013
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 2.6
**  --------------------------------------------------------------------
**   Nom fichier : terminal.h         | Version : 1.0
**  --------------------------------------------------------------------
**   Description : En-tête librairie terminal
** =====================================================================*/

#ifndef __TERMINAL_H__
#define	__TERMINAL_H__

/* -------------------- déclaration des constantes -------------------- */

/* ----------------------- déclaration des types ---------------------- */

/* --------------------- déclaration des variables -------------------- */

int _raw (int fd);
int _unraw (int fd);
void _clrscr (void);
void _gotoxy(int x,int y);

#endif	// ifndef __TERMINAL_H__
