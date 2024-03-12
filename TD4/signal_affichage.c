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

#include "terminal.h"
#include "meteo.h"


int fd;
shmem_t * ptr;

int main (int argc, char *argv[])
{
  
  if (argc != 2)
  {
    fprintf( stderr, "\nusage : %s nom_objet_partage\n\n", argv[0]);
    printf("Exemple : affichage meteo");
    return EXIT_FAILURE;
  }


  _clrscr();
  
  // Initialise la mémoire partagée
  // Création
  fd = shm_open(argv[1], O_RDWR, S_IRWXU);

  if (fd == -1)
  {
    printf("affichage: erreur d'accès à la memoire partagee %s : %s\n",
        argv[1],
        strerror(errno) );
    exit( EXIT_FAILURE );
  }
  // pointe sur une zone de la mémoire partagée
  ptr = mmap(0, sizeof(shmem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

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

 
