# EcoPV
EcoPV is a Arduino program that maximizes the use of PV production by monitoring home energy consumption and by diverting power to a resistive charge when needed.  
 
EcoPV est un programme pour Arduino (ATMega328P) qui permet de gérer l'excédent de production photovoltaïque dans une installation d'autoconsommation en effectuant un routage de l'excédent vers une charge résistive, typiquement la résistance d'un chauffe-eau électrique. Ainsi l'autoconsommation est optimisée.  
EcoPV est inspiré de réalisations précédentes dont des références sont données en fin de document.  

## Fonctionnement  
EcoPV mesure en permanence la puissance consommée par la maison. Lorsque le production photovoltaïque dépasse la consommation, cela est immédiatement détecté par EcoPV. EcoPV pilote alors un relais électronique qui va alimenter de manière variable une résistance électrique (chauffe-eau) pour équilibrer puissance consommée et puissance produite. Ainsi, l'excédent de production photovoltaïque est dirigé vers la résistance du chauffe-eau et valorisé en chaleur au lieu d'être cédé au réseau électrique extérieur.  
  
Techniquement, EcoPV est basé sur :  
* un échantillonage rapide de la puissance consommée (8300 fois par seconde),  
* une régulation proportionnelle-intégrale de la charge résistive,  
* une programmation du régulateur entièrement gérée par les interruptions de l'ATMega328.  
  
## Mise en oeuvre  
EcoPV nécessite plusieurs choses pour fonctionner :  
* **Une carte Arduino** basé sur un ATMega 328 5V 16 MHz de type Arduino Nano et son alimentation électrique :  
C'est le coeur du système qui exécute le programme EcoPV.ino  
* **Un circuit de mesure de la tension et du courant consommé par la maison** :  
C'est un circuit électronique analogique dont le schéma est donné dans le répertoire schematics. Il convertit la tension et le courant dans des valeurs acceptables par l'Arduino. Il utilise un transformateur pour mesurer la tension et une pince ampérémétrique pour mesurer le courant. Celle-ci devra être placée au niveau du fil de phase qui alimente la maison en aval du disjoncteur principal.  
* **Un relais électronique SSR (Solid State Relay)** de type *non zero crossing* capable d'être piloté par une tension de 5V. Ce relais électronique pilotera la résistance du chauffe-eau.  
  
De manière optionnelle, EcoPV peut être équipé de :  
* **2 LEDs signalant le fonctionnement,**  
* **Un relais secondaire de délestage :**  
Ce relais permet le pilotage d'une charge quelconque en tout ou rien en fonction de seuils de mise en route et d'arrêt définis.  
* **Un système de communication :**  
Il s'agit soit d'une communication des données selon le protocoles MYSENSORS et qui nécessite un module radio NRF24L01, soit d'une communication ethernet via une API HTTP et qui nécessite un shield ENC28J60 pour Arduino.  
  
Un schéma général de branchement est disponible dans le répertoire schematics.  
  
## Specifications de EcoPV  
* Mesure de la tension et du courant consommé : 8300 fois par seconde.  
* Régulation proportionnelle-intégrale calculée toutes les 10 ms pour le pilotage de la charge résistive.  
* Calcul toutes les secondes de Vrms, Irms, Pactive, Papparente, cos phi, Proutée, énergie importée, énergie exportée, énergie routée (estimation).  
* Relais secondaire à activation/désactivation sur seuils paramétrables.  
* Interface utilisateur par liaison série : voir le répertoire screenshots.  
* Affichage des statistiques de fonctionnement toutes les secondes sur la liaison série.  
* Paramétrage et calibrage complet par liaison série.  
* Sauvegarde des paramètres en EEPROM.  
* Sauvegarde des compteurs d'énergie en EEPROM.  
* Horloge et scheduler internes pour la programmation de tâches planifiées.  
* Sauvegarde des compteurs d'énergie en EEPROM.  
* Communication MYSENSORS optionnelle (option de compilation).  
* Communication ETHERNET optionnelle (option de compilation).  API HTTP de récupération des données.  
* Auto-contrôle du fonctionnement et visualisation du statut par mot d'état.  
  
## Mise en oeuvre du programme  
EcoPV nécessite l'installation de l'IDE Arduino disponible sur le site Arduino. Voir www.arduino.cc  
Pour l'utilisation de la communication MYSENSORS, les bibliothèques corresponsantes devront être installées. Voir www.mysensors.org  
Pour l'utilisation de la communication Ethernet, les 2 bibliothèques EtherShield et ETHER_28J60 devront être installées **manuellement**. Elles sont disponibles dans le répertoire libraries.  
  
Dans l'IDE de l'Arduino ainsi installé, ouvrir le programme EcoPV.ino.  
Choisir au début du code les options de compilation souhaitées.  
Compiler le programme et le télécharger dans la carte Arduino.  
Le programme démarrera automatiquement avec des paramètres par défaut.  
L'accès à l'interface utilisateur par liaison série se fait par défaut à la vitesse de 500 000 bauds.  
*Note : en absence du circuit analogique fonctionnel, le programme ne détectera pas la synchronisation secteur et entrera dans un mode d'erreur majeure.*  
  
## En pratique : réalisation des branchements  
TO DO  
Un schéma général des branchements est donné dans le répertoire schematics.  
  
## Calibrage et paramétrage  
TO DO  
Description des paramètres...  
  
## Sources et liens  
TO DO  
Forum photovoltaïque, travaux de Tignous  
Site Openenergy monitor  
  
## Remerciements  
TO DO  
  
## Responsabilité  
Les programmes et schémas proposés ont une vocation informative et pédagogique. Ils ont été testés avec succès par le ou les auteurs.  
Cependant le ou les auteurs de ces programmes et schémas déclinent toute responsabilité et ne pourraient être tenus pour responsables du fonctionnement et des conséquences de l'utilisation des programmes et des schémas mis à disposition.
