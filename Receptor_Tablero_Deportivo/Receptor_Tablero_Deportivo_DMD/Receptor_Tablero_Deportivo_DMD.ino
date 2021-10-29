//Libreria SPI para manejo de DMD
#include <SPI.h>

//Libreria Software Serial
//#include <SoftwareSerial.h>

//Definimos los pines del puerto serial
//SoftwareSerial arduino(14,15); //Rx, Tx

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

//Array con la data recibida por medio de I2C
char datos[10];

//Array con la data obtenida del control remoto
char datos_remote[6];

//Bandera de comandos recibidos.
boolean flag = false;

//Bandera de juego seleccionado
boolean juego = false;

//Banderas de control de juego (arranque, pausa y parada)
boolean control[3] = {false,false,false};

//Valores del juego
int maxTime, maxPeriods, maxExtra;

//Datos de tiempo
int minutos, segundos, segundero = 60, minGame, segGame;

//Indicador LED de recepcion Inalambrica
#define PIN_LED 4

//Indicador de transmision correcta de datos por I2C
#define PIN_FLAG 5

// Arranque inicial del sistema
void setup() {
  //Inicializamos el puerto serie
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto serial emulado
  //  arduino.begin(9600);
  //  Serial.println("Iniciando serial emulado...");

  //Inicializamos el puerto I2C como esclavo en el canal 1
  //  Serial.println("Iniciando el puerto I2C como esclavo en el canal 1...");
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
  if(juego) {
    controlGame();
    if(control[0]){
      if(segundero != segundos){
        cronometer();
          
      }
      
    }  
    
  }
}

//Interrupcion al recibir datos del maestro
void receiveEvent(int bytes) {
  Serial.print("Los datos recibidos son ");
  for (int x = 0; x < bytes; x++) {
    if (Wire.available() > 0) {
      datos[x] = (char)Wire.read();
      Serial.print(datos[x]);
      delay(5);
    }
  }
  Serial.print(" siendo un total de ");
  Serial.print(bytes);
  Serial.println(" bytes.");
  tiempo();
  data();
}

void tiempo() {
  minutos = (charToNumber(datos[0]) * 10) + charToNumber(datos[1]);
  segundos = (charToNumber(datos[2]) * 10) + charToNumber(datos[3]);
  Serial.print("Tiempo (mm:ss): ");
  Serial.print(minutos);
  Serial.print(':');
  Serial.print(segundos);
}

void data() {
  for (int x = 4; x < sizeof(datos); x++) {
    if (datos[x] != 'N') {
      datos_remote[x - 4] = datos[x];
      if (x == 4) {
        Serial.print(" ----> Data recibida del control remoto: ");
      }
      Serial.print(datos[x]);
      flag = true;
    }
  }
  Serial.println();
  if (!juego && flag) {
    juego = gameSet(datos_remote[0]);
  }
}

boolean gameSet(char game) {
  switch (game) {
    case 's':
      configGame(20, 2, 5);
      return true;
    case 'b':
      configGame(10, 4, 5);
      return true;
    case 'o':
      configGame(0, 0, 0);
      return true;
    default:
      flag = false;
      return false;
  }
}

void controlGame(){
    switch(datos_remote[0]){
      case 'r':
        control[0] = true;
        control[1] = false;
        control[2] = false;
        break;
      case 'p':
        control[0] = false;
        control[1] = true;
        control[2] = false;
        break;
      case 's':
        control[0] = false;
        control[1] = false;
        control[2] = true;
        break;
      default:
        break;      
    }  
}

void configGame(int maxTimeGame, int maxPeriodsGame, int maxExtraTime) {
  maxTime = maxTimeGame;
  maxPeriods = maxPeriodsGame;
  maxExtra = maxExtraTime;
  minGame = 0;
  segGame = 0;
  segundero = segundos;
  //imprimir pantalla con valores iniciales
}

int charToNumber(char number) {
  switch (number) {
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    default:
      return 0;
      break;
  }
}

boolean cronometer() {
  if (segGame < 60) {
    segGame++;
  } else {
    if (minGame < maxTime) {
      minGame++;
      segGame = 0;
      return false;
    } else {
      minGame == maxTime;
      segGame = 0;
      return true;
    }
  }
}
