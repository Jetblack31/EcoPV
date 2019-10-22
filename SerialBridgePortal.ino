/*
SerialBridge.ino - Wemos program to bridge serial communication to wifi,
Copyright (C) 2019 - Bernard Legrand

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
**        Ce programme fonctionne sur Wemos ou autre carte ESP8266                  **
**        La compilation s'effectue avec l'IDE Arduino                              **        
**        Un seul client est autorisé, toute nouvelle connexion à la liaison        **
**        déconnecte le client précédent                                            **        
**                                                                                  **
**************************************************************************************/

// Pour configurer :
// Se connecter au réseau WIFI créé par le Wemos : EcoPV par défaut
// Puis ouvrir un navigateur Web et configurer la connexion au réseau Wifi local
// Le Wemos redémarre et se connecte automatiquement au réseau local

// Découvrir l'adresse IP du Wemos dans votre box, par exemple 192.168.1.250
// Se connecter à EcoPV en telnet : telnet 192.168.1.250 ou nc 192.168.1.250 23


#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  

const char *ssid = "EcoPV";            // Nom du point d'accès portail
IPAddress ip ( 192, 168, 4, 1 );       // Adresse IP pour le portail
IPAddress netmask ( 255, 255, 255, 0 );

/*************************  PORT SERIE *******************************/
#define UART_BAUD0 500000           // Baudrate
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop
#define SERIAL0_TCP_PORT 23         // Wifi Port du bridge série

#define bufferSize 2048

WiFiServer server ( SERIAL0_TCP_PORT );
WiFiClient tcpClient;

char buf [bufferSize];
int i = 0;
unsigned long refTime;

void setup ( ) {

  delay (500);

  Serial.begin ( UART_BAUD0, SERIAL_PARAM0, SERIAL_FULL );

  WiFiManager wifiManager;
  wifiManager.setDebugOutput ( false );
  //wifiManager.resetSettings ( );
  wifiManager.setAPStaticIPConfig ( ip, ip, netmask );
  wifiManager.autoConnect ( ssid );

  server.begin ( );
  server.setNoDelay ( true );
}


void loop ( ) {

// DECOUVERTE D'UNE CONNEXION
  
  if ( server.hasClient ( ) ) tcpClient = server.available ( );

// WIFI TO LIAISON SERIE

  if ( tcpClient ) {
    while ( tcpClient.available ( ) ) {
      buf[i] = tcpClient.read ( );
      if ( i < bufferSize - 1 ) i++;
    }  
    Serial.write ( buf, i );
    i = 0;
  }

// LIAISON SERIE TO WIFI

  if ( Serial.available ( ) ) refTime = millis ( );
  while ( ( millis ( ) - refTime ) < 100 ) {
    if ( Serial.available ( ) ) {
      buf[i] = Serial.read ( );
      if ( i < bufferSize - 1 ) i++;
    }
  }
  if ( tcpClient ) tcpClient.write ( buf, i );
  i = 0;
}
