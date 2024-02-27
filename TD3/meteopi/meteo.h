/** ====================================================================
**   Auteur  : FERRERE                | Date    : 17/11/2013
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 2.6
**  --------------------------------------------------------------------
**   Nom fichier : meteo.h            | Version : 1.0
**  --------------------------------------------------------------------
**   Description : gestion de la station météorologique WMR928N
** =====================================================================*/

#include "wmr928.h"
#include <termios.h>

#ifndef __METEO_H__
#define  __METEO_H__

/* -------------------- Déclaration des constantes -------------------- */
#define NB_THREADS    5
#define MAX_REL       240   // nombre maximum pour un type de relevés sur 1 heure

#define PERIODE_1H    60    // période 1 h. pour le calcul des moyennes en min.
#define MAX_MOY       1440  // 24 * 60 si calcul toutes les minutes

#define COM_METEO     "/dev/ttyUSB0"   // port communication avec la station

/* ----------------------- Déclaration des types ---------------------- */
// structure des données météo pour le panneau lumineux
typedef struct 
{
  float     temp;           // température ambiante
  float     ventVit;        // vitesse du vent
  uint16_t  ventDir;        // direction du vent
} meteo_pl_t;


/* -------------------- Prototypage des fonctions --------------------- */
void Init (int periode, pthread_t threadID[]);
void *TLireSerie (void *arg);
void *TTraiterTrame(void *arg);
void *TAffichage (void *arg);
void *TMoyenne (void *arg);
void *TLireClavier (void *arg);

int OuvrirLiaison (char *portcom, struct termios *pterm);
void FermerLiaison (int fd, struct termios *pterm);

#endif  // ifndef __METEO_H__
