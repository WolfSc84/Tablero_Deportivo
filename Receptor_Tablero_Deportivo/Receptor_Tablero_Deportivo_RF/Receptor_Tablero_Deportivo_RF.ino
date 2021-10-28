//Libreria para operar a 32Mhz
//#include <avr/power.h>

//Libreria I2C
#include <Wire.h>

//Libreria Software Serial
#include <SoftwareSerial.h>

//Definimos los pines del puerto serial
SoftwareSerial arduino(15,14); //Rx, Tx

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

char datos[10], temp;
boolean flag = false;
char dataSlave;

//Variables para el control de tiempo
int  minuto = 0, segundo = 0, decenas_minutos = 0, unidad_minutos = 0, decenas_segundos = 0, unidad_segundos = 0, segundero = 0;
DateTime now;

//Indicador LED de recepcion Inalambrica
#define PIN_LED 4

//Indicador de transmision de datos por I2C correctamente
#define PIN_FLAG 5

// Arranque inicial del sistema
void setup() {
  // Seteamos el reloj a 32Mhz
  // clock_prescale_set(clock_div_1);

  //Inicializamos el puerto serie nativo
  Serial.begin(9600);
  Serial.println("Iniciando...");

  //Inicializamos el puerto serial emulado
  arduino.begin(9600);
  Serial.println("Iniciando serial emulado...");

  //Inicializamos el puerto I2C como maestro
  Serial.println("Iniciando el puerto I2C como maestro...");
  Wire.begin();

  //Deteccion del reloj
  Serial.println("Detectando reloj...");
  if (rtc.begin()) {
    Serial.println("RTC detectado!");
  } else {
    Serial.println("No se puede encontrar Reloj RTC!!");
    while (1);
  }

  //Verificar si el reloj no tiene bateria y se encuentra desincronizado
  Serial.println("Verificando estado de energia y configuracion de tiempo actualmente en el RTC.");
  if (rtc.lostPower()) {
    Serial.println("RTC sin energia. Configurando el tiempo!");
    // Esta linea setea el RTC a la fecha y tiempo de esta compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {
    Serial.println("RTC con configuracion de tiempo valida!");
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

  //Led indicador de datos recibidos
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  //Flag de transmision de datos correctos
  pinMode(PIN_FLAG, OUTPUT);
  digitalWrite(PIN_FLAG, LOW);

  //Seteamos el tiempo inicial
  rtc.begin();
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
    flag = true;
    digitalWrite(PIN_LED, HIGH);
    radio.read(datos_remote, sizeof(datos_remote));
    delay(50);
    digitalWrite(PIN_LED, LOW);

    //Reportamos por el puerto serial los datos recibidos
    Serial.print("Los datos recibidos inalambricamente son " );
    Serial.print(datos_remote);
    Serial.print(" con una longitud de ");
    Serial.print(sizeof(datos_remote));
    Serial.println(" bytes.");

    //Procedemos a enviar los datos por puerto I2C
    tiempo();
    assemble_data();
    if (!send_data()) {
      Serial.println("Error al enviar los datos por I2C... No se enviaron correctamente!");
      digitalWrite(PIN_FLAG, LOW);
    } else {
      digitalWrite(PIN_FLAG, HIGH);
      Serial.print("Los datos se enviaron correctamente por el puerto I2C! Los datos son ");
      Serial.print(datos);
      Serial.print(" con una longitud de ");
      Serial.print(sizeof(datos));
      Serial.println(" bytes.");
    }
    delay(50);
    digitalWrite(PIN_FLAG, LOW);
  } else {
    //Leemos el tiempo actual
    flag = false;
    tiempo();
    if (segundero != segundo) {
      //Procedemos a enviar los datos por puerto I2C
      assemble_data();
      if (!send_data()) {
        Serial.println("Error al enviar los datos por I2C... No se enviaron correctamente!");
        digitalWrite(PIN_FLAG, LOW);
      } else {
        digitalWrite(PIN_FLAG, HIGH);
        Serial.print("Los datos se enviaron correctamente por el puerto I2C! Los datos son " );
        Serial.print(datos);
        Serial.print(" con una longitud de ");
        Serial.print(sizeof(datos));
        Serial.println(" bytes.");
      }
      delay(50);
      digitalWrite(PIN_FLAG, LOW);
      segundero = segundo;
      hora();
      Serial.println("Esperando recibir informacion en el bus de datos SPI por el dispositivo RF...");
    }
  }
}

//Inicia el proceso de envio de datos por puerto I2C
boolean send_data() {
  Wire.beginTransmission(1);
  delay(20);
  Wire.write(datos, sizeof(datos));
  delay(20);
  Wire.endTransmission();
  Serial.print("Se enviaron por el puerto I2C los datos: " );
  Serial.print(datos);
  Serial.print(" con una longitud de ");
  Serial.print(sizeof(datos));
  Serial.println(" bytes.");
  delay(100);
//  Serial.print("La datos recibidos por el puerto serial emulado son ");
//  Serial.print(dataSlave);
//  Serial.print(" con una longitud de ");
//  Serial.print(sizeof(dataSlave));
//  Serial.println(" bytes");
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

//Funcion mostrar en pantalla LED informacion
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
