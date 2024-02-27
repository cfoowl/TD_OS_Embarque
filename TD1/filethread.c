/** ====================================================================
**   Auteur  : FERRERE                | Date    : 10/01/2015
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : Linux 3.0
**  --------------------------------------------------------------------
**   Nom fichier : filethread.c       | Version : 1.0
**  --------------------------------------------------------------------
**   Description : gestion d'une file dynamique par des threads
**                 Le thread Tenfiler() génère toutes les 2 secondes
**                 des nombres aléatoires compris entre 1 et 10 et les
**                 insère dans la file
**                 Le thread TDefiler() retire un élément toutes les 5s
**                 Le thread TCommande() permet de lister ou vider la file
**                 en fonction des commandes de l'utilisateur
** =====================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "terminal.h"
#include "filedyn.h"
#include "filethread.h"

/* ---------------- déclaration des variables globales ---------------- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attrib;        // attributs d'ordonnancement des threads
struct sched_param  param;    // pour la priorité

file_t  *fileAttente;

int main (void)
{
  pthread_t  threadID[NB_THREADS];          // sauvegarde ID threads

  _clrscr();
  puts ("Gestion d'une file dynamique par des threads\n\r");
  puts ("Lister\t<l>\r");
  puts ("Vider\t<v>\n\r");
  puts ("Quitter\t<q>\r"); 

  initThreads( threadID );            // création des threads

  int err = pthread_join( threadID[0], NULL );  // attend la fin de TCommandes
  if (err)
    printf ("Erreur de jonction de TCommandes : %s\r\n", strerror(err));
  for (int i=1; i<NB_THREADS; i++)
    pthread_cancel( threadID[i] );     // termine tous les autres threads

  pthread_attr_destroy( &attrib );
  Vider( &fileAttente );

  return EXIT_SUCCESS;
}

/*======================================================================
**  Nom         : initThreads
**  Description : configure les attributs d'ordonnancement des 3 threads
**                fixe les priorités d'exécution
**                crée et lance les 3 threads :
**                TCommande, TEnfiler, TDefiler
** ---------------------------------------------------------------------
**  Parametres  : [E] [S] [ES]
**      1 [S]   : table des identifiants de Threads
**======================================================================*/
void initThreads (pthread_t threadID[])
{
  int err;

  pthread_mutex_init( &mutex, NULL );    // initialisation du mutex

  // initialisation du thread TDefiler()
  err = pthread_attr_init( &attrib );
  if (err != 0)
  {
    perror( "Echec de la création d'attributs\n" );
    exit( EXIT_FAILURE );
  }
  err = pthread_attr_setinheritsched( &attrib, PTHREAD_EXPLICIT_SCHED );
  if (err != 0)
  {
    perror( "Echec de la définition d'ordonnancement explicite\n" );
    exit( EXIT_FAILURE );
  }
  pthread_attr_setschedpolicy( &attrib, SCHED_RR );  // Round-Robin
  param.sched_priority = 12;          // priorité 12
  pthread_attr_setschedparam( &attrib, &param );
  pthread_create( &threadID[1], &attrib, &TDefiler, NULL );
  
  // initialisation du thread TEnfiler()
  param.sched_priority = 15;  // priorité plus élevée que TDefiler
  pthread_attr_setschedparam( &attrib, &param );
  pthread_create( &threadID[2], &attrib, &TEnfiler, NULL );

  // initialisation du thread TCommande()
  pthread_attr_setschedpolicy( &attrib, SCHED_FIFO );  // FIFO
  param.sched_priority = 14;  // priorité plus élevée que TEnfiler
  pthread_attr_setschedparam( &attrib, &param );
  pthread_create( &threadID[0], &attrib, &TCommande, NULL );
}

/*======================================================================
**  Nom         : TCommande
**  Description :  Thread exécutant les commandes des utilisateurs
**                configure l'entrée du clavier dans le mode raw
**                lit le clavier jusqu'à l'appui sur la touche <q>
**                si touche='l' liste les le contenu de la file
**                si touche='v' vide la file
**                configure l'entrée du clavier dans le mode edited
**                avant de quitter le thread
**                utilise un mutex pour la synchronisation
**======================================================================*/
void* TCommande (void* arg)
{
  int    err, nbCar;
  char  touche;

  _raw( STDIN_FILENO );     // configuration du clavier dans le mode raw
  do
  {
    nbCar = read( STDIN_FILENO, &touche, 1 );  // attend l'appui sur une touche
    if (nbCar == -1)
      printf( "Erreur lecture clavier %s\r\n", strerror(errno) );

    pthread_mutex_lock( &mutex );     // prend le mutex s'il est libre
    switch (touche)
    {
      case 'l':                       // liste les éléments de la file
        err = Lister( fileAttente );
        if (err == ERR_FILE_VIDE)
          puts( "Lister: File vide\r" );
        puts( "\r" );
        break;
      case 'v':                       // vide la file
        err = Vider( &fileAttente );
        if (err == ERR_FILE_VIDE)
          puts( "Vider: File vide\r" );
        else
          puts( "File vidée\r" );
        break;
      case 'q':
        break;
      default :
        printf( "Commande incorrecte !\r\n" );
        break;
    }
    pthread_mutex_unlock( &mutex );   // libère le mutex

  } while (touche != 'q');

  _unraw( STDIN_FILENO );  // configuration du clavier dans le mode edited

  return NULL;
}

/*======================================================================
**  Nom         : TEnfiler
**  Description :  Thread mettant dans la file un nombre aléatoire
**                compris entre 1 et 10, si le nombre ne se trouve pas
**                dans la file, toutes les 2 secondes
**                utilise un mutex pour la synchronisation
**======================================================================*/
void* TEnfiler (void* arg)
{
  int  err, nbre;

  while (1)
  {
    nbre = (rand() % 10) + 1;
    sleep( 2 );
    pthread_mutex_lock( &mutex );     // prend le mutex s'il est libre

    if (!EstPresent( fileAttente, nbre ))
    {
      err = Enfiler( &fileAttente, nbre );
      if (err == ERR_FILE_MEM)
        puts( "Erreur d'allocation memoire\n\r" );
      printf( "Valeur enfilee : %d\n\r", nbre );
    }
    else
    {
      printf( "%d : deja present\r\n",  nbre );
    }

    pthread_mutex_unlock( &mutex );   // libère le mutex
  }
  return NULL;
}

/*======================================================================
**  Nom         : TDefiler
**  Description : Thread retirant un élément de la file toutes les 5s
**                utilise un mutex pour la synchronisation
**======================================================================*/
void* TDefiler (void* arg)
{
  int  err, nbre;

  while (1)
  {
    sleep( 5 );
    pthread_mutex_lock( &mutex );       // prend le mutex s'il est libre

    err =  Defiler( &fileAttente, &nbre );
    if (err == ERR_FILE_VIDE)
      puts( "TDefiler: File vide" );
    else
      printf( "valeur retiree de la file : %d\r\n", nbre );

    pthread_mutex_unlock( &mutex );   // libère le mutex
  }
  return NULL;
}
