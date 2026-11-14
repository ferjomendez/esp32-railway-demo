// esp32_railway.ino
// Envia lecturas de un sensor a la API en Railway por HTTPS.
// Librerias: WiFi y HTTPClient (vienen con el core ESP32), ArduinoJson (Library Manager).

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ---------- CAMBIAR ESTOS DATOS ----------
const char* WIFI_SSID   = "NOMBRE_DE_TU_RED";
const char* WIFI_PASS   = "CLAVE_DE_TU_RED";
const char* SERVER_URL  = "https://TU-APP.up.railway.app/api/lecturas";
const char* API_KEY     = "la-misma-clave-que-pusiste-en-railway";
const char* DISPOSITIVO = "esp32-grupo1";
// -----------------------------------------

const int PIN_SENSOR = 34;                 // pin analogico de ejemplo
const unsigned long INTERVALO_MS = 10000;  // enviar cada 10 s
unsigned long ultimoEnvio = 0;

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());
}

float leerSensor() {
  // Reemplazar por la lectura real de su sensor (DHT, LDR, ultrasonico, etc.)
  int crudo = analogRead(PIN_SENSOR);      // 0 a 4095
  return crudo * 3.3 / 4095.0;             // ejemplo: voltaje
}

void enviarLectura(float valor) {
  WiFiClientSecure cliente;
  cliente.setInsecure();  // acepta el certificado sin verificarlo (ver guia)

  HTTPClient http;
  http.begin(cliente, SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", API_KEY);

  JsonDocument doc;
  doc["dispositivo"] = DISPOSITIVO;
  doc["sensor"] = "voltaje";
  doc["valor"] = valor;
  String cuerpo;
  serializeJson(doc, cuerpo);

  int codigo = http.POST(cuerpo);
  Serial.printf("POST %s -> %d\n", cuerpo.c_str(), codigo);
  if (codigo > 0) {
    Serial.println(http.getString());
  } else {
    Serial.printf("Error: %s\n", http.errorToString(codigo).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  if (millis() - ultimoEnvio >= INTERVALO_MS) {
    ultimoEnvio = millis();
    enviarLectura(leerSensor());
  }
}
