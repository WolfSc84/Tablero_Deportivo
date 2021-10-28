//Libreria SPI para manejo de DMD
#include <SPI.h>

//Libreria Software Serial
#include <SoftwareSerial.h>

//Definimos los pines del puerto serial
SoftwareSerial arduino(14,15); //Rx, Tx

//Libreria I2C
#include <Wire.h>

// Librerias Funciones DMD
#include <DMD2.h>
#include "D:\Wolfgang Santamaria\Documents\Arduino\libraries\DMD2\fonts\Arial_black_16.h"

// Dimensiones del aviso Led
#define ROW 1
#define COLUMN 8

// Dimensiones del display
SoftDMD dmd(COLUMN, ROW);

// Dimensiones del panel utilizado en el display
DMD_TextBox box(dmd, 0, 0, 32, 16);

// Tipo de fuente a utilizar
#define FONT Arial_Black_16

//Vector con los datos a recibir
char datos[10];

//Contados de bytes leidos correctamente.
char count = 0;

//Indicador LED de recepcion Inalambrica
#define PIN_LED 4

//Indicador de transmision correcta de datos por I2C
#define PIN_FLAG 5

unsigned int val = 2;

// Arranque inicial del sistema
void setup() {
  //Inicializamos el puerto serie
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto serial emulado
  arduino.begin(9600);
  Serial.println("Iniciando serial emulado...");

  //Inicializamos el puerto I2C como esclavo en el canal 1
  Serial.println("Iniciando el puerto I2C como esclavo en el canal 1...");
  Wire.begin(1);

  //Creamos las interrupciones que se activaran en caso de una solicitud o recepcion de dato
  Wire.onReceive(receiveEvent);

  // Configuracion inicial de la pantalla
  dmd.setBrightness(255);
  dmd.selectFont(Arial_Black_16);
  dmd.begin();
  dmd.clearScreen();

  //Led indicador de datos recibidos
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  //Flag de transmision de datos
  pinMode(PIN_FLAG, INPUT);
}

//Ejecucion del loop principal
void loop() {
  delay(100);
}

//Interrupcion al recibir datos del maestro
void receiveEvent(int bytes) {
  Serial.print("Los datos recibidos son ");
  for (int x = 0; x < bytes; x++) {
    if (Wire.available() > 0) {
      datos[x] = (char)Wire.read();
      Serial.print(datos[x]);
      delay(10);
    }
  }
  Serial.print(" siendo un total de ");
  Serial.print(bytes);
  Serial.println(" bytes.");
  validateData();
}

void validateData(){
  Serial.print("Enviando los datos ");
  Serial.print(datos);
  Serial.println(" para comprobar lo que se recibio por el I2C, a traves el puerto serial emulado...");
  arduino.print(datos);
}
