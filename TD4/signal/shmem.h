/** ====================================================================
**   Auteur  : FERRERE                | Date    : 13/11/2019
**  --------------------------------------------------------------------
**   Langage : C                      | Système : Linux 4.x
**  --------------------------------------------------------------------
**   Nom fichier : shmem.h            | Version : 1.0
**  --------------------------------------------------------------------
**   Description : fichier en-tête
** =====================================================================*/
#include <semaphore.h>
#include <stdint.h>

#ifndef __SHMEM_H__
#define  __SHMEM_H__

#define MAX_MOY     24    // nombre de moyennes à calculer

// structure des données météo pour le panneau lumineux
typedef struct
{
  float     temp;               // température ambiante
  float     ventVit;            // vitesse du vent
  uint16_t  ventDir;            // direction du vent
} meteo_pl_t;

// structure de la mémoire partagée
typedef struct {
  sem_t      semaphore;
  meteo_pl_t releve;             // derniers relevés
  meteo_pl_t moyReleve[MAX_MOY]; // valeurs moyennes
  int        indRelMoy;          // indice du tableau des moyennes
} shmem_t;

shmem_t *InitShmem (void);

#endif  // ifndef __SHMEM_H__
