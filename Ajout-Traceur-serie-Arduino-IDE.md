Nouvelle fonction : Traceur courbes sur Arduino IDE

Le but de ce "MOD" est d'ajouter une fonctionnalité permettant d'avoir une visualisation graphique des principales valeurs importantes du routeur EcoPV. 
Il est ainsi plus simple de visualiser l'évolution de la puissance mesurée, le retard de déclenchement  du relais statiques, ainsi que la puissance routée.

![EcoPV Traceur courbes](screenshots/ExempleTraceurSerie.png)

Pour activer cette nouvelle fonction (15) , une nouvelle variable a été ajoutée dans le menu configuration :

***        Configuration courante        ***
    1. Facteur de calibrage de la tension            0.687000
    2. Facteur de calibrage de la puissance            0.078000
    3. Facteur de calibrage de la phase            -12
    4. Décalage de puissance active (W)            -5
    5. Puissance de la résistance commandée (W)        160
    6. Consigne de régulation (W)                10
    7. Gain proportionnel de régulation            10
    8. Gain intégral de régulation                60
    9. Tolérance de régulation (J)                5
    10. Excédent de production pour relais ON (W)        9999
    11. Importation minimale pour relais OFF (W)        0
    12. Relais : durée minimale ON (min)            5
    13. Relais : durée minimale OFF (min)            5
    14. Relais : constante de lissage (min)            1
**    15. Activation traceur courbe=1 défaut=0         0 **

Par défaut l'affichage des statistiques classiques reste activé (valeur = 0), pour sérialiser les données et les rendre compatible avec le traceur série de l’Arduino IDE , il faut positionner cette valeur a 1.
Après le redémarrage du routeur, grâce au câble USB et de l'outil -> traceur série, vous pourrez visualiser ces courbes.

*Ps1 : l’intervalle de temps entre chaque échantillons peut être modifié dans le code source en modifiant : #define NB_CPTPERIODES   5 (exemple: 5 x 20ms = toutes les 100 ms )
*Ps2 : il peut être nécessaire de formater l'EEPROM pour obtenir cette nouvelle fonctionnalité. Dans ce cas n'oubliez pas de noter vos paramètres avec l'option 11 du menu.*

StephF13 , 25 Avril 2020
