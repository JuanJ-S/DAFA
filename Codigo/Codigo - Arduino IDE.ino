//Variables de blynk
#define BLYNK_TEMPLATE_ID "TMPL2hTkAHlpS"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "sauoRlbpT7XZwRfP80DVDuPRCn39Podm"

//Librerias
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "HX710B.h"

// WiFi
char ssid[] = "Redmi Note 11";
char pass[] = "1234567890";

// Pines sensores de caudal
const byte flowPin1 = 15;   // D5 - GPIO5
const byte flowPin2 = 2;   // D4 - GPIO4
const byte flowPin3 = 4;  // D4 - GPIO14

// Pines sensores de presión
#define PRES1_DATA  5  // D7 - GPIO13
#define PRES1_CLK   18   // D4 - GPIO2
#define PRES2_DATA  19   // D3 - GPIO0
#define PRES2_CLK   21  // D8 - GPIO15

HX710B presion1(PRES1_DATA, PRES1_CLK);
HX710B presion2(PRES2_DATA, PRES2_CLK);

// Bomba
const byte bombaPin = 23; // D6 - GPIO12
bool bombaEstado = false;

// Contadores de pulsos
volatile int pulses1 = 0, pulses2 = 0, pulses3 = 0;

BlynkTimer timer;

ICACHE_RAM_ATTR void contarPulsos1() { pulses1++; }
ICACHE_RAM_ATTR void contarPulsos2() { pulses2++; }
ICACHE_RAM_ATTR void contarPulsos3() { pulses3++; }

float calcularCaudal(int pulses) {
  return (pulses * 6.0) / 450.0; // L/min
}

float calcularPresion(float voltaje) {
  return (voltaje - 0.5) * (40.0 / 4.0); // en kPa
}

void enviarDatos() {
  float c1 = calcularCaudal(pulses1);
  float c2 = calcularCaudal(pulses2);
  float c3 = calcularCaudal(pulses3);
  pulses1 = pulses2 = pulses3 = 0;

  uint32_t raw1, raw2;
  float pres1kPa = NAN, pres2kPa = NAN;

  if (presion1.read(&raw1, 1000UL) == HX710B_OK) {
    float v1 = raw1 * 5.0 / 16777215.0;
    pres1kPa = calcularPresion(v1);
  }

  if (presion2.read(&raw2, 1000UL) == HX710B_OK) {
    float v2 = raw2 * 5.0 / 16777215.0;
    pres2kPa = calcularPresion(v2);
  }

  Blynk.virtualWrite(V0, c1);
  Blynk.virtualWrite(V1, c2);
  Blynk.virtualWrite(V2, c3);
  Blynk.virtualWrite(V4, pres1kPa);
  Blynk.virtualWrite(V5, pres2kPa);
}

BLYNK_WRITE(V3) {
  bombaEstado = param.asInt();
  digitalWrite(bombaPin, bombaEstado ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(flowPin1, INPUT_PULLUP);
  pinMode(flowPin2, INPUT_PULLUP);
  pinMode(flowPin3, INPUT_PULLUP);
  pinMode(bombaPin, OUTPUT);
  digitalWrite(bombaPin, LOW);

  attachInterrupt(digitalPinToInterrupt(flowPin1), contarPulsos1, RISING);
  attachInterrupt(digitalPinToInterrupt(flowPin2), contarPulsos2, RISING);
  attachInterrupt(digitalPinToInterrupt(flowPin3), contarPulsos3, RISING);

  if (!presion1.init() || !presion2.init()) {
    Serial.println("Error iniciando sensores HX710B");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, enviarDatos);
}

void loop() {
  Blynk.run();
  timer.run();
}