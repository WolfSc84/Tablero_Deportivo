//Libreria SPI para manejo de DMD y RF
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

//Vector con los datos a enviar
char datos[6] = {'0', '0', '0', '0', '0', '0'};

//Contador de segundos por boton
float c[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//Mapeo de puertos digitales Arduino
int p[12] = {2, 3, 4, 5, 6, 7, 8, 14, 15, 16, 17, 18};

//Detector de pulsaciones
boolean flag = false;

//Indicador LED de envio
#define PIN_LED 19

void setup() {

  //Inicializamos el NRF24L01
  radio.begin();

  //Configuracion de la modulacion
  radio.setAutoAck(false);

  //Frecuencia a utilizar
  radio.setDataRate(RF24_250KBPS);

  //Abrimos un canal de escritura
  radio.openWritingPipe(codigo);

  //Inicializamos el puerto serie
  Serial.begin(19200);
  Serial.println("Iniciando..............");

  //Configuramos pines para botones
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);
  pinMode(18, INPUT);
  pinMode(PIN_LED, OUTPUT);
}

void loop() {

  for (int x = 0; x < sizeof(p) / sizeof(p[0]) ; x++) {
    while (digitalRead(p[x])) {
      digitalWrite(PIN_LED, HIGH);
      c[x]++;
    }
    digitalWrite(PIN_LED, LOW);
    if (c[x] > 500) {
      flag = true;
      break;
    }
  }

  if (flag) {
    for (int x = 0; x < sizeof(p) / sizeof(p[0]) ; x++) {
      if (c[x] >= 500 && c[x] < 70000) {
        //Funcionalidad para pulsacion menor de 5 segundos.
        Serial.print("----------------------------Boton ");
        Serial.print(p[x]);
        Serial.println(" presionado por menos de 5 segundos aproximadamente!!!!----------------------------------------");
        Serial.print("Duracion del pulso en pin ");
        Serial.print(p[x]);
        Serial.print(":");
        Serial.println(c[x]);
        c[x] = 0;
        flag = false;
        setData(x, 1);
      } else if (c[x] >= 70000 && c[x] < 200000 ) {
        //Funcionalidad para pulsacion mayor de 5 segundos y menor de 10 segundos.
        Serial.print("----------------------------Boton ");
        Serial.print(p[x]);
        Serial.println(" presionado por 5 segundos aproximadamente!!!!----------------------------------------");
        Serial.print("Duracion del pulso en pin ");
        Serial.print(p[x]);
        Serial.print(":");
        Serial.println(c[x]);
        c[x] = 0;
        flag = false;
        setData(x, 2);
      } else if (c[x] >= 200000) {
        //Funcionalidad para pulsacion mayor de 10 segundos.
        Serial.print("----------------------------Boton ");
        Serial.print(p[x]);
        Serial.println(" presionado por 10 segundos aproximadamente!!!!----------------------------------------");
        Serial.print("Duracion del pulso en pin ");
        Serial.print(p[x]);
        Serial.print(":");
        Serial.println(c[x]);
        c[x] = 0;
        flag = false;
        setData(x, 3);
      }
    }
  }
}

void setData(int x, int y) {
  Serial.print("Bus de datos inicial antes de cualquier pulsacion ----> ");
  Serial.println(datos);
  if (y == 1) {
    switch (x) {
      case 0:
        datos[0] = 'r';
        break;
      case 1:
        datos[1] = 's';
        break;
      case 2:
        datos[1] = 'f';
        break;
      case 3:
        datos[1] = 'b';
        break;
      case 4:
        datos[1] = 'o';
        break;
      case 5:
        datos[2] = 'L';
        break;
      case 6:
        datos[2] = 'V';
        break;
      case 7:
        datos[3] = 'L';
        break;
      case 8:
        datos[3] = 'V';
        break;
      case 9:
        datos[4] = 'U';
        break;
      case 10:
        datos[4] = 'P';
        break;
      case 11:
        datos[5] = 'X';
        break;
    }
  } else if (y == 2) {
    switch (x) {
      case 0:
        datos[0] = 'p';
        break;
      case 1:
        datos[1] = 's';
        break;
      case 2:
        datos[1] = 'f';
        break;
      case 3:
        datos[1] = 'b';
        break;
      case 4:
        datos[1] = 'o';
        break;
      case 5:
        datos[2] = 'l';
        break;
      case 6:
        datos[2] = 'v';
        break;
      case 7:
        datos[3] = 'l';
        break;
      case 8:
        datos[3] = 'v';
        break;
      case 9:
        datos[4] = 'D';
        break;
      case 10:
        datos[4] = 'p';
        break;
      case 11:
        datos[5] = 'X';
        break;
    }
  } else if (y == 3) {
    switch (x) {
      case 0:
        datos[0] = 'S';
        break;
      case 1:
        datos[1] = 's';
        break;
      case 2:
        datos[1] = 'f';
        break;
      case 3:
        datos[1] = 'b';
        break;
      case 4:
        datos[1] = 'o';
        break;
      case 5:
        datos[2] = 'X';
        break;
      case 6:
        datos[2] = 'Y';
        break;
      case 7:
        datos[3] = 'X';
        break;
      case 8:
        datos[3] = 'Y';
        break;
      case 9:
        datos[4] = 'R';
        break;
      case 10:
        datos[4] = 'r';
        break;
      case 11:
        datos[5] = 'Y';
        break;
    }
  }
  Serial.print("Los datos generados segun la pulsacion son: ");
  Serial.println(datos);
  Serial.println("Enviando Datos... ");
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_LED, LOW);
  delay(100);
  digitalWrite(PIN_LED, HIGH);
  while (!sendData());
  digitalWrite(PIN_LED, LOW);
  delay(100);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  Serial.println("Datos enviados... ");
  digitalWrite(PIN_LED, LOW);
  Serial.println("Limpieza de datos iniciada....");
  clearData();
  Serial.print("Datos limpios... ");
  Serial.println(datos);
  Serial.println("En espera de nuevos datos..... -----> ");

}

bool sendData() {
  return radio.write(datos, sizeof(datos));
}

void clearData() {
  for (int x = 0; x < sizeof(datos) / sizeof(datos[0]) ; x++) {
    datos[x] = '0';
  }
}
