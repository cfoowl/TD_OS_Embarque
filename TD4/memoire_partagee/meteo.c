/** ====================================================================
**   Auteur  : FERRERE                | Date    : 05/02/2017
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 4.0
**  --------------------------------------------------------------------
**   Nom fichier : meteo.c            | Version : 1.0
**  --------------------------------------------------------------------
**   Description :lit en permanence les trames provenant de la station 
**                météorologique WMR928N, affiche les dernières mesures
**                de la température intérieure, de la vitesse et de la
**                direction du vent lorsqu'une trame contenant ces
**                données est reçue et analysée.
**                calcule les valeurs moyennes tous les t temps (période
**                en minutes) et les stocke dans un tableau sur une durée
**                de 24 heures, le tableau est remis à 0 au bout de 24 h.
**                se termine lorsqu'on appuie sur la touche <q>
** =====================================================================*/

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

/* -------------------- déclaration des constantes -------------------- */
// tableau contenant le taille des trames suivant leur type
const int lTrame[NB_TRAME] = {
    SIZE_TRAME00, SIZE_TRAME01, SIZE_TRAME02, SIZE_TRAME03,
  0, SIZE_TRAME05, SIZE_TRAME06, 0, 0, 0, 0, 0, 0, 0,
    SIZE_TRAME0E, SIZE_TRAME0F };
    
/* ---------------- déclaration des variables globales ---------------- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

int       nouvTrame,          // indique la disponibilité d'une nouvelle trame
          majAff,             // indique la mise à jour de l'affichage
          fdSer;              // descripteur du port série

trame_t   trame;              // contient la dernière trame reçue
meteo_t   relMeteo;           // contient les derniers relevés météo

int       iTempInt;           // indice des données de température intérieure
// int       iTempExt;           // indice des données de température extérieure
float     tempInt[MAX_REL];   // stockage de la température intérieure
// float     tempExt[MAX_REL];   // stockage de la température extérieure

// int       iVent;              // indice des données liées au vent
// float     ventVit[MAX_REL];   // stockage de la vitesse du vent
// uint16_t  ventDir[MAX_REL];   // stockage de la direction du vent
//
// meteo_pl_t moyenneMeteo[MAX_MOY];  // stockage des valeurs moyennes
//
int       nbMoy;              // nbre max. de moyennes à calculer (fonct. période)
// int       iMoy;
int       flagAffMoy;         // drapeau pour afficher les moyennes

int fd;
shmem_t * ptr;


struct termios oldTerm;       // sauvegarde configuration du port série

/*======================================================================
**  Nom          : main
**  Description  : démarre les threads, attend la fin de TLireClavier,
**                 termine les threads TLireSerie et TAffichage
** ---------------------------------------------------------------------
**  ValRetournée : EXIT_FAILURE si erreur
**                 EXIT_SUCCESS sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**    2[E]       : nombre d'arguments (argc), liste des arguments (argv)
**======================================================================*/
int main (int argc, char *argv[])
{
  int   err, i;
  int   periode;              // période des calculs de moyennes en min.
  
  pthread_t threadID[NB_THREADS];
  
  if (argc != 2)
  {
    fprintf( stderr, "\nusage : %s periode\n\n", argv[0]);
    fprintf( stderr, "        periode des calculs des moyennes en minute, 0=defaut 1 heure\n");
    return EXIT_FAILURE;
  }

  sscanf(argv[1], "%d", &periode);
  if (periode == 0)
    periode = PERIODE_1H;     // période par défaut d'une heure
    
  _clrscr();
  
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

  // initialise les threads
  Init( periode, threadID );
  
  err = pthread_join (threadID[0], NULL); // attend la fin de TLireClavier
  if (err)
    fprintf( stderr, "Erreur de jonction de TLireClavier : %s\r\n", strerror(err));
  for (i=1; i<NB_THREADS; i++)
    pthread_cancel (threadID[i]);         // arrête tous les autres threads
  close(fd);
  munmap(ptr, sizeof(shmem_t));
  shm_unlink("meteo");
  FermerLiaison( fdSer, &oldTerm );
  
  return EXIT_SUCCESS;
}

/*======================================================================
**  Nom         : Init
**  Description : configure les attributs d'ordonnancement des 5 threads
**                crée et lance les 5 threads :
**                TLireSerie, TTraiterTrame, TAffichage, TMoyenne
**                et TLireClavier
** ---------------------------------------------------------------------
**  Parametres  : [E] [S] [ES]
**      1 [E]   : periode des calculs des moyennes
**      1 [S]   : table des identifiants de Threads
**===========periode===========================================================*/
void Init (int periode, pthread_t threadID[])
{
  pthread_attr_t attrib;
  struct sched_param param;    // pour la priorité
  int err;
  
  nouvTrame = 0;   // inhibe le traitement des trames
  majAff = 0;      // inhibe l'affichage des trames

  // raz des indices des tableaux de mesures
  iTempInt = 0;
  ptr->iTempExt = 0;
  ptr->iVent = 0;
  ptr->iMoy = 0;
  flagAffMoy = 0;     // pas d'affichage des moyennes
  
  pthread_mutex_init(&mutex, NULL);		// initialisation du mutex
  pthread_cond_init( &cond, NULL );
  

  // initialisation des Threads
  err = pthread_attr_init (&attrib);
  if (err != 0)
  {
    perror("Echec de la création d'attribut\n");
    exit (EXIT_FAILURE);
  }
  pthread_attr_setinheritsched( &attrib, PTHREAD_EXPLICIT_SCHED );
  pthread_attr_setschedpolicy (&attrib, SCHED_RR);	// Round-Robin
  param.sched_priority = 12;						// priorité 12
  pthread_attr_setschedparam (&attrib, &param);
  pthread_create( &threadID[1], NULL, &TLireSerie, NULL );
  pthread_create( &threadID[2], NULL, &TTraiterTrame, NULL );
  pthread_create( &threadID[3], NULL, &TAffichage, (void *)periode );
  pthread_create( &threadID[4], NULL, &TMoyenne, (void *)periode );
  pthread_create( &threadID[0], NULL, &TLireClavier, NULL ); 
}

/*======================================================================
**  Nom          : TLireSerie
**  Description  : Thread de lecture du port série
**                 Ouvre le port série COM_METEO
**                 Récupère une trame provenant de la station météo
**                 met à jour la variable nouvTrame et réveille le thread
**                 TTraiterTrame, la synchronisation des threads est 
**                 assurée par un mutex et une variable condition
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1[E]     : arg non utilisé
**======================================================================*/
void *TLireSerie (void *arg)
{
  int     i=0, err;
  int     typeTrame=-1, tailleTrame=0;
  uint8_t car, trameRec[MAX_TRAME];
  
  // ouvre la liaison série reliée à la station météo
  fdSer = OuvrirLiaison ( COM_METEO, &oldTerm);
  if (fdSer == -1)
  {
    exit( EXIT_FAILURE );
  }
    
  while (1)
  {      
    // recherche le début d'une trame: 2 caractéres 0xFF qui se suivent
    i = 0;
    do {
      err = read( fdSer, &car, 1 );
      if (err > 0)
      {
        switch (i)
        {
          case 0 :
          case 1 :
            if (car == 0xFF)
              trameRec[i] = car;
            else
              i = -1;
            break;
          case 2 :
            typeTrame = car;
            trameRec[i] = typeTrame;
            break;
        }
        i++;
      }
    } while (i < SIZE_HEAD_FRAME);

    tailleTrame = lTrame[typeTrame];
    i = 0;

    if ((car >= NB_TRAME) || (tailleTrame == 0))
    {
      fputs( "Trame inconnue\r", stderr );
    }
    else
    // réception du début d'une trame
    // lecture ensuite des données de la trame
    {
      i = 0;
      do
      {
        err = read( fdSer, &trameRec[SIZE_HEAD_FRAME + i], 1 );
         if (err == -1)
        {
          fprintf( stderr, "Erreur lecture sur le port com %s\r\n", strerror(errno) );
          exit (EXIT_FAILURE);
        }
        i++;
      } while (i < tailleTrame);
    
      pthread_mutex_lock(&mutex);
      memcpy(&trame, (trame_t *) trameRec, MAX_TRAME);
      trame.dataLen = tailleTrame;
      nouvTrame = 1;
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock(&mutex);

    }
  } 
  return NULL;
}

/*======================================================================
**  Nom          : TTraiterTrame
**  Description  : Thread attend qu'une trame complète soit disponible
**                 pour la traiter,
**                 l'attente se fait par variable condition,
**                 le thread est réveillé par TLireSerie
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1[E]     : arg non utilisé
**======================================================================*/
void *TTraiterTrame (void *arg)
{ 
  while (1)
  {
    pthread_mutex_lock( &mutex );
    while(nouvTrame == 0)
    {
      pthread_cond_wait(&cond, &mutex);
    }
    int typeTrame = trame.head[2];
    switch (typeTrame) {
      case 0:
        if (AnalyserTrame00(&trame, &relMeteo) == -1)
        {
          printf("Mauvaise trame");
        }
        ptr->ventVit[ptr->iVent] = relMeteo.ventVit;
        ptr->ventDir[ptr->iVent] = relMeteo.ventDir;
        ptr->iVent++;

        majAff = 1;
        break;
      case 2:
        if (AnalyserTrame02(&trame, &relMeteo) == -1)
        {
          printf("Mauvaise trame");
        }
        ptr->tempExt[ptr->iTempExt] = relMeteo.tempExt;
        ptr->iTempExt++;

        majAff = 2;
        break;
      case 3:
        if (AnalyserTrame03(&trame, &relMeteo) == -1)
        {
          printf("Mauvaise trame");
        }
        ptr->tempExt[ptr->iTempExt] = relMeteo.tempExt;
        ptr->iTempExt++;

        majAff = 2;
        break;
      case 5:
        if (AnalyserTrame05(&trame, &relMeteo) == -1)
        {
          printf("Mauvaise trame");
        }

        tempInt[iTempInt] = relMeteo.tempInt;
        iTempInt++;
        majAff = 2;
        break;
      case 6:
        if (AnalyserTrame06(&trame, &relMeteo) == -1)
        {
          printf("Mauvaise trame");
        }

        tempInt[iTempInt] = relMeteo.tempInt;
        iTempInt++;
        majAff = 2;
        break;
      default:
        printf("Mauvais type de trame\r\n");
        break;

    }
    
    nouvTrame = 0;
    pthread_cond_broadcast( &cond );
    pthread_mutex_unlock( &mutex );
  }  
  return NULL;
}

/*======================================================================
**  Nom          : TAffichage
**  Description  : Thread attend qu'une trame complète soit disponible
**                 pour l'afficher,
**                 l'attente se fait par variable condition
**                 le thread est réveillé par TTraiterTrame
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1[E]     : arg non utilisé
**======================================================================*/
void *TAffichage (void *periode)
{
  puts( "*** Station Meteorologique ***\r\n");

  while (1)
  {
    pthread_mutex_lock( &mutex);
    while(majAff == 0)
    {
      pthread_cond_wait(&cond, &mutex);
    }
    // Trame vent
    if(majAff == 1)
    {
      if(ptr->iVent > 0) {
        printf("Ind%d : Update vent : \r\n", ptr->iVent);
        printf("Dir : %d\r\n", ptr->ventDir[ptr->iVent-1]);
        printf("Vit : %f Km/h\r\n", ptr->ventVit[ptr->iVent-1]);
      }

    }
    // Trame température
    else if(majAff == 2){
      if(ptr->iTempExt > 0) {
        printf("Ind%d : Update température : \r\n", ptr->iTempExt);
        printf("Temp : %f C\r\n", ptr->tempExt[ptr->iTempExt-1]);
      }
    } 
    // Moyenne
    else if(majAff == 3)
    {
      printf("Ind%d : Affichage de la moyenne : \r\n", ptr->iMoy);
      if (ptr->iMoy > 0) { 
        printf("Dir : %d\r\n", ptr->moyenneMeteo[ptr->iMoy-1].ventDir);
        printf("Vit : %f Km/h\r\n", ptr->moyenneMeteo[ptr->iMoy-1].ventVit);
        printf("Temp : %f C\r\n", ptr->moyenneMeteo[ptr->iMoy-1].temp);
    }

    }

    majAff = 0;    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock( &mutex);
  }  
  return NULL;
}

/*======================================================================
**  Nom          : TMoyenne
**  Description  : Thread calcule la moyenne des relevés météorologiques
**                 (température, vitesse et direction du vent) à chaque
**                 période fixée en minutes, sauvegarde dans un tableau
**                 les relevés
**                 au bout de 24 heures, remise à zéro des tableaux
**                 des moyennes
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1[E]     : arg non utilisé
**======================================================================*/
void *TMoyenne (void *periode)
{
  int   i;
  float somTemp, moyTemp;
  int  somVentDir, moyVentDir;
  float somVentVit, moyVentVit;
  
  // nombre de calculs à effectuer sur 24 heures, période en minutes
  nbMoy = (60.0 / (int)periode) * 24;
                           
  while (1)
  {
       
    somVentVit= 0;
    somVentDir = 0;
    for(int i = 0; i < ptr->iVent; i++) {
      somVentVit += ptr->ventVit[i];
      somVentDir += ptr->ventDir[i];
    }

    somTemp = 0;
    for(int i = 0; i < ptr->iTempExt; i++) {
      somTemp += ptr->tempExt[i];
    }
    moyVentVit = somVentVit/(ptr->iVent+1); 
    moyVentDir = somVentDir/(ptr->iVent+1);
    moyTemp = somTemp/(ptr->iTempExt+1);

    pthread_mutex_lock(&mutex);
    ptr->moyenneMeteo[ptr->iMoy].ventVit = moyVentVit;
    ptr->moyenneMeteo[ptr->iMoy].ventDir = moyVentDir;
    ptr->moyenneMeteo[ptr->iMoy].temp = moyTemp;

    ptr->iMoy = (ptr->iMoy + 1) % nbMoy;
    majAff = 3;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    sleep((int)periode*10);
  }  
  return NULL;
}

/*======================================================================
**  Nom          : TLireClavier
**  Description  : Thread exécute les commandes des utilisateurs
**                 configure l'entrée du clavier dans le mode raw
**                 lit le clavier jusqu'à l'appui sur la touche <q>
**                 configure l'entrée du clavier dans le mode edited
**                 avant de quitter le thread
**======================================================================*/
void *TLireClavier (void *arg)
{
  char touche='a';
  
 _raw( STDIN_FILENO );     // configuration du clavier dans le mode raw
  do
  {
    touche = getchar();
    switch (touche) {
      case 'q':
        break;
      default:
        printf( "Commande incorrecte ! %c\r\n", touche );
        break;
    }

  } while (touche != 'q');
  
 _unraw( STDIN_FILENO );   // configuration du clavier dans le mode edited

  return NULL;
}

/*======================================================================
**  Nom         : OuvrirLiaison
**  Description : ouvre le port série communiquant avec la station météo
**                paramètres: contrôle de flux matériel, 9600 bauds,
**                8 bits données, pas de parité, 1 bit de stop
** ---------------------------------------------------------------------
**  retour      : descripteur port série ouvert
**                -1 si problème à l'ouverture
** ---------------------------------------------------------------------
**  Parametres  : [E] [S] [ES]
**      1 [E]   : nom du port série à ouvrir
**      1 [S]   : structure pour sauvegarder des paramètres du terminal
**                associé au port
**======================================================================*/
int OuvrirLiaison (const char *portcom, struct termios *pterm)
{ 
  struct  termios new_term;
  speed_t speed;
  int     fd;
  
  if ((fd=open(portcom, O_RDONLY )) == -1 )
  {
    fprintf( stderr, "Erreur lors de l'ouverture du port com %s\r\n", strerror(errno) );
    return -1;
  }

  tcgetattr( fd, pterm );
  tcgetattr( fd, &new_term );

  /* mode raw, pas de mode canonique, pas d'écho */
  new_term.c_iflag = IGNBRK;
  new_term.c_oflag = 0L;
  new_term.c_lflag = 0L;
  /* 1 caractére suffit */
  new_term.c_cc[VMIN] = 1;
  /* donnée disponible immédiatement */
  new_term.c_cc[VTIME] = 0;
  
  /* contrôle de flux matériel (RTS/CTS) */
  new_term.c_cflag |= CREAD;
  /* 8 bits de données,pas de parité, 1 bit de stop */
  new_term.c_cflag &= ~(CSIZE |PARENB  |CSTOPB);
  new_term.c_cflag |= CS8;
  /* ignorer les signaux de contrôle du modem */
  new_term.c_cflag |= CLOCAL;
  /* inhibe le contrôle de flux logiciel XON-XOFF */
  new_term.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  /* 9600 Bauds en émission et en réception */
  speed = B9600;
  cfsetispeed( &new_term, speed );
  cfsetospeed( &new_term, speed );
  
  tcsetattr( fd, TCSANOW, &new_term );  // changement immédiat des paramètres
  
  return fd;
}

/*======================================================================
** Nom        : FermerLiaison
**Description : ferme le port serie
** ---------------------------------------------------------------------
**  Parametres  : [E] [S] [ES]
**      2 [E]   : descripteur du port série ouvert
**                sauvegarde des paramètres du terminal associé au port
**======================================================================*/
void FermerLiaison (int fd, struct termios *pterm)
{
  tcsetattr( fd, TCSANOW, pterm );
  close( fd );
}
