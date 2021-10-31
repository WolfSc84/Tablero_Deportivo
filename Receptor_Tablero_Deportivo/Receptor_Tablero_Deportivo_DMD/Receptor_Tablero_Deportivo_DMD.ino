//Libreria SPI para manejo de DMD
#include <SPI.h>

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

//Control de juego (Arranque, Pausa, Fin Periodo, Fin Extra, Fin Juego)
boolean control[5] = {false, false, false, false, false};

//Tipo de cronometro (Periodo, Complementario, Periodos Completados, Complementario Completados)
boolean runningTime[4] = {false, false, false, false};

//Control de puntajes y faltas (Puntaje Local,Falta Local,Puntaje Visitante,Falta Visitante)
int teams[4] = {0, 0, 0, 0};

//Control de tiempo complementario y periodos (Complementario Actual, Periodo Actual)
int controlTime[2] = {0, 0};

//Valores del juego (Tiempo maximo periodo Normal, Maximo numero de periodos, Maximo Tiempo de un omplementario, Maximo numero de Tiempos Complemenmtarios, Extra)
int configGame[5] = {0, 0, 0, 0, 0};

//Datos de tiempo (Minutos, Segundos, Minuto de juego, Segundo de Juego)
int timer[4] = {0, 0, 0, 0};

//Control de cambio en el tiempo
int segundero;

//Indicador LED de recepcion Inalambrica
#define PIN_LED 4

//Indicador de transmision correcta de datos por I2C
#define PIN_FLAG 5

// Arranque inicial del sistema
void setup() {
  //Inicializamos el puerto serie
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto I2C como esclavo en el canal 1
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
  if (juego) {
    game();
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
  timer[0] = (charToNumber(datos[0]) * 10) + charToNumber(datos[1]);
  timer[1] = (charToNumber(datos[2]) * 10) + charToNumber(datos[3]);
  Serial.print("Tiempo (mm:ss): ");
  Serial.print(timer[0]);
  Serial.print(':');
  Serial.print(timer[1]);
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
    } else {
      flag = false;
    }
  }
  Serial.println();
  if (!juego && flag) {
    juego = gameSet();
  }
  if (!runningTime[2] && !runningTime[3]) {
    period();
  }
  if (datos_remote[5] == 'Y') {
    resetBoard();
  }
}

boolean gameSet() {
  switch (datos_remote[1]) {
    case 's':
      setGame(20, 2, 5, 2);
      return true;
    case 'b':
      setGame(10, 4, 5, 0);
      return true;
    case 'o':
      setGame(0, 0, 0, 0);
      return true;
    default:
      return false;
  }
}

void controlGame() {
  switch (datos_remote[0]) {
    case 'r':
      control[0] = true;
      control[1] = false;
      control[4] = false;
      break;
    case 'p':
      control[0] = false;
      control[1] = true;
      control[4] = false;
      break;
    case 's':
      control[0] = false;
      control[1] = false;
      control[4] = true;
      break;
    default:
      break;
  }
}

void setGame(int maxTimeGame, int maxPeriodsGame, int maxExtraTime, int maxExtras) {
  configGame[0] = maxTimeGame;
  configGame[1] = maxPeriodsGame;
  configGame[2] = maxExtraTime;
  configGame[3] = maxExtras;
  timer[0] = 0;
  timer[1] = 0;
  teams[0] = 0;
  teams[1] = 0;
  teams[2] = 0;
  teams[3] = 0;
  controlTime[0] = 0;
  controlTime[1] = 1;
  runningTime[0] = true;
  runningTime[1] = false;
  runningTime[2] = false;
  runningTime[3] = false;
  segundero = timer[3];
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

void points() {
  switch (datos_remote[2]) {
    case 'L':
      teams[0]++;
      break;
    case 'l':
      if (teams[0] > 0) {
        teams[0]--;
      }
      break;
    case 'V':
      teams[2]++;
      break;
    case 'v':
      if (teams[2] > 0) {
        teams[2]--;
      }
      break;
    case 'X':
      teams[0] = 0;
      break;
    case 'Y':
      teams[2] = 0;
      break;
    default:
      break;
  }
}

void fouls() {
  switch (datos_remote[3]) {
    case 'L':
      teams[1]++;
      break;
    case 'l':
      if (teams[1] > 0) {
        teams[1]--;
      }
      break;
    case 'V':
      teams[3]++;
      break;
    case 'v':
      if (teams[3] > 0) {
        teams[3]--;
      }
      break;
    case 'X':
      teams[1] = 0;
      break;
    case 'Y':
      teams[3] = 0;
      break;
    default:
      break;
  }
}

void extra() {
  switch (datos_remote[4]) {
    case 'U':
      if (configGame[5] < configGame[0]) {
        configGame[5]++;
      }
      break;
    case 'D':
      if (configGame[5] > 0) {
        configGame[5]--;
      }
      break;
    case 'R':
      configGame[5] = 0;
      break;
    default:
      break;
  }
}

void period() {
  if (!control[0] && control[1]) {
    if (controlTime[1] && !controlTime[0]) {
      switch (datos_remote[4]) {
        case 'P':
          if (controlTime[1] < configGame[1]) {
            controlTime[1]++;
            timer[2] = 0;
            timer[3] = 0;
            control[2] = false;
            control[3] = false;
            control[4] = false;
          }
          break;
        case 'p':
          if (controlTime[1] > 0) {
            controlTime[1]--;
            timer[2] = 0;
            timer[3] = 0;
            control[2] = false;
            control[3] = false;
            control[4] = false;
          }
          break;
        case 'r':
          controlTime[1] = 0;
          timer[2] = 0;
          timer[3] = 0;
          control[2] = false;
          control[3] = false;
          control[4] = false;
          break;
        default:
          break;
      }
    } else if (!controlTime[1] && controlTime[0]) {
      switch (datos_remote[4]) {
        case 'P':
          if (controlTime[0] < configGame[3]) {
            controlTime[0]++;
            timer[2] = 0;
            timer[3] = 0;
            control[3] = false;
            control[4] = false;
          }
          break;
        case 'p':
          if (controlTime[0] > 0) {
            controlTime[0]--;
            timer[2] = 0;
            timer[3] = 0;
            control[3] = false;
            control[4] = false;
          }
          break;
        case 'r':
          controlTime[0] = 0;
          timer[2] = 0;
          timer[3] = 0;
          control[3] = false;
          control[4] = false;
          break;
        default:
          break;
      }
    }
  }
}

void cronometer() {
  if (runningTime[0] && !runningTime[2]) {
    if (timer[3] < 60) {
      timer[3]++;
    } else {
      if (timer[2] < (configGame[0] + configGame[5])) {
        timer[2]++;
        timer[3] = 0;
        return false;
      } else {
        timer[2] = (configGame[0] + configGame[5]);
        timer[3] = 0;
        control[2] = true;
        control[0] = false;
        if (control[2] && (controlTime[1] == configGame[1])) {
          runningTime[2] = true;
          runningTime[0] = false;
          runningTime[1] = true;
        }
      }
    }
  } else if (runningTime[1] && !runningTime[3]) {
    if (timer[3] < 60) {
      timer[3]++;
    } else {
      if (timer[2] < configGame[2]) {
        timer[2]++;
        timer[3] = 0;
        return false;
      } else {
        timer[2] = configGame[2];
        timer[3] = 0;
        control[3] = true;
        control[0] = false;
        if (control[3] && (controlTime[2] == configGame[3])) {
          runningTime[3] = true;
          runningTime[1] = false;
        }
      }
    }
  }
}

void resetBoard() {
  flag = false;
  juego = false;
  for (int x = 0; x < sizeof(control); x++) {
    control[x] = false;
    configGame[x] = 0;
    if (x < sizeof(runningTime)) {
      runningTime[x] = false;
      teams[x] = 0;
      timer[x] = 0;
      if (x < sizeof(controlTime)) {
        controlTime[x] = 0;
      }
    }
  }
}

void game() {
  controlGame();
  boolean valRun = control[0] && !control[1] && !control[2] && !control[3];
  if (valRun) {
    if (segundero != timer[1]) {
      cronometer();
    }
    if (flag) {
      extra();
      points();
      fouls();
    }
  }
}
