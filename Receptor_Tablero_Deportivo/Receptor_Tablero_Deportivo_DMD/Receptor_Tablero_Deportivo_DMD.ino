//Libreria SPI para manejo de DMD
#include <SPI.h>

//Libreria I2C
#include <Wire.h>

// Librerias Funciones DMD
#include <DMD2.h>
#include "D:\Wolfgang Santamaria\Documents\Arduino\libraries\DMD2\fonts\Arial_black_16.h"

// Dimensiones del aviso Led
#define ROW 2
#define COLUMN 4

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

//Bandera de reseteo del tablero
boolean resetAll = true;

//Control de juego (Arranque, Pausa, Fin Periodo, Fin Extra, Fin Juego)
boolean control[5] = {false, false, false, false, false};

//Tipo de cronometro (Periodo, Complementario, Periodos Completados, Complementario Completados)
boolean runningTime[4] = {false, false, false, false};

//Control de puntajes y faltas (Puntaje Local,Falta Local,Puntaje Visitante,Falta Visitante)
int teams[4] = {0, 0, 0, 0};

//Control de tiempo complementario y periodos (Complementario Actual, Periodo Actual)
int controlTime[2] = {0, 0}, tempTime = 0;

//Valores del juego (Tiempo maximo periodo Normal, Maximo numero de periodos, Maximo Tiempo de un omplementario, Maximo numero de Tiempos Complemenmtarios, Extra)
int configGame[5] = {0, 0, 0, 0, 0};

//Datos de tiempo (Minutos, Segundos, Minuto de juego, Segundo de Juego)
int timer[4] = {0, 0, 0, 0};

//Control de cambio en el tiempo
int segundero;

//Estatus del juego de juego
#define GAME_STATUS 2

// Arranque inicial del sistema
void setup() {
  //Inicializamos el puerto serie
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto I2C como esclavo en el canal 1
  Wire.begin(1);

  //Creamos las interrupciones que se activaran en caso de una solicitud o recepcion de dato
  Wire.onReceive(receiveEvent);

  //Escaneamos los paneles
  scanDMD();

  //Seteamos el indicador de estatus del partido
  pinMode(GAME_STATUS, OUTPUT);
  digitalWrite(GAME_STATUS, LOW);

  // Configuracion inicial de la pantalla
  dmd.setBrightness(255);
  dmd.selectFont(Arial_Black_16);
  dmd.begin();
  dmd.clearScreen();
  printScreen();
}

//Ejecucion del loop principal
void loop() {
  delay(10);
  digitalWrite(GAME_STATUS, LOW);
}

//Interrupcion al recibir datos del maestro
void receiveEvent(int bytes) {
  printScreen();
  for (int x = 0; x < bytes; x++) {
    if (Wire.available() > 0) {
      datos[x] = (char)Wire.read();
    }
  }
  data();
  tiempo();
}

//Captura el tiempo recibido del reloj desde el maestro
void tiempo() {
  timer[0] = (charToNumber(datos[0]) * 10) + charToNumber(datos[1]);
  timer[1] = (charToNumber(datos[2]) * 10) + charToNumber(datos[3]);
}

//Captura la data proveniente del control remoto RF
void data() {
  for (int x = 4; x < sizeof(datos); x++) {
    if (datos[x] != 'N') {
      datos_remote[x - 4] = datos[x];
      Serial.print(datos[x]);
      flag = true;
    } else {
      flag = false;
    }
  }
  Serial.println();
  if (!flag) {
    if (juego && control[0]) {
      if (segundero != timer[1]) {
        cronometer();
        printScreen();
      }
    }
    if (runningTime[2] && runningTime[3]) {
      digitalWrite(GAME_STATUS, HIGH);
    }
  } else {
    if (datos_remote[5] == 'Y') {
      resetBoard();
    } else {
      if (!juego && resetAll) {
        juego = gameSelect();
        printScreen();
      } else {
        controlGame();
        if (control[0]) {
          extra();
          printScreen();
          fouls();
          printScreen();
        } else if (control[1] && (!runningTime[2] || !runningTime[3])) {
          period();
          printScreen();
        }
        points();
        printScreen();
      }
    }
  }
}

//Selecciona el tipo de deporte
boolean gameSelect() {
  switch (datos_remote[1]) {
    case 's':
      setGame(20, 2, 5, 2);
      return true;
    case 'b':
      setGame(10, 4, 5, 999);
      return true;
    case 'o':
      setGame(999, 999, 999, 999);
      return true;
    default:
      return false;
  }
}

//Controla el juego (Arrancar, Pausar o Finalizar)
void controlGame() {
  switch (datos_remote[0]) {
    case 'r':
      control[0] = true;
      control[1] = false;
      control[2] = false;
      control[4] = false;
      break;
    case 'p':
      control[0] = false;
      control[1] = true;
      control[2] = false;
      control[4] = false;
      break;
    case 'S':
      control[0] = false;
      control[1] = false;
      control[2] = true;
      control[4] = true;
      runningTime[2] = true;
      runningTime[3] = true;
      runningTime[0] = false;
      runningTime[1] = false;
      juego = false;
      break;
    default:
      break;
  }
}

//Configura el juego con los parametros iniciales
void setGame(int maxTimeGame, int maxPeriodsGame, int maxExtraTime, int maxExtras) {
  resetBoard();
  configGame[4] = 0;
  configGame[0] = maxTimeGame;
  configGame[1] = maxPeriodsGame;
  configGame[2] = maxExtraTime;
  configGame[3] = maxExtras;
  timer[2] = 0;
  timer[3] = 0;
  teams[0] = 0;
  teams[1] = 0;
  teams[2] = 0;
  teams[3] = 0;
  controlTime[0] = 0;
  controlTime[1] = 1;
  tempTime = 1;
  runningTime[0] = true;
  runningTime[1] = false;
  runningTime[2] = false;
  runningTime[3] = false;
  segundero = timer[3];
  resetAll = false;
  printScreen();
}

//Convierte caracteres numericos a numeros decimales
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

//Controla el puntaje de los equipos
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
  printScreen();
}

//Controla las faltas de los equipos
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
  printScreen();
}

//Controla el tiempo extra durante cada periodo de juego
void extra() {
  if (runningTime[0] && !runningTime[2]) {
    switch (datos_remote[4]) {
      case 'U':
        if (configGame[4] < configGame[0]) {
          configGame[4]++;
        }
        break;
      case 'D':
        if (configGame[4] > 0) {
          configGame[4]--;
        }
        break;
      case 'R':
        configGame[4] = 0;
        break;
      default:
        break;
    }
  }
  printScreen();
}

//Controla los periodos de juego
void period() {
  if (runningTime[0] && !runningTime[1]) {
    switch (datos_remote[4]) {
      case 'P':
        if (controlTime[1] < configGame[1]) {
          controlTime[1]++;
          tempTime = controlTime[1];
          timer[2] = 0;
          timer[3] = 0;
          configGame[4] = 0;
          control[2] = false;
          control[3] = false;
          control[4] = false;
        } else {
          runningTime[0] = false;
          runningTime[1] = true;
          runningTime[2] = true;
          controlTime[0]++;
          configGame[4] = 0;
          tempTime = controlTime[0];
        }
        break;
      case 'p':
        if (controlTime[1] > 1) {
          controlTime[1]--;
          tempTime = controlTime[1];
          timer[2] = 0;
          timer[3] = 0;
          configGame[4] = 0;
          control[2] = false;
          control[3] = false;
          control[4] = false;
        }
        break;
      case 'r':
        controlTime[1] = 0;
        tempTime = controlTime[1];
        timer[2] = 0;
        timer[3] = 0;
        configGame[4] = 0;
        control[2] = false;
        control[3] = false;
        control[4] = false;
        break;
      default:
        break;
    }
  } else if (runningTime[1] && !runningTime[0]) {
    switch (datos_remote[4]) {
      case 'P':
        if (controlTime[0] < configGame[3]) {
          controlTime[0]++;
          tempTime = controlTime[0];
          timer[2] = 0;
          timer[3] = 0;
          control[3] = false;
          control[4] = false;
        } else {
          runningTime[1] = false;
          runningTime[3] = true;
        }
        break;
      case 'p':
        if (controlTime[0] > 1) {
          controlTime[0]--;
          tempTime = controlTime[0];
          timer[2] = 0;
          timer[3] = 0;
          control[3] = false;
          control[4] = false;
        }
        break;
      case 'r':
        controlTime[0] = 0;
        tempTime = controlTime[0];
        timer[2] = 0;
        timer[3] = 0;
        control[3] = false;
        control[4] = false;
        break;
      default:
        break;
    }
  }
  printScreen();
}

//Cronometro de juego durante un periodo o tiempo complementario
void cronometer() {
  if (runningTime[0] && !runningTime[2]) {
    if (timer[3] < 59) {
      timer[3]++;
      segundero = timer[3];
    } else {
      if (timer[2] < (configGame[0] + configGame[4])) {
        timer[2]++;
        timer[3] = 0;
        segundero = timer[3];
      } else {
        digitalWrite(GAME_STATUS, HIGH);
        timer[2] = (configGame[0] + configGame[4]);
        timer[3] = 0;
        segundero = timer[3];
        control[2] = true;
        control[0] = false;
        control[1] = true;
        if (control[2] && (controlTime[1] == configGame[1])) {
          runningTime[2] = true;
          runningTime[0] = false;
          runningTime[1] = true;
        }
      }
    }
  } else if (runningTime[1] && !runningTime[3]) {
    if (timer[3] < 59) {
      timer[3]++;
      segundero = timer[3];
    } else {
      if (timer[2] < configGame[2]) {
        timer[2]++;
        timer[3] = 0;
        segundero = timer[3];
      } else {
        digitalWrite(GAME_STATUS, HIGH);
        timer[2] = configGame[2];
        timer[3] = 0;
        segundero = timer[3];
        control[3] = true;
        control[0] = false;
        control[1] = true;
        if (control[3] && (controlTime[0] == configGame[3])) {
          runningTime[3] = true;
          runningTime[1] = false;
        }
      }
    }
  }
}

//Resetea el tablero a valores iniciales
void resetBoard() {
  flag = false;
  juego = false;
  tempTime = 0;
  for (int x = 0; x < sizeof(datos_remote); x++) {
    datos_remote[x] = 'N';
    if (x < sizeof(control)) {
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
  resetAll = true;
  printScreen();
}

//Imprime en pantalla del tablero los valores
void printScreen() {
  dmd.clearScreen();

  //Fila Central (Falta Locales)
  if (teams[1] < 10) {
    dmd.drawString(13, 1, String(teams[1]));
  } else {
    separatedNumbers(teams[1], 0, 1);
  }

  //Fila Superior (Puntos Locales)
  if (teams[0] < 10) {
    dmd.drawString(45, 1, String(teams[0]));
  } else {
    separatedNumbers(teams[0], 32, 1);
  }

  //Fila Inferior (Cronometro)
  if (timer[2] < 10) {
    dmd.drawString(69, 1, String(0));
    dmd.drawString(83, 1, String(timer[2]));
  } else {
    separatedNumbers(timer[2], 64, 1);
  }
  dmd.drawString(94, 1, ":");
  if (timer[3] < 10) {
    dmd.drawString(101, 1, String(0));
    dmd.drawString(115, 1, String(timer[3]));
  } else {
    separatedNumbers(timer[3], 96, 1);
  }

  //Fila Central (Extra)
  if (configGame[4] < 10) {
    dmd.drawString(13, 17, String(configGame[4]));
  } else {
    separatedNumbers(configGame[4], 0, 17);
  }

  //Fila Superior (Periodo y Complementario)
  if (runningTime[0] && !runningTime[1]) {
    if (tempTime < 10) {
      dmd.drawString(45, 17, String(tempTime));
    } else {
      separatedNumbers(tempTime, 32, 17);
    }
  } else if ((!runningTime[0] && runningTime[1]) || (runningTime[2] && runningTime[3] && (configGame[3] != 2))) {
    if (tempTime < 10) {
      dmd.drawString(37, 17, String('C'));
      dmd.drawString(51, 17, String(tempTime));
    } else {
      separatedNumbers(tempTime, 32, 17);
    }
  } else if ((configGame[3] == 2) && runningTime[2] && runningTime[3]) {
    dmd.drawString(45, 17, String('P'));
  } else {
    dmd.drawString(45, 17, String(tempTime));
  }

  //Fila Central (Faltas Visitante)
  if (teams[3] < 10) {
    dmd.drawString(77, 17, String(teams[3]));
  } else {
    separatedNumbers(teams[3], 64, 17);
  }

  //Fila Superior (Puntos Visitante)
  if (teams[2] < 10) {
    dmd.drawString(109, 17, String(teams[2]));
  } else {
    separatedNumbers(teams[2], 96, 17);
  }
}

//Divide decenas de unidades
void separatedNumbers(int number, int x, int y) {
  int decenas = number / 10;
  int unidades = number - (decenas * 10);
  dmd.drawString(x + 5, y, String(decenas));
  dmd.drawString(x + 19, y, String(unidades));
}

//Escanea la pantalla
void scanDMD() {
  dmd.scanDisplay();
}
