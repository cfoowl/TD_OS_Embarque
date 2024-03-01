#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "terminal.h"

#define	Clock	27
#define	Address	28
#define	DataOut	29
#define LED 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int ADC_Read(unsigned char channel)
{
	unsigned int value;
	unsigned char i;
	unsigned char LSB = 0, MSB = 0;
 
	channel = channel << 4;
	for (i = 0; i < 4; i ++) 
	{
		if(channel & 0x80)
			digitalWrite(Address,1);
		else 
			digitalWrite(Address,0);
		digitalWrite(Clock ,1);
		digitalWrite(Clock ,0);
		channel = channel << 1;
	}
	for (i = 0; i < 6;i ++) 
	{
		digitalWrite(Clock ,1);
		digitalWrite(Clock ,0);
	}

	delayMicroseconds(15);
	for (i = 0; i < 2; i ++) 
	{
		digitalWrite(Clock ,1);
		MSB <<= 1;
		if (digitalRead(DataOut))
			MSB |= 0x1;
		digitalWrite(Clock ,0);
	} 
	for (i = 0; i < 8; i ++) 
	{
		digitalWrite(Clock ,1);
		LSB <<= 1;
		if (digitalRead(DataOut))
			LSB |= 0x1;
		digitalWrite(Clock ,0);
	} 
	value = MSB;
	value <<= 8;
	value |= LSB;
	return value; 
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
      case 'q':
        digitalWrite(LED, LOW);
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

void * TLed (void* arg)
{
  while(1)
  {
    int temp = ADC_Read(5);
    pthread_mutex_lock( &mutex ); 
    if(temp > 500)
      digitalWrite(LED, HIGH);
    else {
      digitalWrite(LED, LOW);
    }
    pthread_mutex_unlock( &mutex );
    _clrscr();
  	printf("AD: %d \r\n",temp);
		delay(2000);
  }
}

int main()
{
	if (wiringPiSetup() < 0)return 1 ;

	pinMode (DataOut,INPUT);
	pullUpDnControl(DataOut, PUD_UP);

	pinMode (Clock,OUTPUT);
	pinMode (Address,OUTPUT);
  pinMode (LED, OUTPUT);
  
  pthread_t threadID[2];
  pthread_create( &threadID[0], NULL, &TCommande, NULL);
  pthread_create( &threadID[1], NULL, &TLed, NULL);

  int err = pthread_join(threadID[0], NULL);
  if(err)
    fprintf( stderr, "Erreur de jonction de TCommande : %s\r\n", strerror(err));
  pthread_cancel(threadID[1]);
  
  return EXIT_SUCCESS;

}
