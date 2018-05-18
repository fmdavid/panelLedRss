#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include "WiFiEsp.h" //Libreria para comunicarse facilmente con el modulo ESP01
#include "SoftwareSerial.h"

// Matrices LEDs
//  Vcc - Vcc
//  Gnd - Gnd
//  Din - Mosi (Pin 11)
//  Cs  - SS (Pin 10)
//  Clk - Sck (Pin 13)

// ESP8266
//  RX Pin 7
//  TX Pin 6
//  Poner 3,3v con fuente externa a VCC, RST y a CH_PD
//  GND a GND
 
const int pinCS = 10;
const int numDisplayHorizontal = 4;
const int numDisplayVertical = 1;
const int pinRX = 7;
const int pinTX = 6;
 
Max72xxPanel matrix = Max72xxPanel(pinCS, numDisplayHorizontal, numDisplayVertical);
String texto[5] = {"","","","",""};  //texto a mostrar en el display
const int espera = 100; // En milisegundos
const int espaciador = 1;
const int anchura = 5 + espaciador; // La anchura de la fuente es 5 pixeles

char ssid[] = "AQUITUSSID";            // SSID (Nombre de la red WiFi)
char pass[] = "AQUITUPASS";        // Contraseña
int status = WL_IDLE_STATUS;     // Estado del ESP. No tocar

WiFiEspClient client;  //Iniciar el objeto para cliente 
SoftwareSerial esp8266(pinRX, pinTX);

// Leeremos el RSS de europapress
char host[] = "www.europapress.es";
 
void setup() {
   Serial.begin(9600); //Monitor serie
   setupMatrices();
   setupWifi();
   obtenerNoticias();
   finalizarConexion();
   //Serial.println(texto);
}

void setupMatrices(){
  matrix.setIntensity(7); // Un valor entre 0 y 15 para el brillo
 
   // Se ajusta según se necesite
   matrix.setPosition(0, 3, 0);
   matrix.setPosition(1, 2, 0);
   matrix.setPosition(2, 1, 0);
   matrix.setPosition(3, 0, 0);

   // Se ajusta la rotación
   matrix.setRotation(0, 0);
   matrix.setRotation(1, 0);
   matrix.setRotation(2, 0);
   matrix.setRotation(3, 0);

   matrix.fillScreen(HIGH);
}

void setupWifi(){
  esp8266.begin(9600); //ESP01

   WiFi.init(&esp8266);
   
   //intentar iniciar el modulo ESP
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Modulo no presente. Reinicie el Arduino y el ESP01 (Quite el cable que va de CH_PD a 3.3V y vuelvalo a colocar)");
    //Loop infinito
    while (true);
  }

  //Intentar conectar a la red wifi
  while ( status != WL_CONNECTED) {
    Serial.print("Intentando conectar a la red WiFi: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }
}

void finalizarConexion(){
  //Desconexion
  if (client.connected()) {
    Serial.println();
    Serial.println("Desconectando del servidor...");
    client.flush();
    client.stop();
  }
}

void obtenerNoticias(){
  Serial.println("Iniciando conexion..."); // Intentar la conexion al servidor dado
  if (client.connect(host, 80)) {
    Serial.println("Conectado al servidor");
    String url = "/rss/rss.aspx";
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    String lineaActual = "";
    bool estoyEnUnItem = false;
    int num = 0;
    while (client.available() && num <5) {
      char c = client.read();
      lineaActual += c;
      if (c == '\n') {
        if(lineaActual.indexOf("<item>") > 0){
          estoyEnUnItem = true;
        }else if(lineaActual.indexOf("<title>") > 0 && estoyEnUnItem){
          texto[num] = lineaActual.substring(lineaActual.indexOf("<title>") + 7, min(lineaActual.indexOf("</title>"),120));
          estoyEnUnItem = false;
          Serial.println(lineaActual);
          Serial.println(num);
          Serial.println(texto[num]);
          num++;
        }
      lineaActual = "";
      }  
    }
  }
}
 
void loop() {
   escribeMensaje(host);
   escribeMensaje(texto[0]);
   escribeMensaje(texto[1]);
   escribeMensaje(texto[2]);
   escribeMensaje(texto[3]);
   escribeMensaje(texto[4]);
}

void escribeMensaje(String mensaje){
  for (int i = 0; i < anchura * mensaje.length() + matrix.width() - 1 - espaciador; i++) {
      matrix.fillScreen(LOW);
      int letra = i / anchura;
      int x = (matrix.width() - 1) - i % anchura;
      int y = (matrix.height() - 8) / 2; // centra el texto verticalmente
      while (x + anchura - espaciador >= 0 && letra >= 0) {
         if (letra < mensaje.length()) {
            matrix.drawChar(x, y, mensaje[letra], HIGH, LOW, 1);
         }
         letra--;
         x -= anchura;
      }
      matrix.write(); // Envía los datos a la matriz
      delay(espera);
   }
}

