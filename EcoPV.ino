/*
EcoPV.ino - Arduino program that maximizes the use of home photovoltaïc production
by monitoring energy consumption and diverting power to a resistive charge
when needed.
Copyright (C) 2019 - Bernard Legrand and Mickaël Lefebvre.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*************************************************************************************
**                                                                                  **
**        Ce programme fonctionne sur ATMEGA 328P @ VCC = 5 V et clock 16 MHz       **
**        comme l'Arduino Uno et l'Arduino Nano                                     **
**        La compilation s'effectue avec l'IDE Arduino                              **        
**        Site Arduino : https://www.arduino.cc                                     **        
**                                                                                  **
**************************************************************************************/

// ***********************************************************************************
// ******************            OPTIONS DE COMPILATION                ***************
// ***********************************************************************************

// ***********************************************************************************
// ****************** Affichage et configuration interactive           ***************
// ****************** par terminal liaison série                       ***************
// ****************** Dé-commenter une ligne pour activer une fonction ***************
// ***********************************************************************************

#define PV_STATS             // Active la sortie d'informations statistiques
#define PV_MOD_CONFIG        // Accès interactif à la modification de la configuration

// ***********************************************************************************
// ****************** Dé-commenter pour activer l'affichage            ***************
// ****************** des données sur écran oled 128 x 64  I2C         ***************
// ****************** Voir :                                           ***************
// ***********************************************************************************

#define OLED_128X64

  // *** Note : l'écran utilise la connexion I2C 
  // *** sur les pins A4 (SDA) et A5 (SCK)
  // *** Ces pins ne doivent pas être utilisées comme entrées analogiques  
  // *** si OLED_128X64 est activé.
  // *** La bibliothèque SSD1306Ascii doit être installée dans l'IDE Arduino.
  // *** Voir les définitions de configuration OLED_128X64
  // *** dans la suite des déclarations, en particulier l'adresse de l'écran
 
// ***********************************************************************************
// ****************** Dé-commenter pour activer la communication       ***************
// ****************** des données par MYSENSORS                        ***************
// ****************** Voir : www.mysensors.org                         ***************
// ***********************************************************************************

//#define MYSENSORS_COM

  // *** Note : MYSENSORS utilise les pins D2 D9 D10 D11 D12 et D13
  // *** pour la connexion et l'utilisation de la radio NRF24
  // *** Ces pins ne doivent pas être utilisées pour une autre fonction  
  // *** si MYSENSORS est activé.
  // *** La bibliothèque MYSENSORS doit être installée dans l'IDE Arduino.
  // *** Voir les définitions de configuration MYSENSORS
  // *** dans la suite des déclarations, en particulier l'ID du node

// ***********************************************************************************
// ****************** Dé-commenter pour activer la communication       ***************
// ****************** des données par ETHERNET avec shield ENC28J60    ***************
// ***********************************************************************************

//#define ETHERNET_28J60

  // *** Note : la connexion ethernet nécessite un shield ENC28J60 pour Arduino nano
  // *** Il utilise les pins  D10 D11 D12 et D13
  // *** pour la communication Arduino <> shield
  // *** Ces pins ne doivent pas être utilisées pour une autre fonction  
  // *** si ETHERNET_28J60 est activé.
  
  // *** Les bibliothèques etherShield et ETHER_28J60 doivent installées
  // *** dans l'IDE Arduino.
  // *** L'installation se fait manuellement.
  
  // *** ADRESSAGE DU LIEN TCP/IP pour les requêtes HTTP :
  // *** Voir les définitions de configuration dans la suite des déclarations :
  //     adresse mac, adresse IP et port

  // *** Structure des requêtes :
  // *** http://adresseIP:port/GetXX
  // *** où XX est compris en 01 et 99
  // *** Réponse au format json : {"value":"xxxxx"}
  // *** où xxxxx sera une valeur entière
          // *** XX = 01 : Vrms (V)
          // *** XX = 02 : Irms (A)
          // *** XX = 03 : Pact (W)
          // *** XX = 04 : Papp (VA)
          // *** XX = 05 : Prouted (W)
          // *** XX = 06 : Pimp (W)
          // *** XX = 07 : Pexp (W)
          // *** XX = 08 : cosinus phi * 1000
          // *** XX = 09 : index d'énergie routée (kWh) (estimation)
          // *** XX = 10 : index d'énergie importée (kWh)
          // *** XX = 11 : index d'énergie exportée (kWh)
          // *** XX = 20 : bits d'erreur et de statut (byte)
          // *** XX = 21 : temps de fonctionnement ddd:hh:mm:ss
          // *** XX = 90 : mise à 0 des 3 index d'énergie (réponse : "ok")
          // *** XX = 99 : version logicielle

//                   **************************************************
//                   **********   A  T  T  E  N  T  I  O  N   *********
//                   **************************************************
//                   ***** INCOMPATIBILITE :                  *********
//                   ***** NE PAS ACTIVER MYSENSORS_COM       *********
//                   ***** et/ou ETHERNET_28J60               *********
//                   ***** et/ou OLED_128X64 SIMULTANEMENT    *********
//                   **************************************************

// ***********************************************************************************
// ******************         FIN DES OPTIONS DE COMPILATION           ***************
// ***********************************************************************************

// ***********************************************************************************
// ************************       DEFINITIONS GENERALES        ***********************
// ***********************************************************************************

#define VERSION            "1.0"      // Version logicielle
#define SERIAL_BAUD       500000      // Vitesse de la liaison port série
#define SERIALTIMEOUT      30000      // Timeout pour les interrogations sur liaison série en ms

#define ON                     1 
#define OFF                    0 
#define POSITIVE            true 
#define YES                 true 
#define NO                 false 

#define NB_CYCLES             50      // nombre de cycles / s du secteur AC (50 ou 60 Hz) : 50 ou 60
#define SAMP_PER_CYCLE       166      // Nombre d'échantillons I,V attendu par cycle : 166
                                      // Dépend du microcontrôleur et de la configuration de l'ADC
#define BIASOFFSET_TOL        20      // Tolérance en bits sur la valeur de biasOffset par rapport
                                      // au point milieu 511 pour déclencher une remontée d'erreur


// ***********************************************************************************
// *******************   Déclarations des pins d'entrée/sortie   *********************
// ***********************************************************************************

// ***   I/O analogiques   ***
#define voltageSensorMUX       3    // IN ANALOG   = PIN A3, lecture tension V sur ADMUX 3
#define currentSensorMUX       0    // IN ANALOG   = PIN A0, lecture courant I sur ADMUX 0
                                    // ATTENTION PIN A4 et A5 incompatibles avec activation de OLED_128X64

// ***   I/O digitales     ***
#define synchroACPin           3    // IN DIGITAL  = PIN D3, Interrupt sur INT1, signal de détection du passage par zéro (toggle)
#define synchroOutPin          4    // OUT DIGITAL = PIN D4, indique un passage par zéro détecté par l'ADC (toggle)
#define pulseTriacPin          5    // OUT DIGITAL = PIN D5, commande Triac/Solid State Relay
#define ledPinStatus           6    // OUT DIGITAL = PIN D6, LED de signalisation fonctionnement / erreur
#define ledPinRouting          7    // OUT DIGITAL = PIN D7, LED de signalisation routage de puissance
#define relayPin               8    // OUT DIGITAL = PIN D8, commande On/Off du relais de délestage secondaire

//                   **************************************************
//                   **********   A  T  T  E  N  T  I  O  N   *********
//                   **************************************************

  // A4 et A5 constituent le port I2C qui est utilisé pour l'affichage écran
  // Si OLED_128X64 est activé, ne pas utiliser A4 et A5 comme entrées analogiques ADC pour I ou V
  
  // D0 et D1 sont utilisés par la liaison série de l'Arduino et pour sa programmation
  // D3 est l'entrée d'interruption INT1 et doit être absolument affecté à synchroACPin
  // D2 et D9 sont utilisés pour la radio NRF24 pour communication MYSENSORS (option)
  // D10, D11, D12, D13 sont utilisés par MYSENSORS ou la communication ETHERNET

  // !! choisir impérativement synchroOutPin et pulseTriacPin parmi D4, D5, D6, D7 (port D) !!

//                   **************************************************
//                   **********          N  O  T  E           *********
//                   **************************************************

  // *** Fonction d'autotrigger de la détection du passage par zéro
  // *** en reliant physiquement synchroACPin IN et synchroOutPin OUT
  // *** PAR DEFAUT : RELIER ELECTRIQUEMENT D3 ET D4


// ***********************************************************************************
// ***************   Définition de macros de manipulation des pins   *****************
// ***************   pour la modification rapide des états           *****************
// ***********************************************************************************

// *** Définitions valables pour le PORTD, pins possibles :
// *** 0 = PIN D0 à 7 = PIN D7
// *** Cela concerne les pins pulseTriacPin et synchroOutPin
// *** Utilisation dans les interruptions pour réduire le temps de traitement

#define TRIAC_ON               PORTD |=  ( 1 << pulseTriacPin )
#define TRIAC_OFF              PORTD &= ~( 1 << pulseTriacPin )
#define SYNC_ON                PORTD |=  ( 1 << synchroOutPin )
#define SYNC_OFF               PORTD &= ~( 1 << synchroOutPin )


// ***********************************************************************************
// ************************ Declaration des variables globales ***********************
// ***********************************************************************************

// ************* Définition du calibrage du système (valeurs par défaut)
// NOTE : Ces valeurs seront remplacées automatiquement
// par les valeurs lues en EEPROM si celles-ci sont valides

float V_CALIB     =     0.800;        // Valeur de calibration de la tension du secteur lue (Volt par bit)
                                      // 0.823 = valeur par défaut pour Vcc = 5 V
float P_CALIB     =     0.111;        // Valeur de calibration de la puissance (VA par bit)
                                      // Implicitement I_CALIB = P_CALIB / V _CALIB
int   PHASE_CALIB =    13;            // Valeur de correction de la phase (retard) de l'acquisition de la tension
                                      // Entre 0 et 32 :
                                          // 16 = pas de correction
                                          // 0  = application d'un retard = temps de conversion ADC
                                          // 32 = application d'une avance = temps de conversion ADC
int   P_OFFSET     =    0;            // Correction d'offset de la lecture de Pactive en Watt
int   P_RESISTANCE = 1500;            // Valeur en Watt de la résistance contrôlée

// ************* Définition des paramètres de régulation du routeur de puissance (valeurs par défaut)
// NOTE : Ces valeurs seront remplacées automatiquement
// par les valeurs lues en EEPROM si celles-ci sont valides

int  P_MARGIN      =   15;            // Cible de puissance importée en Watt
int  GAIN_P        =   10;            // Gain proportionnel du correcteur
                                      // Permet de gérer les transitoires
int  GAIN_I        =   60;            // Gain intégral du correcteur
byte E_RESERVE     =    5;            // Réserve d'énergie en Joule avant régulation

// ***********************************************************************************************************************
// energyToDelay [ ] = Tableau du retard de déclenchement du SSR/TRIAC en fonction du routage de puissance désiré.
// 256 valeurs codées pour envoyer linéairement 0 (0) à 100 % (255) de puissance vers la résistance.
// Délais par pas de 64 us (= pas de comptage de CNT1) calculés pour une tension secteur 50 Hz.
// Les valeurs de DELAY_MIN (8) et de DELAY_MAX (140) sont fixées :
// Soit pour la fiabilité du déclenchement, soit parce que l'energie routée sera trop petite.
// La pente de la montée de la tension secteur à l'origine est de 72 V / ms, une demi-alternance fait 10 ms.
// Pour DELAY_MIN = 8 par pas de 64 us, soit 512 us, le secteur a atteint 36 V, le triac peut se déclencher correctement.
// On ne déclenche plus au delà de DELAY_MAX = 140 par pas de 64 us, soit 9 ms, pour la fiabilité du déclenchement
// DELAY_MAX doit être inférieur à PULSE_END qui correspond à l'instant où on arrêtera le pulse de déclenchement SSR/TRIAC
// ***********************************************************************************************************************

byte energyToDelay [ ] = {
    144, 143, 141, 140, 139, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 128,
    127, 127, 126, 125, 124, 123, 123, 122, 122, 121, 121, 120, 119, 119, 118, 118,
    117, 117, 116, 116, 115, 115, 114, 114, 114, 113, 113, 112, 112, 112, 111, 111,
    110, 110, 109, 109, 109, 108, 108, 108, 107, 107, 107, 106, 106, 106, 105, 105,
    104, 104, 104, 103, 103, 103, 102, 102, 102, 101, 101, 101, 100, 100, 100,  99,
     99,  99,  99,  98,  98,  98,  97,  97,  97,  96,  96,  95,  95,  95,  95,  94,
     94,  94,  93,  93,  93,  93,  92,  92,  91,  91,  91,  91,  90,  90,  90,  89,
     89,  89,  88,  88,  88,  87,  87,  87,  86,  86,  86,  85,  85,  84,  84,  84, 
     84,  83,  83,  83,  82,  82,  82,  82,  81,  81,  80,  80,  80,  80,  79,  79, 
     79,  78,  78,  78,  77,  77,  77,  76,  76,  76,  75,  75,  74,  74,  74,  73, 
     73,  73,  72,  72,  72,  71,  71,  71,  70,  70,  70,  69,  69,  69,  69,  68, 
     68,  67,  67,  67,  66,  66,  65,  65,  65,  64,  64,  63,  63,  63,  62,  62, 
     62,  61,  61,  61,  60,  60,  60,  59,  59,  58,  58,  58,  57,  57,  56,  56, 
     56,  55,  55,  54,  54,  53,  53,  52,  52,  52,  51,  51,  50,  50,  50,  49, 
     49,  48,  48,  47,  46,  45,  44,  44,  43,  43,  42,  42,  41,  41,  40,  39, 
     39,  38,  37,  36,  35,  33,  32,  31,  27,  25,  20,  15,  12,  10,   9,   8
};

#define PULSE_END              148    // Instant d'arrêt du pulse triac après le passage à 0
                                      // par pas de 64 us (9.5 ms = 148)


// ************* Définition des paramètres de délestage secondaire de puissance (DIV2)
// ************* Fonctionnement pour un relais en tout ou rien
// NOTE : Ces valeurs seront remplacées automatiquement
// par les valeurs lues en EEPROM si celles-ci sont valides

int  P_DIV2_ACTIVE       =    1000;        // Valeur de puissance routée en Watt qui déclenche le relais de délestage
int  P_DIV2_IDLE         =       0;        // Puissance active importée en Watt qui désactive le relais de délestage
byte T_DIV2_ON           =       5;        // Durée minimale d'activation du délestage en minutes
byte T_DIV2_OFF          =       5;        // Durée minimale d'arrêt du délestage en minutes
byte T_DIV2_TC           =       1;        // Constante de temps de moyennage des puissance routées et active
// NOTE : Il faut une condition d'hystérésis pour un bon fonctionnement :
// P_DIV2_ACTIVE + P_DIV2_IDLE > à la puissance de la charge de délestage secondaire


// ************* Variables globales pour le fonctionnement du régulateur
// ************* Ces variables permettent de communiquer les informations entre les routines d'interruption

volatile int     biasOffset    = 511;       // pour réguler le point milieu des acquisitions ADC, 
                                            // attendu autour de 511
volatile long    periodP       =   0;       // Samples de puissance accumulés
                                            // sur un demi-cycle secteur (période de la puissance)
#define          NCSTART           5
volatile byte    coldStart     =   NCSTART; // Indicateur de passage à 0 après le démarrage
                                            // Atteint la valeur 0 une fois le warm-up terminé
                                            // Attente de NCSTART passage à 0 avant de démarrer la régulation

// ************* Variables globales utilisées pour les calcul des statistiques de fonctionnement
// ************* Ces variables permettent de communiquer les informations entre les routines d'interruption

volatile long          sumP           = 0;
volatile unsigned long PVRClock       = 0;  // horloge interne par pas de 20 ms
volatile unsigned long sumV           = 0;
volatile unsigned long sumI           = 0;
volatile unsigned long sumVsqr        = 0;
volatile unsigned long sumIsqr        = 0;
volatile unsigned int  routed_power   = 0;
volatile unsigned int  samples        = 0;
volatile byte          error_status   = 0;
      // Signification des bits du byte error_status
      // bits 0..3 : informations
        // bit 0 (1)   : Routage en cours
        // bit 1 (2)   : Routage à 100 %
        // bit 2 (4)   : Relais secondaire de délestage activé
        // bit 3 (8)   : Exportation de puissance
      // bits 4..7 : erreurs               
        // bit 4 (16)  : Anomalie signaux analogiques : ADC I/V overflow, biasOffset
        // bit 5 (32)  : Anomalie taux d'acquisition
        // bit 6 (64)  : Anomalie furtive Détection passage à 0 (bruit sur le signal)
        // bit 7 (128) : Anomalie majeure Détection passage à 0 (sur 2 secondes de comptage)


// ************* Variables utilisées pour le transfert des statistiques de fonctionnement
// ************* des routines d'interruption vers le traitement dans la LOOP tous les NB_CYCLES

volatile long          stats_sumP         = 0;   // Somme des échantillons de puissance
volatile unsigned long stats_sumVsqr      = 0;   // Somme des échantillons de tension au carré
volatile unsigned long stats_sumIsqr      = 0;   // Somme des échantillons de courant au carré
volatile long          stats_sumV         = 0;   // Somme des échantillons de tension
volatile long          stats_sumI         = 0;   // Somme des échantillons de courant
volatile unsigned int  stats_routed_power = 0;   // Evaluation de la puissance routée vers la charge
volatile unsigned int  stats_samples      = 0;   // Nombre d'échantillons total
volatile byte          stats_error_status = 0;
volatile int           stats_biasOffset   = 0;   // Valeur de la correction d'offset de lecture ADC
volatile byte          stats_ready_flag   = 0;      
                       // 0 = Données traitées, en attente de nouvelles données
                       // 1 = Nouvelles données disponibles
                       // 9 = Données statistiques en cours de transfert

// ************* Variables globales utilisées pour les statistiques de fonctionnement et d'information
// ************* Ces variables n'ont pas d'utilité directe pour la régulation du PV routeur
// ************* Elles ne sont pas utilisées dans les interruptions

float                  VCC_1BIT         = 0.0049;   // valeur du bit de l'ADC sous Vcc = 5 V
float                  indexKWhRouted   = 0;        // compteur d'énergie
float                  indexKWhImported = 0;        // compteur d'énergie
float                  indexKWhExported = 0;        // compteur d'énergie

float                  Prouted          = 0;        // puissance routée en Watts
float                  Vrms             = 0;        // tension rms en V
float                  Irms             = 0;        // courant rms en A
float                  Papp             = 0;        // puissance apparente en VA
float                  Pact             = 0;        // puissance active en Watts
float                  cos_phi          = 0;        // cosinus phi

byte                   secondsOnline    = 0;        // horloge interne
byte                   minutesOnline    = 0;        // basée sur la période secteur
byte                   hoursOnline      = 0;        // et mise à jour à chaque fois que les données
byte                   daysOnline       = 0;        // statistiques sont disponibles

byte                   ledBlink         = 0;        // séquenceur de clignotement pour les LEDs, période T
                       // bit 0 (1)   : T = 40 ms
                       // bit 1 (2)   : T = 80 ms
                       // bit 2 (4)   : T = 160 ms
                       // bit 3 (8)   : T = 320 ms
                       // bit 4 (16)  : T = 640 ms
                       // bit 5 (32)  : T = 1280 ms
                       // bit 6 (64)  : T = 2560 ms
                       // bit 7 (128) : T = 5120 ms                      

// ***********************************************************************************
// ********  Définitions pour l'accès à la configuration stockée en EEPROM  **********
// ***********************************************************************************

#include <EEPROM.h>
#define                   PVR_EEPROM_START            550   // Adresse de base des données PVR
#define                   PVR_EEPROM_SIZE              33   // Taille des données PVR dans EEPROM
#define                   PVR_EEPROM_INDEX_ADR       1000   // Adresse de base des compteurs de kWh
const unsigned long       DATAEEPROM_MAGIC   =  370483670;  // UID de signature de la configuration
const byte                DATAEEPROM_VERSION =          1;  // Version du type de configuration

//                   **************************************************
//                   **********   A  T  T  E  N  T  I  O  N   *********
//                   **************************************************

  //  *** MYSENSORS utilise la partie basse des adresses de l'EEPROM pour son fonctionnement
  //  *** C'est pourquoi les adresses EEPROM sont > à 500

struct dataEeprom {                         // Structure des données pour le stockage en EEPROM
  unsigned long         magic;              // Magic Number
  byte                  struct_version;     // Structure version
  float                 v_calib;
  float                 p_calib;
  int                   phase_calib;
  int                   p_offset;
  unsigned int          p_resistance;
  int                   p_margin;
  int                   gain_p;
  int                   gain_i;
  byte                  e_reserve;  
  int                   p_div2_active;
  int                   p_div2_idle;
  byte                  t_div2_on;  
  byte                  t_div2_off;  
  byte                  t_div2_tc;  
  // fin des données eeprom V1
  // taille totale : 33 bytes (byte = 1 byte, int = 2 bytes, long = float = 4 bytes)
};

// ***********************************************************************************
// ********* Définitions pour la modification de la configuration en EEPROM **********
// ***********************************************************************************

#define NB_PARAM            14          // Nombre de paramètres dans la configuration, 14 en EEPROM V1

struct paramInConfig {                  // Structure pour la manipulation des données de configuration
  byte dataType;                        // 0 : int, 1 : float, 2 : array of 256 bytes, 4 : byte, 5 : long
  int  minValue;                        // valeur minimale que peut prendre le paramètre (-16383)
  int  maxValue;                        // valeur maximale que peut prendre le paramètre (16384)
  bool advancedParameter;               // signale un paramètre de configuration avancée
  void *adr;                            // pointeur vers le paramètre
};

const paramInConfig pvrParamConfig [ ] = {
// Tableau utilisé pour la manipulation des données de configuration
// dataType  min      max  advanced   adr
  { 1,        0,       5,   false,  &V_CALIB },        // V_CALIB
  { 1,        0,      25,   false,  &P_CALIB },        // P_CALIB
  { 0,      -16,      48,   true,   &PHASE_CALIB },    // PHASE_CALIB
  { 0,     -100,     100,   true,   &P_OFFSET },       // P_OFFSET
  { 0,      100,   10000,   false,  &P_RESISTANCE },   // P_RESISTANCE
  { 0,     -200,    2000,   false,  &P_MARGIN },       // P_MARGIN
  { 0,        0,    1000,   true,   &GAIN_P },         // GAIN_P
  { 0,        0,    1000,   true,   &GAIN_I },         // GAIN_I
  { 4,        0,     100,   true,   &E_RESERVE },      // E_RESERVE
  { 0,        0,    9999,   false,  &P_DIV2_ACTIVE },  // P_DIV2_ACTIVE
  { 0,        0,    9999,   false,  &P_DIV2_IDLE },    // P_DIV2_IDLE
  { 4,        0,     240,   false,  &T_DIV2_ON },      // T_DIV2_ON
  { 4,        0,     240,   false,  &T_DIV2_OFF },     // T_DIV2_OFF
  { 4,        0,      60,   false,  &T_DIV2_TC }       // T_DIV2_TC
};

const char string_0 []   PROGMEM = "Facteur de calibrage de la tension\t\t";        // V_CALIB
const char string_1 []   PROGMEM = "Facteur de calibrage de la puissance\t\t";      // P_CALIB
const char string_2 []   PROGMEM = "Facteur de calibrage de la phase\t\t";          // PHASE_CALIB
const char string_3 []   PROGMEM = "Décalage de puissance active (W)\t\t";          // P_OFFSET
const char string_4 []   PROGMEM = "Puissance de la résistance commandée (W)\t";    // P_RESISTANCE
const char string_5 []   PROGMEM = "Consigne de régulation (W)\t\t\t";              // P_MARGIN
const char string_6 []   PROGMEM = "Gain proportionnel de régulation\t\t";          // GAIN_P
const char string_7 []   PROGMEM = "Gain intégral de régulation\t\t\t";             // GAIN_I
const char string_8 []   PROGMEM = "Tolérance de régulation (J)\t\t\t";             // E_RESERVE
const char string_9 []   PROGMEM = "Excédent de production pour relais ON (W)\t";   // P_DIV2_ACTIVE
const char string_10 []  PROGMEM = "Importation minimale pour relais OFF (W)\t";    // P_DIV2_IDLE
const char string_11 []  PROGMEM = "Relais : durée minimale ON (min)\t\t";          // T_DIV2_ON
const char string_12 []  PROGMEM = "Relais : durée minimale OFF (min)\t\t";         // T_DIV2_OFF
const char string_13 []  PROGMEM = "Relais : constante de lissage (min)\t\t";       // T_DIV2_TC

const char *const pvrParamName [ ] PROGMEM = {
  string_0, string_1, string_2, string_3, string_4,
  string_5, string_6, string_7, string_8, string_9,
  string_10, string_11, string_12, string_13
};


// ***********************************************************************************
// ****************** Définitions pour la communication OLED_128X64    ***************
// ***********************************************************************************

  // *** OLED_128X64 utilise les pins A4 et A5
  // *** pour la connexion I2C de l'écran

#if defined (OLED_128X64)

  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiAvrI2c.h"
  
  #define I2C_ADDRESS 0x3C                    // adresse I2C de l'écran oled
  #define OLED_128X64_REFRESH_PERIOD  6       // période de raffraichissement des données à l'écran

  SSD1306AsciiAvrI2c oled;

#endif

// ***********************************************************************************
// ****************** Définitions pour la communication MYSENSORS      ***************
// ****************** Voir : www.mysensors.org                         ***************
// ***********************************************************************************

  // *** MYSENSORS utilise les pins D2 D9 D10 D11 D12 et D13
  // *** pour la connexion et l'utilisation de la radio NRF24

#if defined (MYSENSORS_COM)

  //#define MY_DEBUG                          // Debug Mysensors sur port série
  #define MY_NODE_ID                 30       // Adresse du noeud de capteurs MYSENSORS
  #define MY_RADIO_RF24                       // Type de module radio
  #define MY_BAUD_RATE               SERIAL_BAUD
  #define MY_TRANSPORT_WAIT_READY_MS 15000

  #include <MySensors.h>

  // Définition de 2 capteurs dans le noeud
  #define CHILD_ID_POWER             0        // capteur 0 = Power meter
  #define CHILD_ID_MULTIMETER        1        // capteur 1 = Multimètre
  
  #define MYSENSORS_TRANSMIT_PERIOD 20        // Période de transmission des données en secondes
                                              // valeurs possibles pour une transmission régulière :
                                              // 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60
  
  MyMessage msg_pwr       ( CHILD_ID_POWER, V_WATT );           // puissance active
  MyMessage msg_pva       ( CHILD_ID_POWER, V_VA );             // puissance apparente
  MyMessage msg_cosphi    ( CHILD_ID_POWER, V_POWER_FACTOR );   // cosinus phi
  MyMessage msg_kwh       ( CHILD_ID_POWER, V_KWH );            // énergie routée
  MyMessage msg_pimp      ( CHILD_ID_POWER, V_VAR1 );           // puissance importée
  MyMessage msg_pexp      ( CHILD_ID_POWER, V_VAR2 );           // puissance exportée
  MyMessage msg_prt       ( CHILD_ID_POWER, V_VAR3 );           // puissance routée
  MyMessage msg_error     ( CHILD_ID_POWER, V_VAR4 );           // byte d'erreur / statut
  MyMessage msg_vrms      ( CHILD_ID_MULTIMETER, V_VOLTAGE );   // tension
  MyMessage msg_irms      ( CHILD_ID_MULTIMETER, V_CURRENT );   // courant

#endif


// ***********************************************************************************
// ****************** Définitions pour la communication ETHERNET       ***************
// ****************** Nécessite un shield ENC28J60                     ***************
// ***********************************************************************************

  // *** Note : La connexion ethernet utilise les pins D10 D11 D12 et D13
  // *** pour la communication Arduino <-> shield ENC28J60

#if defined (ETHERNET_28J60)

  #include "etherShield.h"
  #include "ETHER_28J60.h"
  // *** Les bibliothèques etherShield et ETHER_28J60 fournies
  // *** doivent installées dans l'IDE Arduino.
  // *** L'installation se fait manuellement.

  byte ethMac [6] = { 0x10, 0x12, 0x16, 0xB0, 0x63, 0x24 };
    // La mac address doit être unique sur le réseau local : 10:12:16:B0:63:24
  byte ethIp [4] = { 192, 168, 1, 250 };
    // Adresse IP correspondant à une adresse libre sur le réseau local : 192.168.1.250
  unsigned int ethPort = 80;
    // Port IP pour l'accès aux requêtes HTTP : 80
    
  ETHER_28J60 ethernet;              
  
#endif

// ***********************************************************************************
// ************************   FIN DES DEFINITIONS GENERALES    ***********************
// ***********************************************************************************


// ***********************************************************************************
// *******************   DEFINITIONS DES FONCTIONS ET PROCEDURES   *******************
// ***********************************************************************************
 
/////////////////////////////////////////////////////////
// presentation                                        //
// Routine d'initialisation  MYSENSORS                 //
// Voir : www.mysensors.org                            //
/////////////////////////////////////////////////////////

#if defined (MYSENSORS_COM)

void presentation ( ) {

  // Envoi des informations de version
  sendSketchInfo ( "EcoPV", VERSION );
  // Enregistrement des capteurs du noeud de capteur
  present ( CHILD_ID_POWER, S_POWER );
  present ( CHILD_ID_MULTIMETER, S_MULTIMETER );
}

#endif



/////////////////////////////////////////////////////////
// setup                                               //
// Routine d'initialisation générale                   //
/////////////////////////////////////////////////////////

void setup ( ) {

  // Initialisation des pins
  pinMode      ( synchroACPin,  INPUT  );   // Entrée de synchronisation secteur
  pinMode      ( pulseTriacPin, OUTPUT );   // Commande Triac
  pinMode      ( synchroOutPin, OUTPUT );   // Détection passage par zéro émis par l'ADC
  pinMode      ( ledPinStatus,  OUTPUT );   // LED Statut
  pinMode      ( ledPinRouting, OUTPUT );   // LED Routage puissance
  pinMode      ( relayPin,      OUTPUT );   // Commande relais de délestage tout ou rien
  
  digitalWrite ( pulseTriacPin, OFF    );
  digitalWrite ( synchroOutPin, OFF    );
  digitalWrite ( ledPinStatus,  ON     );
  digitalWrite ( ledPinRouting, ON     );
  digitalWrite ( relayPin,      OFF    );

  // Le délai suivant de 1500 ms est important lors du reboot après programmation
  // pour éviter des problèmes d'écriture en EEPROM
  // + Clignotement des leds (power on)
  delay ( 500 );
  digitalWrite ( ledPinStatus,  OFF    );
  digitalWrite ( ledPinRouting, OFF    );
  delay ( 500 );
  digitalWrite ( ledPinStatus,  ON     );
  digitalWrite ( ledPinRouting, ON     );
  delay ( 500 );
  digitalWrite ( ledPinStatus,  OFF    );
  digitalWrite ( ledPinRouting, OFF    );
  // Fin du délai de 1500 ms

  // Activation de l'écran oled si défini
#if defined (OLED_128X64)
  oled.begin( &Adafruit128x64, I2C_ADDRESS );
  oled.setFont ( System5x7 );
  oled.clear ( );
  oled.set2X ( );
  oled.print ( F("EcoPV V") );
  oled.println ( F(VERSION) );
  oled.println ( );
  oled.println ( F("Starting...") );
#endif

  // Activation de la connexion ethernet si définie
#if defined (ETHERNET_28J60)
  ethernet.setup ( ethMac, ethIp, ethPort );
#endif

  // Chargement de la configuration EEPROM si existante
  // Sinon écriture des valeurs par défaut en EEPROM
  if ( eeConfigRead ( ) == false ) eeConfigWrite ( );
  
  // Chargement des l'index d'énergie kWh
  indexRead ( );

  // Lecture de la tension d'alimentation de l'arduino
  analogReadReference ( );                       // Première lecture à vide
  VCC_1BIT = ( 1.1 / analogReadReference ( ) );  // Calcul de la valeur en V d'un bit ADC

  // Activation de la sortie sur liaison série & affichage du message de démarrage
  Serial.begin (SERIAL_BAUD);
  while ( !Serial ) { };
  Serial.setTimeout ( SERIALTIMEOUT );
  clearScreen ( );
  // Affichage de la version
  versionPrint ( );

  // Affichage des options de compilation activées
  optionPrint ( );

  // Affichage de la configuration si mode PV_STATS seul
#if ( defined (PV_STATS) ) && ( !defined (PV_MOD_CONFIG) )
  configPrint ( );
  Serial.println ( );
#endif

  // Accès à la modification de la configuration si autorisé
#if defined (PV_MOD_CONFIG)
  Serial.print ( F("Appuyez sur entrée pour entrer dans le set-up") );
  for ( int i = 0; i <= 4; i++ ) {
    delay ( 800 );
    Serial.print ( F(".") );
  }
  Serial.println ( );
  if ( Serial.available ( ) > 0 ) configuration ( );
  clearSerialInputCache ( );
#endif

  // Séquence de démarrage du PV routeur
  Serial.println ( F("\nInitialisation du PV routeur...") );
  startPVR ( );
  if ( coldStart == 0 ) Serial.println ( F("\nPV routeur actif.\n") );

#if defined (OLED_128X64)
  oled.clear ( );
  oled.set2X ( );
  oled.print ( F("EcoPV V") );
  oled.println ( F(VERSION) );
  oled.println ( );
  oled.println ( F("Running !") );
#endif

}


///////////////////////////////////////////////////////////////////
// loop                                                          //
// Loop routine exécutée en boucle                               //
///////////////////////////////////////////////////////////////////

void loop ( ) {

  static unsigned long refTime        = millis ( );
  static float         routedEnergy   = 0;
  static float         exportedEnergy = 0;
  static float         importedEnergy = 0;

  static float         inv_NB_CYCLES  = 1 / float ( NB_CYCLES );
  static float         inv_255        = 1 / float ( 255 );
  float                inv_stats_samples = 1 / float ( int ( SAMP_PER_CYCLE ) * int ( NB_CYCLES ) );

  float                Filter_param = 1 / float ( 1 + ( int ( T_DIV2_TC ) * 60 ) );
  static float         Pact_filtered = 0;
  static float         Prouted_filtered = 0;
  static unsigned int  Div2_On_cnt = 0;
  static unsigned int  Div2_Off_cnt = 0;
 
  unsigned int         OCR1A_tmp;
  byte                 OCR1A_byte;
  static byte          OCR1A_min = 255;
  static byte          OCR1A_max = 0;
  static unsigned long OCR1A_avg = 0;
  static unsigned long OCR1A_cnt = 0;

  
  // *** Vérification perte longue de synchronisation secteur
  if ( ( millis ( ) - refTime ) > 2010 ) {
    // Absence de remontée d'informations depuis plus de 2 secondes = absence de synchronisation secteur
    refTime = millis ( );
    noInterrupts ( );
    error_status |= B10000000;            // On reporte l'erreur majeure au système de régulation
    stats_error_status |= error_status;   // On transfère tous les bits de statut et d'erreur
    interrupts ( );
  }

  // *** Statistiques du délai de déclenchement du TRIAC / SSR
  // Note : information approximative basée sur la lecture du registre OCR1A
  noInterrupts ( );
  OCR1A_tmp = OCR1A;
  interrupts ( );
  if ( OCR1A_tmp < 256 ) {
    OCR1A_byte = byte ( OCR1A_tmp );
    OCR1A_avg += OCR1A_byte;
    OCR1A_cnt++;
    if ( OCR1A_byte > OCR1A_max ) OCR1A_max = OCR1A_byte;
    if ( OCR1A_byte < OCR1A_min ) OCR1A_min = OCR1A_byte;
  }
  
  // *** Traitement des informations statistiques lorsqu'elles sont disponibles
  // *** tous les NB_CYCLES sur flag, soit toutes les secondes pour NB_CYCLES = 50 @ 50 Hz
  if ( stats_ready_flag == 1 ) {          // Les données statistiques sont disponibles après NB_CYCLES
                                          // Si le flag est différent de 0, elles ne seront pas modifiées
    refTime = millis ( );                 // Mise à jour du temps de passage dans la boucle

    // *** Vérification du routage                                        ***
    if ( stats_routed_power > 0 )
      stats_error_status |= B00000001;
    else
      stats_error_status &= B11111110;

    // *** Vérification du routage pleine puissance                       ***
    if ( ( stats_routed_power / ( 2 * NB_CYCLES ) ) >= 255 )
      stats_error_status |= B00000010;
    else
      stats_error_status &= B11111101;
      
    // *** Vérification du nombre de samples à +/- 2 %                    ***
    if ( ( stats_samples >= int ( NB_CYCLES + 1 ) * SAMP_PER_CYCLE )
         || ( stats_samples <= int ( NB_CYCLES - 1 ) * SAMP_PER_CYCLE ) )
      stats_error_status |= B00100000;

    // *** Calcul des valeurs statistiques en unités réelles              ***
    inv_stats_samples = 1 / float ( stats_samples );
    Vrms    = V_CALIB * sqrt ( stats_sumVsqr * inv_stats_samples );
    Papp    = P_CALIB * sqrt ( ( stats_sumVsqr * inv_stats_samples )
                             * ( stats_sumIsqr * inv_stats_samples ) );
    Pact    = - ( P_CALIB * stats_sumP * inv_stats_samples + P_OFFSET );
      // le signe - sur le calcul de Pact permet d'avoir Pact < 0 en exportation
    Prouted = float ( P_RESISTANCE ) * float ( stats_routed_power )
              * 0.5 * inv_NB_CYCLES * inv_255;
    Irms    = Papp / Vrms;
    cos_phi = Pact / Papp;

    // *** Vérification de l'exportation d'énergie                        ***
    if ( Prouted < 0 )
      stats_error_status |= B00001000;
    else
      stats_error_status &= B11110111;

    // *** Calcul des valeurs filtrées de Pact et Prouted                 ***
    // *** Usage : déclenchement du relais secondaire de délestage        ***
    Filter_param = 1 / float ( 1 + ( int ( T_DIV2_TC ) * 60 ) );
    Pact_filtered = Pact_filtered + Filter_param * ( Pact - Pact_filtered );
    Prouted_filtered = Prouted_filtered + Filter_param * ( Prouted - Prouted_filtered );

    // *** Déclenchement et gestion du relais secondaire de délestage     ***
    if ( ( Prouted_filtered >= float ( P_DIV2_ACTIVE ) ) && ( Div2_Off_cnt == 0 )
      && ( digitalRead ( relayPin ) == OFF ) ) {
      digitalWrite ( relayPin, ON );    // Activation du relais de délestage
      Div2_On_cnt = 60 * T_DIV2_ON;     // Initialisation de la durée de fonctionnement minimale en sevondes
    }
    else if ( Div2_On_cnt > 0 ) {
      Div2_On_cnt --;                     // décrément d'une seconde
    }
    else if ( ( Pact_filtered >= float ( P_DIV2_IDLE ) ) && ( digitalRead ( relayPin ) == ON ) ) { 
      digitalWrite ( relayPin, OFF );     // Arrêt du délestage
      Div2_Off_cnt = 60 * T_DIV2_OFF;     // Initialisation de la durée d'arrêt minimale en secondes
    }
    else if ( Div2_Off_cnt > 0 ) {
      Div2_Off_cnt --;                    // décrément d'une seconde
    }
    
    // *** Vérification de l'état du relais de délestage                  ***
    if ( digitalRead ( relayPin ) == ON )
      stats_error_status |= B00000100;
    else
      stats_error_status &= B11111011;

    // *** Accumulation des énergies routée, importée, exportée           ***
    // *** Les calculs sont faits toutes les secondes                     ***
    // *** La puissance x 1s = énergie en Joule                           ***
    routedEnergy += Prouted;
    if ( Pact < 0 )     exportedEnergy -= Pact;
    else                importedEnergy += Pact;
    if ( routedEnergy >= 3600 ) {     // On a accumulé 3600 J = 1 Wh)
      routedEnergy -= 3600;           // On suppose qu'on ne peut pas router plus de 3600 Watts
      indexKWhRouted += 0.001;
    }
    if ( exportedEnergy >= 3600 ) {   // On a accumulé 3600 J = 1 Wh)
      exportedEnergy -= 3600;         // On suppose qu'on ne peut pas exporter plus de 3600 Watts
      indexKWhExported += 0.001;
    }
    if ( importedEnergy >= 18000 ) {   // On a accumulé 18000 J = 5 Wh)
      importedEnergy -= 18000;         // On suppose qu'on ne peut pas importer plus de 18000 Watts
      indexKWhImported += 0.005;
    }

    // *** Mise à jour du temps de fonctionnement UpTime                  ***
    upTime ( );

    // *** Appel du scheduler                                             ***
    // *** Gestion des actions régulières et tâches planifiées            ***
    PVRScheduler ( );
    
    // *** Calcul des statistiques du déclenchement du TRIAC              ***
    OCR1A_avg /= OCR1A_cnt;

    // *** Affichage des donnéees statistiques si mode PV_STATS           ***
#if defined (PV_STATS)
    clearScreen ( );
    Serial.println ( F("\n********** Statistiques de fonctionnement **********\n") );
    Serial.print ( F("Up time\t: ") );
    Serial.print ( daysOnline );
    Serial.print ( F(" jours ") );
    Serial.print ( hoursOnline );
    Serial.print ( F(" h ") );
    Serial.print ( minutesOnline );
    Serial.print ( F(" min ") );
    Serial.print ( secondsOnline );
    Serial.println ( F(" s") );
    Serial.print ( F("Vrms\t: ") );
    Serial.print ( Vrms, 1 );
    Serial.println ( F(" V") );
    Serial.print ( F("Irms\t: ") );
    Serial.print ( Irms, 3 );
    Serial.println ( F(" A") );
    Serial.print ( F("Cos phi\t: ") );
    Serial.println ( cos_phi, 3 );
    Serial.print ( F("Sin phi\t: ") );
    Serial.println ( sqrt ( ( 1 - cos_phi * cos_phi ) ), 3 );
    Serial.print ( F("Pappar\t: ") );
    Serial.print ( Papp, 0 );
    Serial.println ( F(" VA") );
    Serial.print ( F("Pactive\t: ") );
    Serial.print ( Pact, 1 );
    Serial.print ( F(" W - Valeur filtrée : ") );
    Serial.print ( Pact_filtered , 0 );
    Serial.print ( F(" W ") );
    if ( Pact < 0 )
      Serial.println ( F(" (exportation)") );
    else
      Serial.println ( F(" (importation)") );
    Serial.print ( F("Proutée\t: ") );
    Serial.print ( Prouted , 0 );
    Serial.print ( F(" W - Valeur filtrée : ") );
    Serial.print ( Prouted_filtered , 0 );
    Serial.println ( F(" W") );
    Serial.print ( F("Index 1\t: ") );
    Serial.print ( indexKWhRouted, 3 );
    Serial.println ( F(" kWh routés") );
    Serial.print ( F("Index 2\t: ") );
    Serial.print ( indexKWhExported, 3 );
    Serial.println ( F(" kWh exportés") );
    Serial.print ( F("Index 3\t: ") );
    Serial.print ( indexKWhImported, 3 );
    Serial.println ( F(" kWh importés") );
    Serial.print ( F("Vbias\t: ") );
    Serial.print ( float ( VCC_1BIT * stats_biasOffset ), 3 );
    Serial.println ( F(" V") );
    Serial.print ( F("Nb éch.\t: ") );
    Serial.println ( stats_samples );
    Serial.print ( F("SSR Dly\t: ") );
    if ( OCR1A_min < 255 ) {
      Serial.print ( float ( OCR1A_min * 0.064 ), 2 );
      Serial.print ( F(" / ") );
      Serial.print ( float ( OCR1A_avg * 0.064 ), 2 );
      Serial.print ( F(" / ") );
      Serial.print ( float ( OCR1A_max * 0.064 ), 2 );
      Serial.println ( F(" (min / avg / max [ms])") );
    }
    else {
      Serial.println ( F("Non disponible - pas de routage") );
    }
    Serial.print ( F("Status\t: ") );
    Serial.println ( stats_error_status, BIN );
    if ( digitalRead ( relayPin ) == ON ) Serial.println ( F("Relais secondaire de délestage activé") );
    else Serial.println ( F("Relais secondaire de délestage désactivé") );
    Serial.println ( );
#endif

    // *** Initialisation des statistique OCR1A                           ***
    OCR1A_avg = 0;
    OCR1A_max = 0;
    OCR1A_min = 255;
    OCR1A_cnt = 0;
    
    // *** Reset du Flag pour indiquer que les données ont été traitées   ***
    stats_ready_flag = 0;

    // *** Affichage de l'invite de configuration si mode PV_MOD_CONFIG   ***
#if defined (PV_MOD_CONFIG)
    Serial.println ( F("Appuyez sur entrée pour accéder à la configuration") );
    // Appel au menu de configuration
    if ( Serial.available ( ) > 0 ) {
      delay ( 200 );
      configuration ( );
    }
    clearSerialInputCache ( );
    refTime = millis ( );              
#endif

  }
  // *** Fin du Traitement des informations statistiques                  ***

  // *** La suite est exécutée à chaque passage dans loop                 ***

  // *** Mise à jour de l'état des LEDs de signalisation                  ***
  PVRLed ( );

  // *** Traitement des requêtes HTTP ETHERNET                            ***
#if defined (ETHERNET_28J60)               
  ethernetProcess ( );
#endif

  // *** Traitement de la perte longue de synchronisation, erreur majeure ***
  if ( stats_error_status >= B10000000 ) { 
#if defined (MYSENSORS_COM)                
    mySensorsTransmit ( );
#endif
    fatalError ( );                      
    refTime = millis ( );                
  };
}


///////////////////////////////////////////////////////////////////////
//////////////////// ROUTINES d'INTERRUPTIONS /////////////////////////
///////////////////////////////////////////////////////////////////////

// Quand une interruption est appelée, les autres sont désactivées
// mais gardées en mémoire.
// Elles seront appelées à la fin de l'exécution de l'interruption en cours
// dans un ordre de priorité : INT1, COUNT1, ADC.

///////////////////////////////////////////////////////////////////////
// zeroCrossingInterrupt                                             //
// Interrupt service routine de passage à zéro du secteur            //
// Temps mesuré de traitement de l'interruption : entre 36 et 40 us  //
///////////////////////////////////////////////////////////////////////

void zeroCrossingInterrupt ( void ) {

#define ERROR_BIT_SHIFT        6        // Valeur de décimation des données pour le calcul de la régulation PI
#define COMMAND_BIT_SHIFT     14        // Valeur de décimation pour la commande de puissance

  static bool           periodParity   = POSITIVE;
  static unsigned long  last_time;
  static byte           numberOfCycle  = 0;   
  static long           P_SETPOINT     = long ( float ( ( P_MARGIN + P_OFFSET ) )
                                              * float ( SAMP_PER_CYCLE ) * ( 0.5 / P_CALIB ) );
                        // Setpoint du régulateur : Valeur de P_MARGIN transféré dans le système d'unité du régulateur
                        // Prise en compte de l'offset de mesure P_OFFSET sur la puissance active
  static long           controlError        = P_SETPOINT >> ERROR_BIT_SHIFT;
  static long           lastControlError    = P_SETPOINT >> ERROR_BIT_SHIFT;
  static long           controlIntegral     = 0;
  static long           controlIntegralMinValue = - ( long ( ( long ( NB_CYCLES ) * long ( SAMP_PER_CYCLE ) * long ( E_RESERVE ) )
                                                  / P_CALIB ) >> ERROR_BIT_SHIFT );
  static long           controlCommand      = 0;
  unsigned long         present_time;

  present_time = micros ( );

  if ( coldStart > 0 ) {
    // Phase de Warm-up : initialisation jusque coldStart = 0;
    coldStart --;                               // on décrémente coldStart

    TCCR1B           = 0x00;                    
    TRIAC_OFF;                                  
    TCNT1            = 0x00;                    
    OCR1A            = 30000;                   // on charge à un délai inatteignable

    periodParity     = POSITIVE;                // signe arbitraire de l'alternance
    periodP          = 0;
    P_SETPOINT       = long ( float ( ( P_MARGIN + P_OFFSET ) )
                            * float ( SAMP_PER_CYCLE ) * ( 0.5 / P_CALIB ) );
    controlError     = P_SETPOINT >> ERROR_BIT_SHIFT;
    lastControlError = P_SETPOINT >> ERROR_BIT_SHIFT;
    controlIntegral  = 0;
    controlIntegralMinValue = - ( long ( ( long ( NB_CYCLES ) * long ( SAMP_PER_CYCLE ) * long ( E_RESERVE ) )
                              / P_CALIB ) >> ERROR_BIT_SHIFT );
    controlCommand   = 0;
    numberOfCycle    = 0;
    last_time        = present_time;

    // Initialisation pour les statistiques

    samples          = 0;
    sumVsqr          = 0;
    sumIsqr          = 0;
    sumP             = 0;
    routed_power     = 0;
    stats_ready_flag = 0;
    error_status     = 0;
  }

  else if ( ( present_time - last_time ) > 8000 ) {
    // *** PASSAGE PAR ZERO - ON A TERMINE UNE DEMI PERIODE ***
    // gestion de l'antirebond du passage à 0 en calculant de temps
    // entre 2 passages à 0 qui doivent être séparés de 8 ms au moins

    TCCR1B = 0x00;                        // arrêt du Timer par sécurité
    TRIAC_OFF;
    TCNT1  = 0x00;                        // on remet le compteur à 0 par sécurité
    TCCR1B = 0x05;                        // on démarre le Timer par pas de 64 us
    OCR1A  = 30000;                       // on charge à un délai inatteignable en attendant les calculs
    // ATTENTION, le Timer1 commence à compter ici !!

    // *** Calculs de fin de période (demi-cycle secteur)
    // *** Convention : si injection sur le réseau (PV excédentaire), periodP est positif

    controlError = ( periodP + P_SETPOINT ) >> ERROR_BIT_SHIFT;
    // calcul de l'erreur du régulateur
    // signe + lié à la définition de P_SETPOINT
    // P_SETPOINT prend en compte la correction de l'offset P_OFFSET de lecture de Pact 
    // réduction de résolution de ERROR_BIT_SHIFT bits
    // = division par 2 puissance ERROR_BIT_SHIFT
    // pour éviter de dépasser la capacité des long dans les calculs
    // et donner de la finesse au réglage du gain

    controlIntegral += controlError;
    // Note : l'erreur ne sera intégrée que si on est en régime linéaire de régulation
    // pour éviter le problème d'integral windup et de débordement de controlCommand
    // Le régime linéaire de régulation est observé sur la dernière commande SSR controlCommand
    // Voir le traitement fait plus bas

    // on calcule la commande à appliquer (correction PI)
    // note : lissage de l'erreur sur 2 périodes pour l'action proportionnelle pour corriger le bruit systématique
    controlCommand = long ( GAIN_I ) * controlIntegral + long ( GAIN_P ) * ( controlError + lastControlError );

    // application du gain fixe de normalisation : réduction de COMMAND_BIT_SHIFT bits
    controlCommand = controlCommand >> COMMAND_BIT_SHIFT;
      
    if ( controlCommand <= 0 ) {  // équilibre ou importation, donc pas de routage de puissance
      controlCommand = 0;
      TCCR1B = 0;                 // arrêt du Timer = inhibition du déclenchement du triac pour cette période
      TCNT1  = 0;                 // compteur à 0
      if ( controlIntegral <= controlIntegralMinValue ) {   // fonction anti integral windup
        controlIntegral = controlIntegralMinValue;
      }
    }
    
    else {  // controlCommand est strictement positif
      if ( controlCommand > 255 ) {   // Saturation de la commande en pleine puissance
        controlCommand = 255;         // Pleine puissance
        controlIntegral -= controlError;     // gel de l'accumulation de l'intégrale, fonction anti integral windup

      }
      // *** Régime linéaire de régulation : initialisation du comparateur de CNT1
      // *** pour le déclenchement du SSR/TRIAC géré par interruptions Timer1        
      OCR1A = energyToDelay [ byte ( controlCommand ) ];
    }

    // Calcul pour les statistiques
    routed_power += controlCommand;
    sumP += periodP;

    // Initialisation pour la période suivante
    periodP = 0;
    lastControlError = controlError;

    // changement de parité de la période (pour le demi-cycle suivant)
    periodParity = !periodParity;

    // opérations réalisées toutes les 2 périodes (à chaque cycle secteur)
    if ( periodParity ) {   // La demi-période suivante est positive

      // incrément du nombre de cycles
      numberOfCycle ++;

      // Opérations réalisées tous les NB_CYCLES cycles soit toutes les secondes pour NB_CYCLES = 50
      if ( numberOfCycle == NB_CYCLES ) {

        numberOfCycle = 0;

        if ( stats_ready_flag == 0 ) {        // La LOOP a traité les données précédentes
          // Transfert des données statistiques pour utilisation par la LOOP (Partie 1)
          stats_routed_power = routed_power;  // Evaluation de la puissance routée vers la charge
          routed_power = 0;
          stats_biasOffset = biasOffset;      // Dernière valeur de la correction d'offset de lecture ADC
          stats_error_status &= B00001111;    // RAZ des bits d'ERREUR 4..7
          stats_error_status |= error_status; // Transfert des bits d'ERREUR 4..7 uniquement
          error_status = 0;                   // Les bits signalant les erreurs sont remis à 0
                                              // à chaque traitement statistique

          stats_ready_flag = 9;               // Flag pour le prochain appel de l'interruption ADC
                                              // qui poursuivra le transfert des statistiques (Partie 2)

        }
        else {                                // La LOOP n'a pas (encore) traité les données précédentes :
                                              // Les données courantes sont perdues.
          routed_power = 0;
          error_status = 0;
          sumP = 0;
          sumVsqr = 0;
          sumIsqr = 0;
          sumV    = 0;
          sumI    = 0;
          samples = 0;
        }
      }
    }

    else {   // La demi-période suivante est négative
      // Incrément de l'horloge interne de temps de fonctionnement par pas de 20 ms
      PVRClock ++;
      // incrément du séquenceur pour le clignotement des leds
      ledBlink ++;
      
      // Détection des erreurs de biasOffset
      if ( abs ( biasOffset - 511 ) >= BIASOFFSET_TOL ) {
        error_status |= B00010000;
      }
    }

    last_time = present_time;
  }
  else error_status |= B01000000;  // Fausse détection d'un passage à 0, on signale l'évènement
}

//////////////////////////////////////////////////////////////////////
// ISR ( ADC_vect )                                                 //
// Interrupt service routine pour la conversion ADC de I et V       //
// Temps mesuré de traitement de l'interruption : 20 à 24 us        //
//////////////////////////////////////////////////////////////////////

ISR ( ADC_vect ) {

  // Note : pour des raisons de retard de lecture analogique I et V, on convertit d'abord I, puis V
  // La charge de calcul est répartie entre les 2 phases de conversion pour optimiser les temps d'interruption

  // caractéristique du filtrage de détermination de biasOffset
#define FILTERSHIFT           15  // constante de temps de 4s
#define FILTERROUNDING        0b100000000000000

  static byte   readFlagADC = 0;      
                // readFlagADC = 0 pour la conversion du courant
                // readFlagADC = 9 pour la conversion de la tension
                // Variable locale. Ne peut pas prendre d'autres valeurs que 0 ou 9
  static long   fBiasOffset = ( 511L << FILTERSHIFT );
                // pré-chargement du filtre passe-bas du biasOffset au point milieu de l'ADC
  static int    lastSampleVcorr = 0;
  static int    lastSampleIcorr = 0;
  int           analogVoltage;
  int           analogCurrent;
  int           sampleVcorr;
  int           sampleVcorrDelayed;
  int           sampleIcorr;
  static int    sampleIcorrDelayed;
  long          sampleP;

  // conversion du courant disponible
  if ( readFlagADC == 0 ) {

    // Configuration pour l'acquisition de la tension
    ADMUX &= B11110000;
    ADMUX |= voltageSensorMUX;
    // Démarrage de la conversion
    ADCSRA |= B01000000;
    //Flag pour indiquer une conversion de tension
    readFlagADC = 9;

    // Si fin d'un cycle secteur, transfert de la suite des données statistiques
    if ( stats_ready_flag == 9 ) {
      stats_sumP = sumP;                  // Somme des échantillons de puissance
      sumP = 0;
      stats_sumVsqr = sumVsqr;            // Somme des échantillons de tension au carré
      sumVsqr = 0;
      stats_sumIsqr = sumIsqr;            // Somme des échantillons de courant au carré
      sumIsqr = 0;
      stats_sumV = sumV;                  // Somme des échantillons de tension
      sumV = 0;
      stats_sumI = sumI;                  // Somme des échantillons de courant
      sumI = 0;
      stats_samples = samples;            // Nombre d'échantillons total
      samples = 0;
      // Flag : toutes les données statistiques ont été transférées
      stats_ready_flag = 1;
    }

    analogCurrent = ADCL | ( ADCH << 8 );

    // Calculs après la conversion du courant
    sampleIcorr = analogCurrent - biasOffset;
    // Echantillon de courant sur lequel on applique un délai
    // pour corriger la phase en fonction d'une estimation linéaire d'évolution
    sampleIcorrDelayed = int ( ( 16 - PHASE_CALIB ) * ( sampleIcorr - lastSampleIcorr ) + ( sampleIcorr << 4  ) ) >> 4;
    // Calcul pour les statistiques
    sumIsqr += long ( sampleIcorr ) * long ( sampleIcorr );
    sumI += long ( sampleIcorr );

    // Calcul pour la mise à jour de biasOffset
    biasOffset = int ( ( fBiasOffset + FILTERROUNDING ) >> FILTERSHIFT );

    // détection des erreurs
    if ( ( analogCurrent == 0 ) || ( analogCurrent == 1023 ) ) error_status |= B00010000;

    lastSampleIcorr = sampleIcorr;
  }

  // Sinon conversion de tension disponible
  else if ( readFlagADC == 9 ) {

    //Configuration pour l'acquisition du courant
    ADMUX &= B11110000;
    ADMUX |= currentSensorMUX;
    //Démarrage de la conversion du courant
    ADCSRA |= B01000000;
    //Flag pour indiquer une conversion de courant
    readFlagADC = 0;

    // Si fin d'un cycle secteur, transfert de la suite des données statistiques
    if ( stats_ready_flag == 9 ) {
      stats_sumP = sumP;                  // Somme des échantillons de puissance
      sumP = 0;
      stats_sumVsqr = sumVsqr;            // Somme des échantillons de tension au carré
      sumVsqr = 0;
      stats_sumIsqr = sumIsqr;            // Somme des échantillons de courant au carré
      sumIsqr = 0;
      stats_sumV = sumV;                  // Somme des échantillons de tension
      sumV = 0;
      stats_sumI = sumI;                  // Somme des échantillons de courant
      sumI = 0;
      stats_samples = samples;            // Nombre d'échantillons total
      samples = 0;
      // Flag : toutes les données statistiques ont été transférées
      stats_ready_flag = 1;
    }

    analogVoltage = ADCL | ( ADCH << 8 );

    // Calculs après la conversion de tension
    sampleVcorr = analogVoltage - biasOffset;
    // Calcul de l'échantillon de tension sur lequel on applique un délai
    // pour corriger la phase en fonction d'une estimation linéaire d'évolution
    sampleVcorrDelayed = int ( ( PHASE_CALIB - 16 ) * ( sampleVcorr - lastSampleVcorr ) + ( sampleVcorr << 4 ) ) >> 4;
    // somme des échantillons de puissance pour le calcul de la puissance active
    sampleP = long ( sampleVcorrDelayed ) * long ( sampleIcorrDelayed );
    periodP += sampleP;

    // détection du passage à 0 et génération du signal de synchronisation (changement d'état)
    if ( sampleVcorr >= 0 )  SYNC_ON;         //digitalWrite ( synchroOutPin, ON  );
    else                     SYNC_OFF;        //digitalWrite ( synchroOutPin, OFF );

    // détection des erreurs
    if ( ( analogVoltage == 0 ) || ( analogVoltage == 1023 ) ) error_status |= B00010000;

    // Calcul pour les statistiques
    sumVsqr += long ( sampleVcorr ) * long ( sampleVcorr );
    sumV += long ( sampleVcorr );

    // Calcul pour la mise à jour de biasOffset
    fBiasOffset += sampleVcorr;

    lastSampleVcorr = sampleVcorr;
    samples ++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// ISR ( TIMER1_COMPA_vect )  et  ISR ( TIMER1_OVF_vect )                           //
// Interrupt service routine Timer1 pour générer le pulse de déclenchement du TRIAC //
//////////////////////////////////////////////////////////////////////////////////////

ISR ( TIMER1_COMPA_vect ) {   // TCNT1 = OCR1A : instant de déclenchement du SSR/TRIAC
  TRIAC_ON;
  // chargement du compteur pour que le pulse SSR/TRIAC s'arrête à l'instant PULSE_END
  // relativement au passage à 0 (nécessite PULSE_END > OCR1A) 
  TCNT1 = 65535 - ( PULSE_END - OCR1A );
}

ISR ( TIMER1_OVF_vect ) {     // TCNT1 overflow, instant PULSE_END
  TRIAC_OFF;
  TCCR1B = 0x00;              // arrêt du Timer
  TCNT1 = 0;                  // on remet le compteur à 0 par sécurité
}

///////////////////////////////////////////////////////////////////////
///////////// FIN DES ROUTINES d'INTERRUPTIONS ////////////////////////
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// DEFINITION DES FONCTIONS //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// configADC                                                                        //
// Fonction de configuration du convertisseur ADC                                   //
//////////////////////////////////////////////////////////////////////////////////////

void configADC ( void ) {

  // Configuration : voir documention ATMEGA328P
  // Fonctionnement sur interruption et choix de la référence
  ADMUX &= B11011111;
  ADMUX &= B00111111;
  ADMUX |= B01000000;
  ADCSRA &= B00000000;
  ADCSRA |= B10000000;
  ADCSRA |= B00001000;
  ADCSRA |= B00000110; // Prescaler to 64 (==> 166 échantillons par cycle)

  // désactivaton des I/O digitales branchées sur les ports ADC utilisés
  DIDR0 |= ( B00000001 << voltageSensorMUX );
  DIDR0 |= ( B00000001 << currentSensorMUX );

  // Sélection du port analogique correspondant au courant
  ADMUX &= B11110000;
  ADMUX |= currentSensorMUX;
  
  // La conversion démarrera en mettant à 1 le bit ADSC de ADCSRA
  // après avoir configuré ADMUX pour le choix de l'entrée analogique
  // Pour démarrer : ADCSRA |= B01000000;
}

//////////////////////////////////////////////////////////////////////////////////////
// configTimer1                                                                     //
// Fonction de configuration du Timer 1, gestion du pulse SSR/TRIAC                 //
//////////////////////////////////////////////////////////////////////////////////////

void configTimer1 ( void ) {

  TIMSK1 = 0x03;     // activation des interruptions sur comparateur et overflow
  TCCR1A = 0x00;     // fonctionnement normal,
  TCCR1B = 0x00;     // timer arrêté
  OCR1A  = 30000;    // comparateur initialisé à 30000
  TCNT1  = 0;        // compteur initialisé à 0

  /*******************  Pour information : *****************************
    //TCCR1B=0x05; // démarrage du compteur par pas de 64 us
    //TCCR1B=0x00; // arrêt du compteur
  ********************************************************************/
}


//////////////////////////////////////////////////////////////////////////////////////
// startPVR                                                                         //
// Fonction de démarrage                                                            //
// Premier démarrage ou re-démarrage                                                //
//////////////////////////////////////////////////////////////////////////////////////

void startPVR ( void ) {

  delay ( 200 );
  noInterrupts ( );
  // Configuration du convertisseur ADC pour travailler sur interruptions
  configADC ( );
  // Configuration du Timer1
  configTimer1 ( );
  // Configuration de l'entrée d'interruption synchroACPin
  attachInterrupt ( digitalPinToInterrupt ( synchroACPin ), zeroCrossingInterrupt, CHANGE );
  // Initialisations pour le premier cycle
  coldStart = NCSTART;
  stats_error_status = 0;
  stats_ready_flag = 0;
  interrupts ( );
  delay ( 200 );
   // Démarrage de la première conversion ADC = démarrage du routeur
  ADCSRA |= B01000000;

  // *** Si période de warm-up, affichage des ..... pendant 100 ms
  // *** avant de redonner la main au programme
  unsigned long refTime = millis ( );
  while ( ( coldStart > 0 ) && ( ( millis ( ) - refTime ) < 100 ) ) {
    delay ( 1 );
    Serial.print ( F(".") );
  };
}


//////////////////////////////////////////////////////////////////////////////////////
// stopPVR                                                                          //
// Fonction d'arrêt                                                                 //
//////////////////////////////////////////////////////////////////////////////////////

void stopPVR ( void ) {

  // arrêt ADC et arrêt interruption ADC
  ADCSRA = 0x00;
  // arrêt interruption détection passage par zéro
  detachInterrupt ( digitalPinToInterrupt ( synchroACPin ) );
  // arrêt du Timer 1
  TCCR1B = 0x00;
  // arrêt du SSR
  digitalWrite ( pulseTriacPin, OFF );
  // arrêt du relais secondaire de délestage
  digitalWrite ( relayPin, OFF );
}

//////////////////////////////////////////////////////////////////////////////////////
// configPrint                                                                      //
// Fonction d'affichage de la configuration courante                                //
//////////////////////////////////////////////////////////////////////////////////////

void configPrint ( void ) {

  int i = 0;
  char buffer [50];
  clearScreen ( );

  Serial.print ( F("  >>>> EcoPV version ") );
  Serial.print ( F(VERSION) );
  Serial.println ( F("  <<<<") );
  Serial.println ( F("  >>>> Configuration courante <<<<\n") );

  while ( i < NB_PARAM ) {
    printTab ( );
    Serial.print ( ( i + 1 ) );
    Serial.print ( F(". ") );
    strcpy_P ( buffer, (char *)pgm_read_word(&(pvrParamName[i])) );
    Serial.print ( buffer );
    printTab ( );
    switch ( pvrParamConfig [i].dataType ) {
      case 0: {
          int *tmp_int = (int *) pvrParamConfig [i].adr;
          Serial.println ( *tmp_int );
          break;
        }
      case 1: {
          float *tmp_float = (float *) pvrParamConfig [i].adr;
          Serial.println ( *tmp_float, 6 );
          break;
        }
      case 2: {
          byte *tmp_byte_array = (byte *) pvrParamConfig [i].adr;
          Serial.println ( F("Table [0..255]") );
          for (int j = 0; j <= 15; j++) {
            printTab ( );
            for (int k = 0; k <= 15; k++) {
              Serial.print ( *tmp_byte_array );
              printTab ( );
              tmp_byte_array ++;
            };
            Serial.println ( );
          };
          break;
        }
      case 4: {
          byte *tmp_byte = (byte *) pvrParamConfig [i].adr;
          Serial.println ( *tmp_byte );
          break;
        }
      case 5: {
          long *tmp_long = (long *) pvrParamConfig [i].adr;
          Serial.println ( *tmp_long );
          break;
        }
    }
    i++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// configChange                                                                     //
// Fonction de modification de la configuration courante                            //
//////////////////////////////////////////////////////////////////////////////////////

void configChange ( void ) {

  long  valueInt = 0;
  float valueFloat = 0;
  int   minValue;
  int   maxValue;
  char  buffer [64];

  configPrint ( );

  clearSerialInputCache ( );
  Serial.println ( F("\n\
  >>>>> Paramètre à modifier + entrée ? (ou 0 pour sortir)\t") );

  int choice = Serial.parseInt ( );

  // Cas général
  if ( ( choice > 0) && ( choice <= NB_PARAM ) ) {
    int index = choice - 1;
    if ( pvrParamConfig [index].advancedParameter )
      Serial.println ( F("  /!\\ ATTENTION, VOUS MODIFIEZ UN PARAMETRE AVANCE /!\\") );
    strcpy_P ( buffer, (char *)pgm_read_word ( &(pvrParamName[index]) ) );
    Serial.print ( F("  Valeur courante de ") );
    Serial.print ( buffer );
    Serial.print ( F("= ") );
    byte dataType = pvrParamConfig [index].dataType;
    switch ( dataType ) {
      case 0: {
          int *tmp_int = (int *) pvrParamConfig [index].adr;
          Serial.print ( *tmp_int );
          break;
        }
      case 1: {
          float *tmp_float = (float *) pvrParamConfig [index].adr;
          Serial.print ( *tmp_float, 6 );
          break;
        }
      case 4: {
          byte *tmp_byte = (byte *) pvrParamConfig [index].adr;
          Serial.print ( *tmp_byte );
          break;
        }
      case 5: {
          long *tmp_long = (long *) pvrParamConfig [index].adr;
          Serial.print ( *tmp_long );
          break;
        }
    }
    Serial.print ( F("  |  min = ") );
    minValue = pvrParamConfig [index].minValue;
    Serial.print ( minValue );
    Serial.print ( F("  |  max = ") );
    maxValue = pvrParamConfig [index].maxValue;
    Serial.println ( maxValue );

    clearSerialInputCache ( );
    Serial.print ( F("  Nouvelle valeur ? ") );

    if ( dataType == 1 ) {
      valueFloat = Serial.parseFloat ( );
      valueFloat = constrain ( valueFloat, float ( minValue ), float ( maxValue ) );
    }
    else {
      valueInt = Serial.parseInt ( );
      valueInt = constrain ( valueInt, minValue, maxValue );
    }

    Serial.print ( F("  Nouvelle valeur de ") );
    Serial.print ( buffer );
    Serial.print ( F("= ") );

    switch ( dataType ) {
      case 0: {
          int *tmp_int = (int *) pvrParamConfig [index].adr;
          noInterrupts ( );
          *tmp_int = int ( valueInt );
          interrupts ( );
          Serial.println ( *tmp_int );
          break;
        }
      case 1: {
          float *tmp_float = (float *) pvrParamConfig [index].adr;
          noInterrupts ( );
          *tmp_float = float ( valueFloat );
          interrupts ( );
          Serial.println ( *tmp_float, 6 );
          break;
        }
      case 4: {
          byte *tmp_byte = (byte *) pvrParamConfig [index].adr;
          noInterrupts ( );
          *tmp_byte = byte ( valueInt );
          interrupts ( );
          Serial.println ( *tmp_byte );
          break;
        }
      case 5: {
          long *tmp_long = (long *) pvrParamConfig [index].adr;
          noInterrupts ( );
          *tmp_long = long ( valueInt );
          interrupts ( );
          Serial.println ( *tmp_long );
          break;
        }
    }
    clearSerialInputCache ( );
    Serial.println ( F("\nPensez à enregistrer vos modifications en EEPROM !") );
  }
}


//////////////////////////////////////////////////////////////////////////////////////
// configuration                                                                    //
// Fonction de menu de configuration du PV routeur                                  //
//////////////////////////////////////////////////////////////////////////////////////

void configuration ( void ) {

#define MENU0 "\
    ************************************\n\
    *****    EcoPV set-up menu     *****\n\
    ************************************\n\n"
#define MENU1 "    0.\tQuitter\n\
    1.\tAfficher la version\n\n"
#define MENU2 "    11.\tAfficher la configuration courante\n\
    12.\tCharger la configuration\n\
    13.\tSauvegarder la configuration\n\
    14.\tModifier la configuration\n\n"
#define MENU3 "    21.\tAfficher les index\n\
    22.\tSauvegarder les index\n\
    23.\tMettre à zéro des index\n\
    24.\tModifier des index\n\n"
#define MENU4 "    81.\tDumper l'EEPROM\n\
    82.\tFormater l'EEPROM\n\n\
    99.\tRedémarrer le système\n\n\
Choix (+ entrée) ? \t"

  while ( true ) {
    clearScreen ( );
    clearSerialInputCache ( );
    Serial.print ( F(MENU0) );
    Serial.print ( F(MENU1) );
    Serial.print ( F(MENU2) );
    Serial.print ( F(MENU3) );
    Serial.print ( F(MENU4) );
    int choice = Serial.parseInt ( );
    Serial.println ( );

    switch ( choice ) {
      case 0: {
          // Serial.println ( F("  >>>> Démarrage du PV routeur <<<<") );
          clearSerialInputCache ( );
          return;
          break;
        }
      case 1: {
          clearScreen ( );
          versionPrint ( );
          optionPrint ( );
          break;
        }
      case 11: {
          clearScreen ( );
          configPrint ( );
          break;
        }
      case 12: {
          clearScreen ( );
          if ( eeConfigRead ( ) )
            Serial.println ( F("  >>>> Configuration chargée ! <<<<") );
          else
            Serial.println ( F("  >>>> Pas de configuration en EEPROM ! <<<<\n>>>> Configuration par défaut <<<<") );
          break;
        }
      case 13: {
          clearScreen ( );
          eeConfigWrite ( );
          Serial.println ( F("  >>>> Configuration sauvegardée ! <<<<") );
          break;
        }
      case 14: {
          clearScreen ( );
          configChange ( );
          break;
        }
      case 21: {
          clearScreen ( );
          Serial.println ( F("  >>>> Valeur des index <<<<") );
          Serial.print ( F("  Index d'énergie ") );
          Serial.print ( F("routée   : ") );
          Serial.print ( indexKWhRouted, 3 );
          Serial.println ( F(" kWh") );
          Serial.print ( F("  Index d'énergie ") );
          Serial.print ( F("exportée : ") );
          Serial.print ( indexKWhExported, 3 );
          Serial.println ( F(" kWh") );
          Serial.print ( F("  Index d'énergie ") );
          Serial.print ( F("importée : ") );
          Serial.print ( indexKWhImported, 3 );
          Serial.println ( F(" kWh") );
          break;
        }
      case 22: {
          clearScreen ( );
          indexWrite ( );
          Serial.println ( F("  >>>> Index sauvegardés ! <<<<") );
          break;
        }
      case 23: {
          clearScreen ( );
          indexKWhRouted = 0;
          indexKWhExported = 0;
          indexKWhImported = 0;
          indexWrite ( );
          Serial.println ( F("  >>>>  Index mis à zéro ! <<<<") );
          break;
        }
      case 24: {
          clearScreen ( );
          Serial.println ( F("  >>>> Valeur des index <<<<") );
          Serial.print ( F("1. Index d'énergie ") );
          Serial.print ( F("routée   : ") );
          Serial.print ( indexKWhRouted, 3 );
          Serial.println ( F(" kWh") );
          Serial.print ( F("2. Index d'énergie ") );
          Serial.print ( F("exportée : ") );
          Serial.print ( indexKWhExported, 3 );
          Serial.println ( F(" kWh") );
          Serial.print ( F("3. Index d'énergie ") );
          Serial.print ( F("importée : ") );
          Serial.print ( indexKWhImported, 3 );
          Serial.println ( F(" kWh") );
          clearSerialInputCache ( );
          Serial.println ( F("\n\
  >>>>> Index à modifier + entrée ? (ou 0 pour sortir)\t") );
          int choice = Serial.parseInt ( );
          if ( ( choice > 0 ) && ( choice < 4 ) ) {
            Serial.print ( F("  Nouvelle valeur ? ") );
            float valueFloat = Serial.parseFloat ( );
            switch ( choice ) {
              case 1 : {
                 indexKWhRouted = valueFloat;
                 break;
              }
              case 2 : {
                 indexKWhExported = valueFloat;
                 break;
              }
              case 3 : {
                 indexKWhImported = valueFloat;
                 break;
              }
            }
            indexWrite ( );
            Serial.println ( F("  >>>>  Index modifié ! <<<<") );           
            }
          break;
        }
      case 81: {
          clearScreen ( );
          Serial.println ( F("  >>>>  Dump configuration (expérimental) <<<<") );
          eeConfigDump ( );
          Serial.println ( );
          break;
        }
      case 82: {
          clearScreen ( );
          Serial.println ( F("  >>>>  Effacement de l'EEPROM en cours <<<<") );
          for ( int i = 0 ; i < int ( EEPROM.length ( ) ) ; i++ ) {
            if ( ( i % 50 ) == 0) Serial.print ( F(".") );
            EEPROM.write ( i, 0 );
          }
          Serial.println ( F("  Contenu de l'EEPROM effacé !") );
          Serial.println ( F("  !! Veuillez sauvegarder ou mettre à 0 les index !!") );
          Serial.println ( F("  !! Sauvegardez la configuration pour la conserver !!") );
          Serial.println ( F("  Sinon restauration de la configuration par défaut au prochain démarrage.") );
          break;
        }
      case 99: {
          if ( coldStart == 0 ) {  // On ne redémarre pas si on est encore dans le SETUP
                                  // ou en phase de démarrage
            indexWrite ( );
            delay ( 500 );
            stopPVR ( );
            Serial.println ( F("\n  Redémarrage en cours...") );
            delay ( 500 );
            startPVR ( );  
          }
          clearSerialInputCache ( );
          return;
          break;
        }
      default: {
          Serial.println ( F("\n  >>>> Choix incorrect <<<<") );
          break;
        }
    }
    pressToContinue ( );
  }
}


//////////////////////////////////////////////////////////////////////////////////////
// fatalError                                                                       //
// Fonction de gestion d'erreur majeure                                             //
//////////////////////////////////////////////////////////////////////////////////////

void fatalError ( void ) {

  stopPVR ( );
  // Le système est mis en sécurité
  indexWrite ( );
  // Sauvegarde des index par sécurité

#if defined (OLED_128X64)
  oled.clear ( );
  oled.set2X ( );
  oled.println ( F("***********") );
  oled.println ( F("*  MAJOR  *") );
  oled.println ( F("* FAILURE *") );
  oled.println ( F("***********") );
#endif


#if defined (PV_STATS) || defined (PV_MOD_CONFIG)
  clearScreen ( );
  Serial.print ( F("\n\n***** !!  A T T E N T I O N  !! *****\n\n\
Une erreur majeure s'est produite.\n\
") );
  Serial.print ( F("Bits de statut : ") );
  Serial.println ( stats_error_status, BIN );
  Serial.println ( );
  Serial.print ( F("Les causes possibles sont :\n\
- Tension secteur perturbée\n\
- Défaillance du circuit de lecture de la tension\n\
") );
  Serial.print ( F("- Défaillance du système\n\n\
") );
  Serial.println ( F("Le système a été mis en sécurité,") );
  Serial.println ( F("et tentera de redémarrer dans une minute.") );
  Serial.println ( F("Appuyez sur entrée pour un redémarrage immédiat.\n") );
#endif

  clearSerialInputCache ( );

  for ( int k = 0; k <= 300; k++ ) {
    digitalWrite ( pulseTriacPin, OFF ); //arrêt du SSR par sécurité
    digitalWrite ( ledPinStatus,  OFF );
    digitalWrite ( ledPinRouting, ON );
    delay ( 100 );
    digitalWrite ( ledPinStatus,  ON );
    digitalWrite ( ledPinRouting, OFF );
    delay ( 100 );
    if ( Serial.available ( ) )
      break;
  };

#if defined (PV_STATS) || defined (PV_MOD_CONFIG)
  Serial.println ( F("Redémarrage...\n") );
#endif

  clearSerialInputCache ( );
  startPVR ( ); 
}


//////////////////////////////////////////////////////////////////////////////////////
// eeConfigRead                                                                     //
// Fonction de lecture de la configuration EEPROM                                   //
//////////////////////////////////////////////////////////////////////////////////////

bool eeConfigRead ( void ) {

  dataEeprom pvrConfig;

  EEPROM.get ( PVR_EEPROM_START, pvrConfig );

  if ( pvrConfig.magic != DATAEEPROM_MAGIC ) return false;
  else {
    noInterrupts ( );
    PHASE_CALIB   = pvrConfig.phase_calib;
    P_OFFSET      = pvrConfig.p_offset;
    P_MARGIN      = pvrConfig.p_margin;
    GAIN_P        = pvrConfig.gain_p;
    GAIN_I        = pvrConfig.gain_i;
    E_RESERVE     = pvrConfig.e_reserve;
    interrupts ( );
    V_CALIB       = pvrConfig.v_calib;
    P_CALIB       = pvrConfig.p_calib;
    P_RESISTANCE  = pvrConfig.p_resistance;
    P_DIV2_ACTIVE = pvrConfig.p_div2_active;
    P_DIV2_IDLE   = pvrConfig.p_div2_idle;
    T_DIV2_ON     = pvrConfig.t_div2_on;
    T_DIV2_OFF    = pvrConfig.t_div2_off;
    T_DIV2_TC     = pvrConfig.t_div2_tc;
    return true;
  }
}


//////////////////////////////////////////////////////////////////////////////////////
// eeConfigWrite                                                                    //
// Fonction d'écriture de la configuration EEPROM                                   //
//////////////////////////////////////////////////////////////////////////////////////

void eeConfigWrite ( void ) {

  dataEeprom pvrConfig;

  pvrConfig.magic           = DATAEEPROM_MAGIC;
  pvrConfig.struct_version  = DATAEEPROM_VERSION;
  pvrConfig.v_calib         = V_CALIB;
  pvrConfig.p_calib         = P_CALIB;
  pvrConfig.phase_calib     = PHASE_CALIB;
  pvrConfig.p_offset        = P_OFFSET;
  pvrConfig.p_resistance    = P_RESISTANCE;
  pvrConfig.p_margin        = P_MARGIN;
  pvrConfig.gain_p          = GAIN_P;
  pvrConfig.gain_i          = GAIN_I;
  pvrConfig.e_reserve       = E_RESERVE;
  pvrConfig.p_div2_active   = P_DIV2_ACTIVE;
  pvrConfig.p_div2_idle     = P_DIV2_IDLE;
  pvrConfig.t_div2_on       = T_DIV2_ON;
  pvrConfig.t_div2_off      = T_DIV2_OFF;
  pvrConfig.t_div2_tc       = T_DIV2_TC;

  EEPROM.put ( PVR_EEPROM_START, pvrConfig );
}


//////////////////////////////////////////////////////////////////////////////////////
// eeConfigDump                                                                     //
// Fonction de dump de la configuration EEPROM                                      //
//////////////////////////////////////////////////////////////////////////////////////

void eeConfigDump ( void ) {

  byte pvrConfigDump [PVR_EEPROM_SIZE];

  EEPROM.get ( PVR_EEPROM_START, pvrConfigDump );
  for ( int i = 0; i < PVR_EEPROM_SIZE; i++ ) Serial.print ( char ( pvrConfigDump[i] ) );
  Serial.println ( );
}


//////////////////////////////////////////////////////////////////////////////////////
// indexRead                                                                        //
// Fonction de lecture des index en EEPROM                                          //
//////////////////////////////////////////////////////////////////////////////////////

void indexRead ( void ) {
  
  EEPROM.get ( PVR_EEPROM_INDEX_ADR, indexKWhRouted );
  EEPROM.get ( ( PVR_EEPROM_INDEX_ADR + 4 ), indexKWhExported );
  EEPROM.get ( ( PVR_EEPROM_INDEX_ADR + 8 ), indexKWhImported );
}


//////////////////////////////////////////////////////////////////////////////////////
// indexWrite                                                                       //
// Fonction d'écriture des index en EEPROM                                          //
//////////////////////////////////////////////////////////////////////////////////////

void indexWrite ( void ) {
  
  EEPROM.put ( PVR_EEPROM_INDEX_ADR, indexKWhRouted );
  EEPROM.put ( ( PVR_EEPROM_INDEX_ADR + 4 ), indexKWhExported );
  EEPROM.put ( ( PVR_EEPROM_INDEX_ADR + 8 ), indexKWhImported );
  delay (10);
}


//////////////////////////////////////////////////////////////////////////////////////
// upTime                                                                           //
// Fonction de lecture de mise à jour des données de l'horloge interne              //
//////////////////////////////////////////////////////////////////////////////////////

void upTime ( void ) {

  unsigned long stats_PVRClock;

  noInterrupts ( );
  stats_PVRClock = PVRClock;
  interrupts ( );
  stats_PVRClock /= NB_CYCLES;
  secondsOnline = ( stats_PVRClock         ) % 60;
  minutesOnline = ( stats_PVRClock / 60    ) % 60;
  hoursOnline   = ( stats_PVRClock / 3600  ) % 24;
  daysOnline    = ( stats_PVRClock / 86400 );
      // daysOnline est limité à 994 jours modulo 256, puis repassera à 0
}


//////////////////////////////////////////////////////////////////////////////////////
// PVRScheduler                                                                     //
// Fonction Scheduler                                                               //
//////////////////////////////////////////////////////////////////////////////////////

void PVRScheduler ( void ) {

  //*** Toutes les heures                                               ***
  if ( ( minutesOnline == 0 ) && ( secondsOnline == 0 ) ) {
    // Enregistrement des index en mémoire EEPROM
    indexWrite ( );                   
  }

  // *** Envoi des donnéees statistiques vers MYSENSORS si activé       ***
  // *** Période d'envoi définie par MYSENSORS_TRANSMIT_PERIOD          ***
  // *** Envoi à la 3ème seconde de l'intervalle de temps               ***
#if ( ( defined (MYSENSORS_COM) ) && ( defined (MYSENSORS_TRANSMIT_PERIOD) ) )
  if ( MYSENSORS_TRANSMIT_PERIOD > 0 ) {
    if ( ( secondsOnline % MYSENSORS_TRANSMIT_PERIOD ) == 3 ) mySensorsTransmit ( );
  }
#endif 

  // *** Affichage des donnéees statistiques sur écran oled si activé   ***
  // *** Période d'envoi définie par OLED_128X64_REFRESH_PERIOD         ***
  // *** Envoi page 0 à la 2ème seconde de l'intervalle de temps        ***
  // *** et page 1 à la 5ème seconde                                    ***
#if ( ( defined (OLED_128X64) ) && ( defined (OLED_128X64_REFRESH_PERIOD) ) )
  if ( OLED_128X64_REFRESH_PERIOD > 0 ) {
    if ( ( secondsOnline % OLED_128X64_REFRESH_PERIOD ) == 2 ) oLedPrint ( 0 );
    if ( ( secondsOnline % OLED_128X64_REFRESH_PERIOD ) == 5 ) oLedPrint ( 1 );
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
// PVRLed                                                                           //
// Fonction gestion allumage des leds de signalisation                              //
//////////////////////////////////////////////////////////////////////////////////////

void PVRLed ( void ) {

  byte routingByte = stats_error_status & B00001011;
  byte errorByte = stats_error_status & B11110000;

  if ( routingByte == 0 ) {         // pas de routage
    digitalWrite ( ledPinRouting, OFF );      // led éteinte
  }
  else if ( routingByte == 1 ) {    // routage en régulation
    digitalWrite ( ledPinRouting, ON  );      // allumage fixe
  }
  else {                            // autre cas : routage à 100 % voire exportation
    digitalWrite ( ledPinRouting, ( ( ledBlink & B00001000 ) == 0 ) ? 0 : 1 ); // T = 320 ms
  }

  if ( errorByte == 0 ) {          // pas d'ereur
    digitalWrite ( ledPinStatus, ( ( ledBlink & B01000000 ) == 0 ) ? 0 : 1 ); // T = 2560 ms
  }
  else if ( errorByte < 64 ) {     // anomalie sur les signaux analogiques ou le taux d'acquisition
    digitalWrite ( ledPinStatus, ( ( ledBlink & B00001000 ) == 0 ) ? 0 : 1 ); // T = 320 ms
  }
  else {                           // autre cas : anomalie furtive voire grave de détection du passage à 0
    digitalWrite ( ledPinStatus, ON  );       // allumage fixe
  }
}


//////////////////////////////////////////////////////////////////////////////////////
// clearSerialInputCache                                                            //
// Fonction de vidage du buffer de réception de la liaison série                    //
//////////////////////////////////////////////////////////////////////////////////////

void clearSerialInputCache ( void ) {
  
  while ( Serial.available ( ) > 0 ) Serial.read ( );
}


//////////////////////////////////////////////////////////////////////////////////////
// printTab                                                                         //
// Fonction Afficher tabulation                                                     //
//////////////////////////////////////////////////////////////////////////////////////

void printTab ( void ) {
  
  Serial.print ( F("\t") );
}


//////////////////////////////////////////////////////////////////////////////////////
// clearScreen                                                                      //
// Fonction Effacer terminal                                                        //
// Ne fonctionne qu'avec un vrai terminal, pas avec la console de l'IDE Arduino     //
//////////////////////////////////////////////////////////////////////////////////////

void clearScreen ( void ) {
  
  Serial.write ( 27 );       // ESC
  Serial.print ( "[2J" );    // clear screen
  Serial.write ( 27 );       // ESC
  Serial.print ( "[H" );     // cursor to home
}


//////////////////////////////////////////////////////////////////////////////////////
// pressToContinue                                                                  //
// Fonction "Appui sur entrée pour continuer"                                       //
//////////////////////////////////////////////////////////////////////////////////////

void pressToContinue ( void ) {
  
  unsigned long refTime = millis ( );
  
  clearSerialInputCache ( );
  Serial.println ( F("\nAppuyez sur entrée pour continuer...") );
  while ( ( Serial.available ( ) == 0 ) && ( ( millis ( ) - refTime ) < SERIALTIMEOUT ) );
  clearSerialInputCache ( );
}


//////////////////////////////////////////////////////////////////////////////////////
// analogReadReference                                                              //
// Fonction de lecture de la référence interne de tension                           //
// Adapté de : https://www.carnetdumaker.net/snippets/77/                           //
//////////////////////////////////////////////////////////////////////////////////////

unsigned int analogReadReference ( void ) {

  ADCSRA &= B00000000;
  ADCSRA |= B10000000;
  ADCSRA |= B00000111;
  /* Elimine toutes charges résiduelles */
  ADMUX = 0x4F;
  delayMicroseconds ( 500 );
  /* Sélectionne la référence interne à 1.1 volts comme point de mesure, avec comme limite haute VCC */
  ADMUX = 0x4E;
  delayMicroseconds ( 1500 );

  /* Lance une conversion analogique -> numérique */
  ADCSRA |= ( 1 << ADSC );
  /* Attend la fin de la conversion */
  while ( ( ADCSRA & ( 1 << ADSC ) ) );
  /* Récupère le résultat de la conversion */
  return ADCL | ( ADCH << 8 );

  /* Note : Idéalement VCC = 5 volts = 1023 'bits' en conversion ADC
    1 'bit' vaut VCC / 1023
    Référence interne à 1.1 volts = ( 1023 * 1.1 ) / VCC = 225 'bits'
    En mesurant la référence à 1.1 volts, on peut déduire la tension d'alimentation réelle du microcontrôleur
    vccSupply = ( 1023 * VCC_1BIT )
    avec   VCC_1BIT = ( 1.1 / analogReadReference ( ) )
  */
}

//////////////////////////////////////////////////////////////////////////////////////
// optionPrint                                                                      //
// Affichage des options de compilation activées                                    //
//////////////////////////////////////////////////////////////////////////////////////

void optionPrint ( void ) {

  // Affichage des options STATS
#if defined (PV_STATS)
  Serial.print ( F("Affichage des données statistiques ") );
  Serial.println ( F("par liaison série\t:\tactivé") );
#endif

  // Affichage des options CONFIG
#if defined (PV_MOD_CONFIG)
  Serial.print ( F("Modification de la configuration ") );
  Serial.println ( F("par liaison série\t:\tactivé") );
#endif

  // Affichage de l'option de communication disponible

#if defined (OLED_128X64)
  Serial.print ( F("Option de communication OLED_128X64 installée :\n") );
  Serial.print ( F("\tAdresse de l'écran : 0x") );
  Serial.println ( I2C_ADDRESS, HEX );
#endif
  
#if defined (MYSENSORS_COM)
  Serial.print ( F("Option de communication MYSENSORS installée :\n") );
  Serial.print ( F("\tID du noeud de capteur : ") );
  Serial.println ( MY_NODE_ID );
#endif

#if defined (ETHERNET_28J60)
  Serial.print ( F("Option de communication ETHERNET installée :\n") );
  Serial.print ( F("\tAdresse MAC de la carte\t:\t") );
  Serial.print ( ethMac [0], HEX );
  Serial.print ( F(":") );
  Serial.print ( ethMac [1], HEX );
  Serial.print ( F(":") );
  Serial.print ( ethMac [2], HEX );
  Serial.print ( F(":") );
  Serial.print ( ethMac [3], HEX );
  Serial.print ( F(":") );
  Serial.print ( ethMac [4], HEX );
  Serial.print ( F(":") );
  Serial.println ( ethMac [5], HEX );
  Serial.print ( F("\tAdresse IP de la carte\t:\t") );
  Serial.print ( ethIp [0] );
  Serial.print ( F(".") );
  Serial.print ( ethIp [1] );
  Serial.print ( F(".") );
  Serial.print ( ethIp [2] );
  Serial.print ( F(".") );
  Serial.println ( ethIp [3] );
  Serial.print ( F("\tPort de communication\t:\t") );
  Serial.println ( ethPort );
#endif

  Serial.println ( );
}


//////////////////////////////////////////////////////////////////////////////////////
// versionPrint                                                                     //
// Affichage de la version                                                          //
//////////////////////////////////////////////////////////////////////////////////////

void versionPrint ( void ) {

  Serial.print ( F("\n***** EcoPV version ") );
  Serial.print ( F(VERSION) );
  Serial.print ( F(" *****\n") );
  Serial.print ( F("EcoPV - Copyright (C) 2019 - Bernard Legrand and Mickaël Lefebvre\n\n") );
  Serial.print ( F("This program is free software: you can redistribute it and/or modify\n") );
  Serial.print ( F("it under the terms of the GNU Lesser General Public License as published\n") );
  Serial.print ( F("by the Free Software Foundation, either version 2.1 of the License, or\n") );
  Serial.print ( F("(at your option) any later version.\n\n") );
}


//////////////////////////////////////////////////////////////////////////////////////
// mySensorsTransmit                                                                //
// Fonction de transmission MYSENSORS                                               //
//////////////////////////////////////////////////////////////////////////////////////

#if defined (MYSENSORS_COM)

void mySensorsTransmit ( void ) {

  send ( msg_vrms.set   ( Vrms, 1 ) );
  send ( msg_irms.set   ( Irms, 2 ) );
  send ( msg_pwr.set    ( Pact, 0 ) );
  send ( msg_pva.set    ( Papp, 0 ) );
  send ( msg_prt.set    ( Prouted, 0 ) );
  send ( msg_pimp.set   ( ( ( Pact >= 0 ) ? Pact : 0 ), 0 ) );
  send ( msg_pexp.set   ( ( ( Pact <= 0 ) ? -Pact : 0 ), 0 ) );
  send ( msg_cosphi.set ( cos_phi, 2 ) );
  send ( msg_error.set  ( stats_error_status ) );
  send ( msg_kwh.set    ( long ( indexKWhRouted ) ) );
}

#endif

//////////////////////////////////////////////////////////////////////////////////////
// oLedPrint                                                                        //
// Fonction d'affichage écran oled                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#if defined (OLED_128X64)

void oLedPrint ( int page ) {

  oled.clear ( );
  oled.set2X ( );

  switch ( page ) {

    case 0 : {   
      oled.println ( F("  Running") );
      if ( Pact < 0 )
        oled.print ( F("Expt ") );
      else
        oled.print ( F("Impt ") );
      if ( abs ( Pact ) < 10 ) oled.print ( F("   ") );
      else if ( abs ( Pact ) < 100 ) oled.print ( F("  ") );
      else if ( abs ( Pact ) < 1000 ) oled.print ( F(" ") );
      oled.print ( abs ( Pact ), 0 );
      oled.println ( F("W") );
      oled.print ( F("Rout ") );
      if ( Prouted < 10 ) oled.print ( F("   ") );
      else if ( Prouted < 100 ) oled.print ( F("  ") );
      else if ( Prouted < 1000 ) oled.print ( F(" ") );
      oled.print ( Prouted, 0 );
      oled.println ( F("W") );
      oled.print ( F("Relay ") );
      if ( digitalRead ( relayPin ) == ON ) oled.println ( F("  On") );
        else oled.println ( F(" Off") ); 
      break;
    }

    case 1 : { 
      if ( stats_error_status > 15 ) oled.println ( F("! Check !") );
      else oled.println ( F("  Normal") );
      oled.print ( F(" ") );
      oled.print ( Vrms, 0 );
      oled.println ( F(" Volts") );
      if ( Irms < 10 ) oled.print ( F(" ") );
      oled.print ( Irms, 1 );
      oled.println ( F(" Amps") );
      oled.print ( F(" ") );
      oled.print ( abs ( cos_phi ), 1 );
      oled.println ( F(" Cosfi") );
      break;
     }   
  }    
}

#endif

//////////////////////////////////////////////////////////////////////////////////////
// ethernetProcess                                                                  //
// Fonction de gestion ETHERNET                                                     //
//////////////////////////////////////////////////////////////////////////////////////

#if defined (ETHERNET_28J60)

void ethernetProcess ( void ) {
  
  char *ethParam;
  
  ethParam = ethernet.serviceRequest ( );
    // Note : si aucune trame ethernet n'est disponible, alors ethParam est un pointeur de valeur 0
    // qui ne sera pas a priori l'adresse de la chaine de caractères de la requête si elle est valide
    // cf. codage de la méthode dans la librairie ETHER_28J60

    // Note : les fonctions de la bibliothèque ne permettent que d'écrire des nombres entiers (int)
    // De ce fait, les données numériques sont tronquées en entier.

  if ( ( unsigned int ) ethParam != 0  )
  {
    int ethParamLen = strlen ( ethParam );
    int ethParamNum;
    ethernet.print ( "{\"value\":\"" );
    
    if ( ( ethParamLen == 5 ) && ( strncmp ( "Get", ethParam, 3 ) == 0) ) {
      // Cas normal de demande de données : GetXX
      ethParamNum = atoi ( ( char* ) &ethParam [3] );
      switch ( ethParamNum ) {
        case 0: {
            ethernet.print ( "error param" );
            break;
        }
        case 1: {
            ethernet.print ( (int) Vrms );
            break;
        }
        case 2: {
            ethernet.print ( (int) Irms );
            break;
        }
        case 3: {
            ethernet.print ( (int) Pact );
            break;
        }
        case 4: {
            ethernet.print ( (int) Papp );
            break;
        }
        case 5: {
            ethernet.print ( (int) Prouted );
            break;
        }
        case 6: {
            ethernet.print ( (int) ( ( Pact >= 0 ) ? Pact : 0 ) );
            break;
        }
        case 7: {
            ethernet.print ( (int) ( ( Pact <= 0 ) ? -Pact : 0 ) );
            break;
        }
        case 8: {
            ethernet.print ( (int) ( 1000 * cos_phi ) );
            break;
        }
        case 9: {
            ethernet.print ( (int) indexKWhRouted );
            break;
        }
        case 10: {
            ethernet.print ( (int) indexKWhImported );
            break;
        }
        case 11: {
            ethernet.print ( (int) indexKWhExported );
            break;
        }
        case 20: {
            ethernet.print ( (int) stats_error_status );
            break;
        }
        case 21: {
            ethernet.print ( (int) daysOnline );
            ethernet.print ( ":" );
            ethernet.print ( (int) hoursOnline );
            ethernet.print ( ":" );
            ethernet.print ( (int) minutesOnline );
            ethernet.print ( ":" );
            ethernet.print ( (int) secondsOnline );
            break;
        }
        case 90: {
            indexKWhRouted   = 0;
            indexKWhExported = 0;
            indexKWhImported = 0;
            indexWrite ( );
            ethernet.print ( "ok" );
            break;
        } 
        case 99: {
            ethernet.print ( VERSION );
            break;
        } 
        default: {
            ethernet.print ( "error param" );
            break;
        }
      }
    }
    else if ( ethParamLen == 0 ) {        // Requête vide
      ethernet.print ( "empty param" );
    }
    else {
      ethernet.print ( "error param" );   // Erreur de requête
    }   
    ethernet.print ( "\"}\r\n" );
    ethernet.respond ( );
  }
}

#endif
