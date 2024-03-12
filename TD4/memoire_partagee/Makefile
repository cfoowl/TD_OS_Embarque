all: meteo affichage
meteo:meteo.c
	gcc meteo.c -o meteo -L ../../Libs/ -l pthread -l terminal -l wmr928 
affichage:affichage.c
	gcc affichage.c -o affichage -L ../../Libs/ -l pthread -l terminal -l wmr928 
clean: meteo
	rm meteo affichage

