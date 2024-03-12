/** ====================================================================
**   Auteur  : FOURDRINIER            | Date    : 11/03/2024
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 4.0
**  --------------------------------------------------------------------
**   Nom fichier : affiche_meteo.c    | Version : 1.0
**  --------------------------------------------------------------------
**   Description : Lit la memoire partagee avec meteo.c et affiche
**   les dernières valeurs relevées si on appuie sur A, toutes les 
**   valeurs moyennes en appuyant sur M, et quitte en appuyant sur Q
** =====================================================================*/

#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#include "terminal.h"
#include "shmem.h"

int fd;
shmem_t * ptr;

void handler(int signal_number) {

  printf("Vent : \r\n", ptr->iVent);
  printf("Dir : %d\r\n", ptr->ventDir[ptr->iVent-1]);
  printf("Vit : %f Km/h\r\n", ptr->ventVit[ptr->iVent-1]);
}

int main (int argc, char *argv[])
{
  
  if (argc != 2)
  {
    fprintf( stderr, "\nusage : %s nom_objet_partage\n\n", argv[0]);
    printf("Exemple : affichage meteo");
    return EXIT_FAILURE;
  }


  _clrscr();
  
  /* crée la mémoire partagée */
  fd = shm_open("signal", O_RDWR | O_CREAT, S_IRWXU);
  if (fd == -1)
  {
    printf("signal_affichage: erreur a la creation de la memoire partagee 'signal': %s\n",
    strerror(errno) );
    exit( EXIT_FAILURE );
  }

  /* dimensionne la taille de la mémoire partagée */
  ftruncate(fd, sizeof(shmem_t));

  /* pointe sur une zone de la mémoire partagée */
  ptr = mmap(0, sizeof(shmem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  // initialise le sémaphore
  sem_init( &ptr->semaphore, 1, 0 );

  if(ptr->iVent > 0) {
    printf("Ind%d : Vent : \r\n", ptr->iVent);
    printf("Dir : %d\r\n", ptr->ventDir[ptr->iVent-1]);
    printf("Vit : %f Km/h\r\n", ptr->ventVit[ptr->iVent-1]);
  } 
  if(ptr->iTempExt > 0) {
    printf("Ind%d : Température : \r\n", ptr->iTempExt);
    printf("Temp : %f C\r\n", ptr->tempExt[ptr->iTempExt-1]);
  }

  close(fd);
  munmap(ptr, sizeof(shmem_t));
  
  return EXIT_SUCCESS;
}

 
