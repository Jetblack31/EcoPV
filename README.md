Vous aimez ce projet ? Vous pouvez faire une donation pour m'encourager grâce au bouton **Sponsor** dans la barre ci-dessus !
Merci à tous pour vos messages et votre soutien au projet !

## **Ce dépôt EcoPV n'évoluera plus. La suite des développements est menée dans le dépôt MaxPV : https://github.com/Jetblack31/MaxPV**

# EcoPV
EcoPV is a Arduino program (ATMega328P) that maximizes the use of PV production by monitoring home energy consumption and by diverting power to a resistive charge when needed.  
 
EcoPV est un programme pour Arduino (ATMega328P) qui permet de gérer l'excédent de production photovoltaïque dans une installation d'autoconsommation en effectuant un routage de l'excédent vers une charge résistive, typiquement la résistance d'un chauffe-eau électrique. Ainsi l'autoconsommation est optimisée.  
EcoPV est inspiré de réalisations précédentes dont des références sont données en fin de document.  

La lecture de ces fils de discussion est plus que recommandée pour la mise en oeuvre :  
Forum photovoltaïque, discussion sur EcoPV : https://forum-photovoltaique.fr/viewtopic.php?f=110&t=42721  
Forum photovoltaïque, réalisation d'un PCB : https://forum-photovoltaique.fr/viewtopic.php?f=110&t=42874  
Forum photovoltaïque, montage du PCB : https://forum-photovoltaique.fr/viewtopic.php?f=110&t=43197  

## Mise en garde  
Les programmes et les schémas proposés ont une vocation informative et pédagogique. Ils ont été testés avec succès par les auteurs. Cependant les auteurs de ces programmes et de ces schémas déclinent toute responsabilité. Les auteurs ne pourraient être tenus pour responsables du fonctionnement et des conséquences de l'utilisation des programmes et des schémas mis à disposition.  
Intervenir sur des circuits électriques est dangereux et nécessite le recours à une personne qualifiée et le respect strict des normes de sécurité et de protection en vigueur.

## Fonctionnement  
EcoPV mesure en permanence la puissance consommée par la maison. Lorsque la production photovoltaïque dépasse la consommation, cela est immédiatement détecté par EcoPV. EcoPV pilote alors un variateur électronique qui va alimenter finement une résistance électrique (chauffe-eau) pour équilibrer puissance consommée et puissance produite. Ainsi, l'excédent de production photovoltaïque est dirigé vers la résistance du chauffe-eau et valorisé en chaleur au lieu d'être cédé au réseau électrique extérieur.  
  
Techniquement, EcoPV est basé sur :  
* un échantillonnage rapide de la puissance consommée (8300 fois par seconde),  
* une régulation proportionnelle-intégrale de la charge résistive,  
* une programmation du régulateur entièrement gérée par les interruptions de l'ATMega328.  
  
## Specifications de EcoPV  
* Mesure de la puissance consommée 8300 fois par seconde.  
* Régulation proportionnelle intégrale calculée toutes les 10 ms pour le pilotage de la charge résistive.  
* Calcul de Vrms, Irms, Pactive, Papparente, cos phi, Proutée, énergie importée, énergie exportée, énergie routée.  
* Relais secondaire optionnel à activation/désactivation sur seuils paramétrables.  
* Interface utilisateur interactive sur terminal par liaison série pour :  
  * Affichage des statistiques de fonctionnement toutes les secondes,  
  * Paramétrage et calibrage du système.  
* Sauvegarde des paramètres et des compteurs d'énergie en mémoire non volatile (EEPROM).  
* Horloge et scheduler internes pour la programmation de tâches planifiées.   
* Affichage sur écran optionnel (option de compilation).  
* Communication MYSENSORS optionnelle (option de compilation).  
* Communication ETHERNET optionnelle (option de compilation). API HTTP de récupération des données.  
* Auto-contrôle du fonctionnement et visualisation du statut par mot d'état.  
* Entrée pour comptage d'impulsions. 
  
## La partie matérielle  
EcoPV nécessite plusieurs choses pour fonctionner :  
* **Une carte Arduino** basée sur un ATMega 328 5V 16 MHz de type Arduino Nano. C'est le coeur du système qui exécute le programme EcoPV.ino.  
![Arduino Nano](devices/ArduinoNano.jpg)  
* **Un circuit de mesure de la tension et du courant consommé par la maison** :  
![EcoPV Analog circuit](schematics/EcoPV_analog.png)  
C'est un circuit électronique analogique qui convertit la tension et le courant dans des valeurs acceptables par l'Arduino. Il utilise un transformateur pour mesurer la tension du secteur et une pince ampérémétrique pour mesurer le courant. Celle-ci devra être placée au niveau du fil de phase qui alimente la maison en aval du disjoncteur principal. Un transformateur Hahn BVEI3063359 230V/6V et une pince ampéremétrique YHDC SCT-013-000 100A/50mA ont été employés pour le montage et donnent des résultats très satisfaisants.  
* **Un relais électronique SSR (Solid State Relay)** de type *non zero crossing* capable d'être piloté par une tension de 5V. Ce relais électronique commandera la résistance électrique (chauffe-eau).  
  
![Solid State Relay](devices/solid-state-relay-random-firing.jpg)  
  
De manière optionnelle, EcoPV peut être équipé de :  
* **2 LEDs signalant le fonctionnement.**  
* **Un relais secondaire de délestage.** Il permet le pilotage d'un appareil électrique en "tout ou rien" en fonction de seuils paramétrables de mise en marche et d'arrêt.  
* **Un écran 128x64.** Il permet l'affichage des données essentielles pour le contrôle des puissances mesurées et routées. Il s'agit d'un écran oled à base de puce SSD1306 pour liaison I2C.  
* **Un système de communication.** Les données sont alors transmises :
  * soit sans fil selon le protocole MYSENSORS, ce qui nécessite un module radio NRF24L01,
  * soit par câble ethernet grâce à une API HTTP, ce qui nécessite un shield ENC28J60 pour Arduino.  
  
Le schéma général de branchement est le suivant :  
![EcoPV overview](schematics/EcoPV_arduinoNano.png)  
   
**Note concernant les pins d'entrées/sorties :** Les pins analogiques et digitales sont largement configurables, toutefois des restrictions existent concernant des fonctions spécifiques à certaines pins, qui peuvent dépendre des options de compilations sélectionnées. Se référer aux commentaires du code pour plus de détails. Le tableau ci-dessous résume les associations possibles :  

![EcoPV overview](schematics/pinTable.png)  

Les pins synchroACPin (D3) et synchroOutpuPin sont reliées ensemble sur la carte électronique afin de profiter de la fonction d'auto-détection du passage par zéro de la tension secteur.  

*Nouveau !!* La pin d'entrée D2, en configuration *pull-up*, permet le comptage des impulsions d'un système externe, par exemple les impulsions générées par un compteur de production PV. La source des impulsions doit se comporter comme un contact passif qui s'ouvre et qui se ferme : contact sec de type relais ou sortie de type collecteur ouvert. Ce type d'impulsion est généré typiquement par la sortie 'Impulsions' d'un compteur d'énergie modulaire d'un tableau électrique. Si il y a une polarisation à respecter au niveau du compteur modulaire, il faut connecter le - à GND et le + à D2.  

## La programmation de l'Arduino  
EcoPV nécessite l'installation de l'IDE Arduino disponible sur le site Arduino. Voir www.arduino.cc  
Pour l'utilisation de l'écran oled, la bibliothèque SSD1306Ascii devra être installée via la gestion des bibliothèques de l'IDE Arduino.  
Pour l'utilisation de la communication MYSENSORS, les bibliothèques correspondantes devront être installées. Elles sont disponibles via la gestion des bibliothèques de l'IDE Arduino. Voir www.mysensors.org  
Pour l'utilisation de la communication Ethernet, les 2 bibliothèques EtherShield et ETHER_28J60 devront être installées **manuellement**. Elles sont disponibles dans le répertoire libraries.  
  
Dans l'IDE de l'Arduino, ouvrir le programme EcoPV.ino.  
Choisir au début du code les options de compilation souhaitées.  
Compiler le programme et le télécharger dans la carte Arduino.  
Le programme démarrera automatiquement avec des paramètres par défaut.  
L'accès à l'interface utilisateur par liaison série se fait par défaut à la vitesse de 500 000 bauds. 
  
Voici un exemple d'écran disponible par le terminal série (d'autres exemples dans le répertoire screenshots) :  
![EcoPV Screenshot Statistiques](screenshots/Statistiques.png)
*Note : en absence du circuit analogique fonctionnel, le programme ne détectera pas la synchronisation secteur et entrera dans un mode d'erreur.*  
  
## Exemple de réalisation pratique  

Voici la réalisation de la partie analogique sur une plaquette d'expérimentation. Le transformateur de tension occupe une place importante. Il est généreusement dimensionné pour être bien linéaire et améliorer l'acquisition de la tension :

![EcoPV analog_PCB](devices/analogPCB.JPG)

Et l'ensemble du montage, incluant l'alimentation, l'Arduino Nano avec un shield ethernet, le SSR et son radiateur, les protection par fusible, un interrupteur, un bouton reset, les LEDs et également un circuit ESP8266 programmé pour faire le lien entre la liaosn série de l'Arduino et le wifi :

![EcoPV System](devices/System.JPG)

## Calibrage et paramétrage  
Pour un fonctionnement optimal, EcoPV nécessite le calibrage d'un certain nombre de paramètres en fonction de votre installation et des particularités des composants du circuit élecronique analogique. Ce calibrage des paramètres doit être fait une fois que votre réalisation est terminée, prête à être installée. Réalisez le calibrage dans l'ordre des étapes ci-dessous !

**Note 1 concernant le logiciel :** Le logiciel EcoPV est configuré avec des paramètres par défaut. A la première exécution du programme, ces paramètres par défaut seront stockés dans la mémoire non volatile de l'Arduino. Par la suite, l'ajustement des paramètres se réalise grâce à l'interface de configuration et leur sauvegarde s'effectue par l'option 13 du menu. Si vous souhaitez revenir aux paramètres par défaut, formatez l'EEPROM (option 82) et réalisez un reset matériel de l'Arduino en appuyant sur le bouton reset. EcoPV redémarrera alors avec les paramètres par défaut. 

**Note 2 concernant la pince ampéremétrique :** La pince ampéremétrique ou transformateur de courant se clipse autour du fil de phase. Lorsque la pince se referme, il est important que les 2 demi-tores de ferrite se positionnent correctement et soient bien alignés l'un vis-à-vis de l'autre. Un mauvais positionnement provoque une dégradation de la qualité de la mesure. 

* **Facteur de calibrage de la tension (V_CALIB).** Ce paramètre permet d'obtenir un affichage juste de la tension secteur (Vrms). Ce n'est pas un paramètre critique. Pour assurer un bon fonctionnement, il est recommandé de conserver la valeur par défaut (0.8) et d'ajuster le potentiomètre de la carte électronique pour mettre en accord la tension Vrms affichée et la tension du secteur réelle, mesurée à l'aide d'un voltmètre. Pour information, la valeur 230 V du secteur peut varier de +/- 10 %. 

* **Facteur de calibrage de la puissance (P_CALIB).** Ce paramètre permet d'obtenir un affichage juste des puissances. C'est un paramètre important à ajuster avec soin. Le calibrage va utiliser la lecture de votre compteur électrique EDF qui doit être de type électronique. Clipsez la pince ampéremétrique autour du fil de phase qui alimente votre maison juste en aval du disjonteur principal. Comparez la valeur Pappar donnée par EcoPV à la valeur de puissance en VA mesurée par votre compteur électrique. Si les valeurs différent, ajustez le paramètre P_CALIB pour que EcoPV donne une valeur de puissance identique à celle de votre compteur électrique à 10 VA près. Pour plus de précision, allumez quelques appareils électriques pour faire le réglage pour une puissance d'environ 2000 VA. P_CALIB devrait être de l'ordre de 0.1, et vous serez amené à régler la valeur à 3 chiffres après la virgule. Augmentez P_CALIB si EcoPV donne une valeur de Pappar en VA plus petites que celle du compteur, diminuez la dans le cas contraire. 

* **Décalage de puissance active (W) (P_OFFSET).** C'est un réglage important qui permet d'éliminer les décalages résiduels dans la mesure de la puissance active Pactive. Branchez la pince sur le montage EcoPV mais ne faites passer **aucun fil à l'intérieur du tore de mesure**. Le tore de la pince doit être fermé en position clipsée. Regardez la valeur Pactive affichée par EcoPV. Si la valeur est différente de 0, modifiez le paramètre P_OFFSET. Recommencez la procédure jusqu'à trouver la valeur de P_OFFSET qui permet d'obtenir une valeur de Pactive égale à 0 W à plus ou moind 2 W. Typiquement, P_OFFSET aura une valeur comprise entre -15 W et +15W. 

* **Facteur de calibrage de la phase (PHASE_CALIB).** Il s'agit du réglage le plus important à réaliser pour le bon fonctionnement de EcoPV ! Pour faire ce réglage, clipsez la pince ampéremétrique sur le fil de phase d'un appareil électrique constitué d'une résistance électrique pure : chauffe-eau, radiateur, bouilloire... d'une puissance comprise entre 1500 et 3000 W. Mettez en marche l'appareil. La valeur Pactive affiche alors la puissance active en W correspondant à la puissance de l'appareil. Le réglage de PHASE_CALIB consiste à maximiser la valeur de Cos phi, ce qui minimine Sin phi. Vous aurez plus de précision en utilisant Sin phi. Faites varier PHASE_CALIB en l'augmentant ou en le diminuant pour obtenir la plus petite valeur possible de Sin phi. Sin phi prendra une valeur très proche de 0. Ajustez PHASE_CALIB pour obtenir la plus petite valeur possible de sin phi à 3 chiffres après la virgule. Lorsque vous avez trouvé la valeur PHASE_CALIB qui donne une valeur de sin phi minimale, vous avez atteint le bon réglage. Dans ces conditions, Cos phi sera égal à 1 ou très proche de 1. 

* **Puissance de la résistance commandée (W) (P_RESISTANCE).** Indiquez ici la puissance de la résistance électrique en Watts qui sera branchée et pilotée par EcoPV : chauffe-eau, radiateur électrique. Il est conseillé que la puissance de cette résistance soit légèrement supérieure au surplus maximal que pourrait générer votre installation photovoltaïque. 

* **Consigne de régulation (W) (P_MARGIN).** C'est la cible de puissance importée du réseau EDF que cherchera à atteindre la régulation EcoPV lorsque vous serez en situation de surplus de production photovoltaïque. Idéalement on aimerait pouvoir avoir 0 W, mais en pratique il est prudent de conserver une marge de sécurité. 15 W est une valeur standard qui fonctionnera bien si les étapes précédentes de calibrage ont été effectuées avec soin. **Note :** si vous êtes dans une situation d'autoconsommation avec vente du surplus, vous pouvez indiquez ici une valeur négative. Cette valeur négative correspondra à une valeur de surplus maximal en Watts que vous autorisez pour la vente. Exemple : si vous indiquez -500, cela voudra dire qu'au maximum 500 W de votre production sera destinée à la vente. 

* **Gain proportionnel de régulation (GAIN_P).** C'est un paramètre essentiel du régulateur EcoPV dont dépend la qualité de régulation. Il correspond au gain proportionnel du régulateur de EcoPV. C'est une valeur entière. Une valeur standard de GAIN_P se calcule ainsi : 150000 * P_CALIB / P_RESISTANCE. Typiquement GAIN_P = 5 pour une résistance de charge de 3000 W. 

* **Gain intégral de régulation (GAIN_I).** C'est un paramètre essentiel du régulateur EcoPV dont dépend la qualité de régulation. Il correspond au gain intégral du régulateur de EcoPV. C'est une valeur entière. Une valeur standard de GAIN_I se calcule ainsi : 900000 * P_CALIB / P_RESISTANCE. Typiquement GAIN_I = 30 pour une résistance de charge de 3000 W. 

* **Tolérance de régulation (J) (E_RESERVE).** C'est un paramètre du régulateur qui intervient lorsque le surplus de production est très faible. Valeur à indiquer : 5. 

* **Excédent de production pour relais ON (W) (P_DIV2_ACTIVE).** Ce réglage et les suivants ne sont nécessaires que si vous utilisez le relais secondaire de délestage. P_DIV2_ACTIVE correspond à la valeur en Watts du surplus de production photovoltaïque à partir duquel sera enclenchée la charge secondaire qui consommera votre surplus (pompe de piscine...). 

* **Importation minimale pour relais OFF (W) (P_DIV2_IDLE).** P_DIV2_IDLE correspond à la valeur de la puissance active importée du réseau EDF qui provoquera l'arrêt de la charge secondaire. **Note :** Afin que le pilotage de la charge secondaire de délestage soit stable, il faut respecter la condition : ( P_DIV2_ACTIVE + P_DIV2_IDLE ) supérieur à la valeur de la charge secondaire connectée en Watts. 

* **Relais : durée minimale ON (min) (T_DIV2_ON).** Durée minimale de fonctionnement de la charge secondaire en minutes une fois enclenchée. Il s'agit d'éviter des cycles marche/arrêt trop fréquents de la charge secondaire, en particulier s'il s'agit d'une pompe. Valeur typique : 5 minutes. 

* **Relais : durée minimale OFF (min) (T_DIV2_OFF).** Durée minimale de repos de la charge secondaire en minutes une fois stoppée. Il s'agit d'éviter des cycles marche/arrêt trop fréquents de la charge secondaire, en particulier s'il s'agit d'une pompe. Valeur typique : 5 minutes. 

* **Relais : constante de lissage (min) (T_DIV2_TC).** C'est un réglage qui permet de filtrer la valeur des puissances prises en compte pour le déclenchement ou l'arrêt de la charge secondaire. L'objectif est d'éviter la mise en route ou l'arrêt intempestif de la charge secondaire qui serait lié à une fluctuation transitoire de la production photovoltaïque ou de la consommation (passage nuageux, démarrage d'un réfrigérateur...). T_DIV2_TC est une constante de temps qui s'exprime en minutes. Valeur standard : 1 minute. Note : les valeurs filtrées de Pactive et Proutée sont indiquées sur l'affichage. 

**Note 3 : Sauvegarde des paramètres.** N'oubliez pas de sauvegarder les paramètres modifiés par l'option 13 du menu, sinon ils seront perdus au prochain démarrage. 

**Note 4 :** Certains paramètres nécessitent un redémarrage de EcoPV pour être définitivement pris en compte par toutes les fonctions du programme. Ce redémarrage s'effectue par l'option 99 du menu ou en provoquant un reset matériel du routeur. Les paramètres concernés sont : P_OFFSET, P_MARGIN, E_RESERVE et tous les paramètres relatifs au relais secondaire de délestage. 

**Note 5 :** A la première utilisation, une mise à zéro des index est nécessaire. Cela se réalise par l'option 23 du menu.  

## Indication de fonctionnement par les LEDs  
Deux LEDs indiquent le fonctionnement de EcoPV. Il s'agit de la LED de statut, branchée du D6, et de la LED de routage, branchée sur D7. 

* **LED de statut.** Clignotement lent : fonctionnement normal. Clignotement rapide : anomalie furtive d'acquisition ou de régulation. Allumée fixe : anomalie grave d'acquisition ou de régulation. 

* **LED de routage.** Eteinte : pas de routage. Allumée fixe : routage de puissance régulé. Clignotement : routage maximal / exportation de puissance. 

Lorsque les 2 LEDs clignotent très rapidement en alternance : anomalie sévère du système, redémarrage automatique dans une minute. 

## Ecran oled    
De manière optionnelle, un écran oled peut être connecté à EcoPV. Il s'agit d'un écran oled 128x64 pixels, muni d'une puce SSD1306 et communiquant via le port I2C. Au niveau du programme, l'option de compilation OLED_128X64 doit être activée et l'écran se branche sur l'alimentation électrique et les pins A4 (SDA) et A5 (SCK ou SCL). La bibliothèque SSD1306Ascii devra être installée via le gestionnaire de bibliothèque de l'IDE Arduino.  
Par défaut, l'adresse I2C de l'écran est configurée à 0x3C. Au besoin, si votre écran dispose d'une autre adresse, celle-ci es modifiable dans le code au niveau de la ligne :  
* #define I2C_ADDRESS 0x3C  
    // adresse I2C de l'écran oled  

**Note :** Si vous activez l'option OLED_128X64 et que l'écran est absent, mal connecté, ou que son adresse I2C est mal configurée, alors le programme EcoPV sera bloqué et le routeur ne démarrera pas.  

![EcranOled](devices/maquette_ecran.png)  

## Communication Ethernet  
De manière optionnelle, EcoPV peut se connecter à votre réseau local (LAN) câblé en RJ45. Celà permet d'avoir accès à une API HTTP. Des requêtes http permettent alors de récupérer les informations de fonctionnement de EcoPV. Pour mettre en oeuvre la communication Ethernet, il suffit d'enficher l'Arduino Nano sur un shield ethernet RJ45 à base de puce ENC28J60 comme celui ci-dessous :  

![EthernetShield](devices/EthernetShield.jpg)  

Au niveau du programme, l'option de compilation ETHERNET_28J60 devra être activée au début du code et les 2 bibliothèques EtherShield et ETHER_28J60 devront être installées **manuellement** dans l'IDE Arduino. Elles sont disponibles dans le répertoire libraries de ce dépôt.  
Par défaut, EcoPV a une adresse IP statique : 192.168.2.250 et le port est 80. Vous pouvez modifier ces valeurs dans le programme pour correspondre à votre réseau LAN :  
* byte ethIp [4] = { 192, 168, 1, 250 };  
    // Adresse IP correspondant à une adresse libre sur le réseau local : 192.168.1.250  
* unsigned int ethPort = 80;  
    // Port IP pour l'accès aux requêtes HTTP : 80  

L'utilisation de l'API peut se faire simplement par requête http à l'aide de n'importe quel navigateur, ou alors dans des scripts sous différents langages en utilisant curl par exemple.  
Une requête du type http://adresseIP:port/GetXX renverra une réponse au format json : {"value":"xxxxx"}. 
Par exemple http://192.168.1.250:80/Get01 renverra {"value":"230"} car Get 01 correspond à la lecture de la tension secteur. 

La liste des commandes disponible actuellement est GetXX où : 
* XX = 01 : Vrms (V)
* XX = 02 : Irms (A)
* XX = 03 : Pact (W)
* XX = 04 : Papp (VA)
* XX = 05 : Prouted (W)
* XX = 06 : Pimp (W)
* XX = 07 : Pexp (W)
* XX = 08 : cosinus phi * **1 000**
* XX = 09 : index d'énergie routée (kWh) (estimation)
* XX = 10 : index d'énergie importée (kWh)
* XX = 11 : index d'énergie exportée (kWh)
* XX = 12 : index du compteur d'impulsions
* XX = 20 : bits d'erreur et de statut (byte) en décimal
* XX = 21 : temps de fonctionnement ddd:hh:mm:ss
* XX = 90 : mise à 0 des 3 index d'énergie (réponse : "ok")
* XX = 91 : enregistrement des 4 index d'énergie (réponse : "ok")
* XX = 92 : redémarrage du routeur (réponse : "ok")
* XX = 93 : formatage EEPROM (réponse : "ok")
* XX = 94 : sauvegarde des paramètres en EEPROM (réponse : "ok")
* XX = 99 : version logicielle 

ParXX permet de lire les param§tres de calibrage : 
* XX = 01 : V_CALIB * **1 000 000**
* XX = 02 : P_CALIB * **1 000 000**
* XX = 03 : PHASE_CALIB
* XX = 04 : P_OFFSET
* XX = 05 : P_RESISTANCE
* XX = 06 : P_MARGIN
* XX = 07 : GAIN_P
* XX = 08 : GAIN_I
* XX = 09 : E_RESERVE
* XX = 10 : P_DIV2_ACTIVE
* XX = 11 : P_DIV2_IDLE
* XX = 12 : T_DIV2_ON
* XX = 13 : T_DIV2_OFF
* XX = 14 : T_DIV2_TC 

Et aussi SetXX=yyyy.zzz qui permet de changer les paramètres et d'écrire la valeur yyyy.zzz pour le paramètre XX :  
* XX = 01 : V_CALIB
* XX = 02 : P_CALIB
* XX = 03 : PHASE_CALIB
* XX = 04 : P_OFFSET
* XX = 05 : P_RESISTANCE
* XX = 06 : P_MARGIN
* XX = 07 : GAIN_P
* XX = 08 : GAIN_I
* XX = 09 : E_RESERVE
* XX = 10 : P_DIV2_ACTIVE
* XX = 11 : P_DIV2_IDLE
* XX = 12 : T_DIV2_ON
* XX = 13 : T_DIV2_OFF
* XX = 14 : T_DIV2_TC 

Une réponse au format json : {"value":"ok"} est donnée si succès de l'opération.

## Communication MySensors  
De manière optionnelle, EcoPV peut se connecter à votre système IoT MySensors comme un noeud de capteur. Celà permet de récupérer les informations de fonctionnement de EcoPV sur un système de domotique via une gateway MySensors. Pour mettre en oeuvre la fonctionnalité MySensors, il suffit de connecter un module radio NRF 2.4 GHz comme celui ci-dessous :  

![NRF24](devices/NRF24L01L-Long-Range.jpg)  

Au niveau du programme, l'option de compilation MYSENSORS_COM devra être activée au début du code et la bibliothèque MySensors devra être installée via le gestionnaire de bibliothèque de l'IDE Arduino. La connexion du module radio à l'Arduino Nano est décrite sur la site MySensors : www.mysensors.org  
Par défaut, le numéro du noeud de capteur créé par EcoPV est 30. Il possède deux capteurs : 
* Capteur 0 : power meter qui renseigne sur :
    * V_WATT : puissance active
    * V_VA : puissance apparente
    * V_POWER_FACTOR : cosinus phi
    * V_KWH : index d'énergie routée
    * V_VAR1 : puissance importée
    * V_VAR2 : puissance exportée
    * V_VAR3 : puissance routée
    * V_VAR4 : byte d'erreur / statut
* Capteur 1 : multimeter qui renseigne sur :
    * V_VOLTAGE : tension
    * V_CURRENT : courant  

Vous pouvez modifier certains paramètres de configuration du noeud de capteur dans le code :  
* #define MY_NODE_ID                 30  
// Adresse du noeud de capteurs MYSENSORS
* #define CHILD_ID_POWER             0  
// capteur 0 = Power meter
* #define CHILD_ID_MULTIMETER        1  
// capteur 1 = Multimètre
* #define MYSENSORS_TRANSMIT_PERIOD 20  
    // Période de transmission des données de EcoPV vers la gateway en secondes  
    // valeurs possibles pour une transmission régulière :  
    // 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60 
  
## Déport de la liaison série de l'Arduino en Wifi  
La liaison série de l'Arduino Nano permet le monitoring du fonctionnement de EcoPV et également la modification des paramètres pour le calibrage. En fonction de l'accessibilité de l'Arduino, il peut être pratique de déporter la liaison série au travers du Wifi. Pour cela, il faut connecter un composant Wemos ou ESP8266 sur la liaison série de l'arduino (D0, D1, GND) en prenant garde à l'adaptation des niveaux logiques (5V pour l'Arduino, généralement 3.3V pour l'ESP8266). Le programme SerialBridge.ino doit être chargé dans le Wemos/ESP8266 à l'aide de l'IDE Arduino. De nombreux sites Web expliquent comment utiliser l'IDE de l'Arduino pour programmer un Wemos/ESP8266. Une fois le montage réalisé et les composants programmés, un réseau Wifi nommé EcoPV sera disponible. Pour s'y connecter, le mot de passe 123456789 est nécessaire. Une fois connecté, la liaison série de l'Arduino Nano est disponible en telnet à l'adresse 192.168.4.1 sur le port 23. Par exemple, vous y accédez sur MacOS dans un terminal en tapant :  
nc 192.168.4.1 23  
Vous pourrez alors avoir accès au monitoring de fonctionnement de EcoPV et aux réglages des paramètres.  

**Variante :** le programe serialBridgePortal.ino pour Wemos/ESP8266 est une variante au programme précédent. Ce programme crée d'abord un réseau Wifi avec un portail captif qui vous invite à vous connecter au réseau Wifi de votre box. Une fois cette étape réalisée, le Wemos/ESP8266 se connectera automatiquement à votre réseau Wifi et votre box lui attribuera une adresse IP dynamique par DHCP. Ainsi, pour pourrez accéder à la console série (monitoring, paramétrage), à partir de votre réseau local, ce qui vous permettra d'utiliser un PC relié par câble à ce réseau.  

## Sources et liens  
Forum photovoltaïque, discussion sur EcoPV : https://forum-photovoltaique.fr/viewtopic.php?f=110&t=42721  
Forum photovoltaïque, travaux de tignous84 et rolrider - PV Routeur : https://forum-photovoltaique.fr/viewforum.php?f=110  
Site Openenergy monitor : https://openenergymonitor.org  
PV Routeur pour les nuls : http://pvrouteur.free.fr 

*A compléter...*  
  
