#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "terminal.h"
#include "wmr928.h"
#include "meteo.h"
int fd;
shmem_t * ptr;


int main() {
  // Initialise la mémoire partagée
  // Création
  fd = shm_open("meteo", O_RDWR | O_CREAT, S_IRWXU);

  if (fd == -1)
  {
    printf("meteo: erreur a la creation de la memoire partagee 'meteo': %s\n",
    strerror(errno) );
    exit( EXIT_FAILURE );
  }
  // Dimensionnement
  ftruncate(fd, sizeof(shmem_t));
  
  // pointe sur une zone de la mémoire partagée
  ptr = mmap(0, sizeof(shmem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  // Initialise le semaphore
  sem_init( &ptr->semaphore, 1, 0 );
  
  ptr->iMoy=2;
  ptr->iVent=1;
  ptr->iTempExt=1;
  ptr->tempExt[0] = 1; 
  ptr->ventVit[0] = 2;  
  ptr->ventDir[0] = 3;   
  ptr->moyenneMeteo[0].ventDir = 1;
  ptr->moyenneMeteo[0].ventVit = 2;
  ptr->moyenneMeteo[0].temp = 3;
  ptr->moyenneMeteo[1].ventDir = 3;
  ptr->moyenneMeteo[1].ventVit = 2;
  ptr->moyenneMeteo[1].temp = 1;




  sleep(20);



  close(fd);
  munmap(ptr, sizeof(shmem_t));
  shm_unlink("meteo");
}
