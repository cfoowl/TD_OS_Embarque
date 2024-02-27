#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "terminal.h"

#define LED1 4
#define LED2 5
void * TCommande (void* arg);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void) {
  wiringPiSetup();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  pthread_t threadID[1];
  pthread_create( &threadID[0], NULL, &TCommande, NULL);

  int err = pthread_join(threadID[0], NULL);
  if(err)
    printf("oui");
  return 0;
 

  // for(;;)
  // {
    // digitalWrite(LED1,HIGH);
    // delay(250);
    // digitalWrite(LED2, HIGH);
    // delay(250);
    // digitalWrite(LED1, LOW);
    // delay(250);
    // digitalWrite(LED2, LOW);
    // delay(250);
  // }
  // return 0;
}

void * TCommande (void* arg)
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
      case 'r':
        if (digitalRead(LED1))
          digitalWrite(LED1, LOW);
        else
          digitalWrite(LED1, HIGH);
        break;
      case 'g':
        if (digitalRead(LED2))
          digitalWrite(LED2, LOW);
        else 
          digitalWrite(LED2, HIGH);
        break;
      case 'v':
        if (digitalRead(LED2))
          digitalWrite(LED2, LOW);
        else 
          digitalWrite(LED2, HIGH);
        break;

      case 'q':
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        break;
      default :
        printf( "Commande incorrecte ! %c\r\n", touche );
        break;
    }
    pthread_mutex_unlock( &mutex );   // libère le mutex

  } while (touche != 'q');

  _unraw( STDIN_FILENO );  // configuration du clavier dans le mode edited

  return NULL;
}
