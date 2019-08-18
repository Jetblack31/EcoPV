# EcoPV
Arduino program that maximizes the use of PV production at home by monitoring energy consumption and diverting power to a resistive charge when needed.  
 
EcoPV est un programme pour Arduino (ATMega328P) qui permet de gérer l'excédent de production photovoltaïque dans une installation d'autoconsommation en effectuant un routage de l'excédent vers une charge résistive, typiquement la résistance d'un chauffe-eau électrique. Ainsi l'autoconsommation est optimisée.  
EcoPV est inspiré de réalisation précédentes dont des références sont données en fin de document.  

## Fonctionnement  
EcoPV mesure en permanence la puissance consommée par la maison. Lorsque le production photovoltaïque dépasse la consommation, la puissance consommée devient négative et cela est immédiatement détecté par EcoPV. EcoPV pilote alors un relais électronique qui va alimenter une résistance électrique (chauffe-eau) de manière à rétablir une puissance consommée nulle ou positive. Ainsi, l'excédent de production photovoltaïque est dirigé vers la résistance du chauffe-eau et converti en chaleur au lieu d'être cédé au réseau électrique.  

## Mise en oeuvre  
EcoPV nécessite plusieurs choses pour fonctionner :  
* **Une carte Arduino** de type Arduino Nano et son alimentation électrique :  
C'est le coeur du système qui exécute le programme EcoPV.ino  
* **Un circuit de mesure de la tension et du courant consommé par la maison** :  
C'est un circuit électronique analogique dont le schéma est donné dans le répertoire schematics. Il convertit la tension et le courant dans des valeurs acceptables par l'Arduino. Il utilise un transformateur pour mesurer la tension et une pince ampérémétrique pour mesurer le courant. Celle-ci devra être placée au niveau du fil de phase qui alimente la maison en aval du disjoncteur principal.  
* **Un relais électronique SSR (Solid State Relay)** de type *non zero crossing* capable d'être piloté par une tension de 5V. Ce relais électronique pilotera la résistance du chauffe-eau.  
  
De manière optionnelle, EcoPV peut être équipé de :  
* **2 LEDs signalant le fonctionnement**  
* **Un relais secondaire de délestage :**  
Ce relais permet le pilotage d'une charge quelconque en tout ou rien en fonction de seuis de mise en route et d'arrêt définis.  
* **Un système de communication :**  
Il s'agit soit d'une communication des données selon le protocoles MYSENSORS et qui nécessite un module radio NRF24L01, soit d'une communication ethernet via une API HTTP et qui nécessite un shield ENC28J60 pour Arduino.  
  
Un schéma général de branchement est disponible dans le répertoire schematics.  
  
## Specifications de EcoPV  
* Mesure de la tension et du courant consommé : 8300 fois par seconde.  
* Mesure de Vrms, Irms, Pactive, Papparente, cos phi, Proutée, énergie importée, énergie exportée, énergie routée (estimation).
* Régulation proportionnelle-intégrale calculée toutes les 10 ms pour le pilotage de la charge résistive.  
* Relais secondaire à activation/désactivation paramétrable.  
* Interface par liaison série : voir le répertoire screenshots.  
* Affichage des statistiques de fonctionnement toutes les secondes.  
* Paramétrage et claibrage complet par liaison série.  
* Sauvegarde des paramètres en EEPROM.  
* Sauvegarde des compteurs d'énergie en EEPROM.  
* Communication MYSENSORS optionnelle (option de compilation).  
* Communication ETHERNET optionnelle (option de compilation).  API HTPP de récupération des données.  
