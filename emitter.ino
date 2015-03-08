#include <VirtualWire.h>

int pinTx = 8,pinPIR= A1;
//char *controller;

int pinTemp = A0;
float temp;

int pinLed=13;

//Variables para convertir float a char array
char controller[6];
String str;

void setup() {

  Serial.begin(9600);

  pinMode(pinLed, OUTPUT);
  pinMode(pinPIR, INPUT);

  vw_set_ptt_inverted(true); //
  vw_set_tx_pin(pinTx);
  vw_setup(4000);// speed of data transfer Kbps

}

void loop() {
  
  temp = temperature();
  str = String(temp);
  str.toCharArray(controller,6);
  /*
  //Debug
  Serial.print("controller value is: ");
  Serial.println(controller);
  */

  
  if (temp > 20 ){
    digitalWrite(pinLed, HIGH);
  }else{
    digitalWrite(pinLed, LOW);
  }
  vw_send((uint8_t *)controller, strlen(controller));
  vw_wait_tx(); // Wait until the whole message is gone

}

float temperature () {

  float Vout, aux = 0, acum = 0;
  float res = 0.010; // en V/ºC
  int samples = 100;

  for (int i = 0; i < samples; i++) {

    Vout = analogRead(pinTemp) * 0.00488758553275; // step * V/step = V
    aux = (Vout / res ); // V/(V/ºC) = ºC
    acum += aux;
    delay(10);
  }

  temp = acum / samples;

  return temp;
} //End - temperature
