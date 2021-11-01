//Libreria para operar a 32Mhz
//#include <avr/power.h>

//Libreria I2C
#include <Wire.h>

// Libreria Funcion de tiempo DS3231
#include "RTClib.h"

// Inicializacion del objeto RTC
RTC_DS3231 rtc;

//Libreria SPI
#include <SPI.h>

// Librerias Funciones RF
#include <nRF24L01.h>
#include <RF24.h>

//Codigo del canal de transmision
const uint64_t codigo = 0xE8E8F0F0E1LL;

//Declaremos los pines CE y el CSN
#define CE_PIN 9
#define CSN_PIN 10

//Objeto radio (NRF24L01)
RF24 radio(CE_PIN, CSN_PIN);

//Vector con los datos a recibir
char datos_remote[6];

char datos[10];
boolean flag = false;

//Variables para el control de tiempo
int  minuto = 0, segundo = 0, decenas_minutos = 0, unidad_minutos = 0, decenas_segundos = 0, unidad_segundos = 0, segundero = 0;
DateTime now;

// Arranque inicial del sistema
void setup() {
  // Seteamos el reloj a 32Mhz
  // clock_prescale_set(clock_div_1);

  //Inicializamos el puerto serie nativo
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto I2C como maestro
  Wire.begin();

  //Deteccion del reloj
  if (!rtc.begin()) {
    Serial.println("No se puede encontrar Reloj RTC!!");
    while (1);
  }

  //Verificar si el reloj no tiene bateria y se encuentra desincronizado
  if (rtc.lostPower()) {
    Serial.println("RTC sin energia. Configurando el tiempo!");
    // Esta linea setea el RTC a la fecha y tiempo de esta compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {

  }

  //Inicializamos el NRF24L01
  radio.begin();

  //Configuracion de la modulacion
  radio.setAutoAck(false);

  //Frecuencia a utilizar
  radio.setDataRate(RF24_250KBPS);

  //Abrimos el canal de lectura
  radio.openReadingPipe(1, codigo);

  //Inicializamos la escucha por el canal
  radio.startListening();

  //Seteamos el tiempo inicial
  Serial.print("El tiempo inicial es ");
  now = rtc.now();
  Serial.print(now.hour());
  Serial.print(":");
  tiempo();
  hora();
  segundero = segundo;
}

void loop() {
  // Chequeamos si hay data en el canal de lectura
  if (radio.available()) {
    //Leemos los datos y los guardamos en la variable datos_remote[]
    radio.read(datos_remote, sizeof(datos_remote));
    delay(50);
    tiempo();
    delay(50);
    flag = true;
    assemble_data();
    if (!send_data()) {
      Serial.println("Error al enviar datos por el puerto I2C!!");
    } else {
      Serial.print("Enviados los datos ");
      Serial.flush();
      Serial.print(String(datos));
      Serial.flush();
      Serial.print(" por I2C, con una longitud de ");
      Serial.print(sizeof(datos));
      Serial.println(" bytes.");
    }
    delay(50);
  } else {
    //Leemos el tiempo actual
    tiempo();
    if (segundero != segundo) {
      //Procedemos a enviar los datos por puerto I2C
      flag = false;
      assemble_data();
      if (!send_data()) {
        Serial.println("Error al enviar datos por el puerto I2C!!");
      } else {
        Serial.print("Enviados los datos " );
        Serial.flush();
        Serial.print(String(datos));
        Serial.flush();
        Serial.print(" por I2C, con una longitud de ");
        Serial.print(sizeof(datos));
        Serial.println(" bytes.");
      }
      delay(50);
      segundero = segundo;
    }
  }
}

//Inicia el proceso de envio de datos por puerto I2C
boolean send_data() {
  Wire.beginTransmission(1);
  Wire.write(datos, sizeof(datos));
  Wire.endTransmission();
  delay(50);
  return true;
}

//Armamos la trama de datos a enviar por I2C
void assemble_data() {
  if (flag) {
    for (int x = 0; x < 10; x++) {
      switch (x) {
        case 0:
          datos[x] = numberToChar(decenas_minutos);
          break;
        case 1:
          datos[x] = numberToChar(unidad_minutos);
          break;
        case 2:
          datos[x] = numberToChar(decenas_segundos);
          break;
        case 3:
          datos[x] = numberToChar(unidad_segundos);
          break;
        default:
          datos[x] = datos_remote[x - 4];
          break;
      }
    }
  } else {
    for (int x = 0; x < 10; x++) {
      switch (x) {
        case 0:
          datos[x] = numberToChar(decenas_minutos);
          break;
        case 1:
          datos[x] = numberToChar(unidad_minutos);
          break;
        case 2:
          datos[x] = numberToChar(decenas_segundos);
          break;
        case 3:
          datos[x] = numberToChar(unidad_segundos);
          break;
        default:
          datos[x] = 'N';
          break;
      }
    }
  }
}

//Funcion tiempo
void tiempo() {
  now = rtc.now();
  minuto = now.minute();
  segundo = now.second();
  if (minuto < 10) {
    decenas_minutos = 0;
    unidad_minutos = minuto;
  } else {
    decenas_minutos = minuto / 10;
    unidad_minutos = minuto - (decenas_minutos * 10);
  }
  if (segundo < 10) {
    decenas_segundos = 0;
    unidad_segundos = segundo;
  } else {
    decenas_segundos = segundo / 10;
    unidad_segundos = segundo - (decenas_segundos * 10);
  }
}

////Funcion mostrar en pantalla LED informacion
void hora() {
  if (minuto < 10) {
    Serial.print(decenas_minutos, DEC);
    Serial.print(unidad_minutos, DEC);
  } else {
    Serial.print(minuto, DEC);
  }
  Serial.print(':');
  if (segundo > 9) {
    Serial.print(segundo, DEC);
  } else {
    Serial.print(decenas_segundos, DEC);
    Serial.print(unidad_segundos, DEC);
  }
  Serial.println();
}

//Convertir caracteres numericos a numero decimal
char numberToChar(int number) {
  switch (number) {
    case 1:
      return '1';
      break;
    case 2:
      return '2';
      break;
    case 3:
      return '3';
      break;
    case 4:
      return '4';
      break;
    case 5:
      return '5';
      break;
    case 6:
      return '6';
      break;
    case 7:
      return '7';
      break;
    case 8:
      return '8';
      break;
    case 9:
      return '9';
      break;
    default:
      return '0';
      break;
  }
}
