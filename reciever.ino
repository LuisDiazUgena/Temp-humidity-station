#include <LiquidCrystal.h>
#include <VirtualWire.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte my_ip[] = { 192, 168, 1, 177 }; // google will tell you: "public ip address"

#include "DHT.h"

//LCD
LiquidCrystal lcd(40, 38, 36, 34, 32, 30);
int pinBkl = 2;
boolean isOn = true;
float nextTime, intervale = 8000;

//DHT
float h = 0, t = 0, temp2;
#define DHTPIN 46
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Reciever
int data[5];   // variable used to store received data
int rx_pin = 22;
String str;

EthernetServer server(80);
char c;

//PIR
int pinPIR = A0;
float pirReading = 0;
int threeshold = 150;


void setup() {

  Serial.begin(9600);

  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_rx_pin(rx_pin);
  vw_setup(4000);  // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

  pinMode(pinPIR, INPUT);
  pinMode(pinBkl, OUTPUT);
  digitalWrite(pinBkl, HIGH);

  pir();

  lcd.begin(20, 4);
  lcd.print("Temp & Humedity");
  lcd.setCursor(0, 1);
  lcd.print("station");
  delay(1000);
  lcd.clear();

  Ethernet.begin(mac, my_ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  delay(1000);
  lcd.clear();
}

void loop() {

  EthernetClient client = server.available();
  pir();
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  h = dht.readHumidity();
  t = dht.readTemperature();

  if (vw_get_message(buf, &buflen)) {

    //Necesario para pagina web
    str = "";
    for (int i = 0; i < buflen; i++) {
      if (buf[i] != 46 ) {
        data[i] = ascii2chr(buf[i]);
        str += data[i];
      } else {
        str += '.';
      }
    }
  }



  print_lcd(h, t, str);

  //Parte web
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 10 sec
          client.println();

          //Send webpage
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          client.print(" <ul><li><a href="">Temperatura</a></li>" );
          client.print(" <li> <a href=""> Graficas </a> </li> </ul>");
          client.print("T1 = ");
          client.print(t);
          client.print("H1 = ");
          client.print(h);
          client.println(" <br/> ");
          client.print("T2 = ");
          client.print(str);
          client.println(" <br/> ");

          client.println(" </html> ");
          break;
        }

      }
      if ( c == '\n') {
        currentLineIsBlank = true;
      } else if ( c != '\r') {
        currentLineIsBlank = false;
      }
    }
  }  delay(10);
  client.stop();


}


void pir() {

  pirReading = analogRead(pinPIR);
      Serial.print("pir = ");
      Serial.println(pirReading);
  if ( millis() > nextTime ) {
    if (pirReading > threeshold && !isOn) {
      digitalWrite(pinBkl, HIGH);
      nextTime = millis() + intervale;
      isOn = true;
      Serial.println("pir > threes");

    } else {
      digitalWrite(pinBkl, LOW);
      isOn = false;
      Serial.println("pir < threes");
    }
  }

  Serial.print("is On= ");
  if (isOn == true) {
    Serial.println("true");
  }else{
        Serial.println("false");
  }
  
}


int ascii2chr(int data) {
  switch (data) {
    case 48:
      return 0;
    case 49:
      return 1;
    case 50:
      return 2;
    case 51:
      return 3;
    case 52:
      return 4;
    case 53:
      return 5;
    case 54:
      return 6;
    case 55:
      return 7;
    case 56:
      return 8;
    case 57:
      return 9;

  }
}

void print_lcd(float h, float t, String str) {
  lcd.setCursor(0, 0);
  lcd.print("T1 = ");
  lcd.setCursor(4, 0);
  lcd.print(t);
  lcd.setCursor(10, 0);
  lcd.print("H1 = ");
  lcd.setCursor(14, 0);
  lcd.print(h);
  lcd.setCursor(19, 0);
  lcd.print(" % ");
  lcd.setCursor(0, 1);
  lcd.print("T2 = ");
  lcd.setCursor(4, 1);
  lcd.print(str);
}

void startEthernet() {
  Serial.println("... Initializing ethernet");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("... Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, my_ip);
  }
  Serial.println("... Done initializing ethernet");
  delay(1000);
}
