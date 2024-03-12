/** ====================================================================
**   Auteur  : FERRERE                | Date    : 03/11/2017
**  --------------------------------------------------------------------
**   Langage : C                      | Systeme : linux 4.0
**  --------------------------------------------------------------------
**   Nom fichier : wmr928.h           | Version : 1.0
**  --------------------------------------------------------------------
**   Description : gestion de la station météorologique WMR928
** =====================================================================*/

#include <sys/types.h>

#ifndef __WMR928_H__
#define __WMR928_H__

/* -------------------- Déclaration des constantes -------------------- */
#define NB_TRAME          16    // nombre de types de trames
#define SIZE_HEAD_FRAME    3    // taille en-tête des trames
#define SIZE_DATA_FRAME   13    // taille max. trame de données
#define MAX_TRAME         SIZE_HEAD_FRAME + SIZE_DATA_FRAME

#define TYPE               2    // rang dans la trame

// types de trame
enum {TYPE00, TYPE01, TYPE02, TYPE03, TYPE05=5, TYPE06, TYPE0E=14, TYPE0F};

#define SIZE_TRAME00       8    // longueur trame de type 00
#define SIZE_TRAME01      13    // longueur trame de type 01
#define SIZE_TRAME02       6    // longueur trame de type 02
#define SIZE_TRAME03       6    // longueur trame de type 03
#define SIZE_TRAME05      10    // longueur trame de type 05
#define SIZE_TRAME06      11    // longueur trame de type 06
#define SIZE_TRAME0E       2    // longueur trame de type 0E
#define SIZE_TRAME0F       6    // longueur trame de type 0F

enum {TRAME_OK = 0,
      ERR_TRAME = -1 };
      
/* ----------------------- Déclaration des types ---------------------- */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;

// format d'une trame
typedef struct
{
  uint8_t  head[SIZE_HEAD_FRAME];
  uint8_t  data[SIZE_DATA_FRAME];
  int      dataLen;
} trame_t;

// structure des données météorologiques
typedef struct 
{
  float     tempInt;     // température intérieure
  int       humidInt;    // humidité intérieure
  float     tempExt;     // température extérieure
  int       humidExt;    // humidité extérieure
  float     ventVit;     // vitesse du vent
  uint16_t  ventDir;     // direction du vent
  uint16_t  pluvio;      // pluviométrie
  int       rosee;       // point de rosée
  int       baro;        // pression atmosphérique
  char      tendAtm;     // tendance atmosphérique
} meteo_t;

/* -------------------- Prototypage des fonctions --------------------- */
/*======================================================================
**  Nom          : Analyser_Trame00 
**  Description  : analyse une trame de type 00 et met à jour dans une
**                 structure meteo_t la direction et la vitesse du vent
** ---------------------------------------------------------------------
**  ValRetournée : ERR_TRAME (-1) si mauvais format de trame
**                 TRAME_OK  ( 0) sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : trame de type OO
**      1 [S]      les champs direction et vitesse du vent sont mis à jour
**======================================================================*/
int AnalyserTrame00 (trame_t * trame, meteo_t *rel_meteo);

/*======================================================================
**  Nom          : Analyser_Trame02 
**  Description  : analyse une trame de type 02 et met à jour dans une
**                 structure meteo_t la température, l'humidité
**                 et le point de rosée
** ---------------------------------------------------------------------
**  ValRetournée : ERR_TRAME (-1) si mauvais format de trame
**                 TRAME_OK  ( 0) sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : trame de type O2
**      1 [S]      les différents champs sont mis à jour
**======================================================================*/
int AnalyserTrame02 (trame_t * trame, meteo_t *rel_meteo);

/*======================================================================
**  Nom          : Analyser_Trame03 
**  Description  : analyse une trame de type 03 et met à jour dans une
**                 structure meteo_t la température, l'humidité
**                 et le point de rosée
** ---------------------------------------------------------------------
**  ValRetournée : ERR_TRAME (-1) si mauvais format de trame
**                 TRAME_OK  ( 0) sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : trame de type O3
**      1 [S]      les différents champs sont mis à jour
**======================================================================*/
int AnalyserTrame03 (trame_t * trame, meteo_t *rel_meteo);

/*======================================================================
**  Nom          : Analyser_Trame05 
**  Description  : analyse une trame de type 05 et met à jour dans une
**                 structure meteo_t la température, l'humidité, le point
**                 de rosée et la pression atmosphérique à l'intérieur
** ---------------------------------------------------------------------
**  ValRetournée : ERR_TRAME (-1) si mauvais format de trame
**                 TRAME_OK  ( 0) sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : trame de type O5
**      1 [S]      les différents champs sont mis à jour
**======================================================================*/
int AnalyserTrame05 (trame_t * trame, meteo_t *rel_meteo);

/*======================================================================
**  Nom          : Analyser_Trame06 
**  Description  : analyse une trame de type 06 et met à jour dans une
**                 structure meteo_t la température, l'humidité, le point
**                 de rosée et la pression atmosphérique à l'intérieur
** ---------------------------------------------------------------------
**  ValRetournée : ERR_TRAME (-1) si mauvais format de trame
**                 TRAME_OK  ( 0) sinon
** ---------------------------------------------------------------------
**  Parametres   : [E] [S] [ES]
**      1 [E]    : trame de type O6
**      1 [S]      les différents champs sont mis à jour
**======================================================================*/
int AnalyserTrame06 (trame_t * trame, meteo_t *rel_meteo);

#endif  // ifndef __WMR928_H__
