/********************************************************************
 * Projeto: Wellsess - Sistema IoT Corporativo
 * Baseado no código do Professor André Tritiack
 * Adaptado para Wokwi Online por: Lucas, Ranaldo, Fabrício
 ********************************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

//----------------------------------------------------------
// Configurações Wellsess

#define LED_PIN 2
#define DHT_PIN 12
#define DHT_TYPE DHT22
#define POT_PIN 34

// Identificação
const char* GRUPO_ID = "WELLSESS_GROUP";
const char* DEVICE_ID = "WELLSESS_CORP_01";

// Wi-Fi
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ✅ API NO RAILWAY
const char* API_URL = "https://gs-welless-production.up.railway.app/api/dados-iot";

// ✅ ID DO CHECKIN EXISTENTE
const int CHECKIN_ID = 11;

//----------------------------------------------------------
// Variáveis

DHT dhtSensor(DHT_PIN, DHT_TYPE);
int checkinCount = 1;
bool dhtFuncionando = false;

//----------------------------------------------------------
// Conexão Wi-Fi

void connectWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi Conectado!");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());
}

//----------------------------------------------------------
// Testa sensor DHT

void testarSensorDHT() {
  Serial.print("🔧 Testando sensor DHT... ");
  
  float temp = dhtSensor.readTemperature();
  float hum = dhtSensor.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    dhtFuncionando = true;
    Serial.println("✅ FUNCIONANDO!");
  } else {
    dhtFuncionando = false;
    Serial.println("❌ COM PROBLEMAS - Usando dados simulados");
  }
}

//----------------------------------------------------------
String getDataAtual() {
  return "2025-11-23"; //Digitar data para simulação
}

//----------------------------------------------------------
// Envia dados para API Wellsess (VERSÃO FINAL)

void enviarParaAPI(float temperatura, float umidade, int estresse) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    Serial.print("🔗 Conectando com: ");
    Serial.println(API_URL);
    
    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(15000); // 15 segundos de timeout
    
    // ✅ JSON CORRETO para API Wellsess
    StaticJsonDocument<512> doc;
    doc["dataColeta"] = getDataAtual();
    
    // ✅ Temperatura como string numérica (sem "°C")
    doc["temperatura"] = String(temperatura, 1);
    
    // ✅ Local sensor dentro do limite de caracteres
    doc["localSensor"] = "ESP32";
    
    // ✅ Checkin ID como Long
    doc["checkinId"] = (long)CHECKIN_ID;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println("📤 JSON enviado:");
    Serial.println(jsonString);
    Serial.println("Checkin ID: " + String(CHECKIN_ID));
    
    int httpCode = http.POST(jsonString);
    
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("📨 Código HTTP: " + String(httpCode));
      
      if (httpCode == 200) {
        Serial.println("🎉 Dados ambientais salvos com sucesso!");
        Serial.println("📄 Resposta: " + response);
      } else {
        Serial.println("📄 Resposta: " + response);
        
        if (httpCode == 400) {
          Serial.println("❌ Erro 400 - Bad Request");
        } else if (httpCode == 404) {
          Serial.println("❌ Erro 404 - Not Found");
        } else if (httpCode == 500) {
          Serial.println("❌ Erro 500 - Internal Server Error");
        }
      }
    } else {
      Serial.println("❌ Falha na conexão HTTP: " + String(httpCode));
      Serial.println("Erro: " + http.errorToString(httpCode));
    }
    
    http.end();
  } else {
    Serial.println("❌ WiFi desconectado");
    Serial.print("Status WiFi: ");
    Serial.println(WiFi.status());
  }
}

//----------------------------------------------------------
// Setup

void setup() {
  Serial.begin(115200);
  Serial.println("\n" \
    "╔══════════════════════════════╗\n" \
    "║         🌐 WELLSESS          ║\n" \
    "║    Sistema IoT Corporativo   ║\n" \
    "║   ESP32 → API REST Direct    ║\n" \
    "╚══════════════════════════════╝");
  
  Serial.println("📋 Configuração:");
  Serial.println("   API: " + String(API_URL));
  Serial.println("   Checkin ID: " + String(CHECKIN_ID));
  Serial.println("   Grupo: " + String(GRUPO_ID));
  Serial.println("   Device: " + String(DEVICE_ID));
  
  // Hardware
  pinMode(LED_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Sensores
  dhtSensor.begin();
  delay(2000); // Espera sensor inicializar
  
  // Testa componentes
  connectWiFi();
  testarSensorDHT();
  
  Serial.println("🚀 Sistema Wellsess inicializado com sucesso!");
  Serial.println("🔍 Checkin ID: " + String(CHECKIN_ID));
}

//----------------------------------------------------------
// Loop principal

void loop() {
  // ✅ VERIFICA CONEXÃO
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🔌 Reconectando WiFi...");
    connectWiFi();
    return; // Espera próximo ciclo
  }
  
  // ✅ LEITURA DOS SENSORES
  float temperatura, umidade;
  int nivelEstresse = analogRead(POT_PIN);
  
  if (dhtFuncionando) {
    temperatura = dhtSensor.readTemperature();
    umidade = dhtSensor.readHumidity();
    
    // Verifica se as leituras são válidas
    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("⚠️  Leitura DHT inválida, usando dados simulados");
      dhtFuncionando = false;
      temperatura = 23.5 + (random(0, 150) / 100.0);
      umidade = 45.0 + (random(0, 200) / 100.0);
    }
  } else {
    // ✅ DADOS SIMULADOS (se sensor com problema)
    temperatura = 23.5 + (random(0, 150) / 100.0);
    umidade = 45.0 + (random(0, 200) / 100.0);
  }
  
  // ✅ MOSTRA NO SERIAL
  Serial.println("\n📊 Dados Coletados:");
  Serial.println("─────────────────────────────");
  Serial.print("🌡️  Temperatura: "); Serial.print(temperatura, 1); Serial.println("°C");
  Serial.print("💧 Umidade: "); Serial.print(umidade, 1); Serial.println("%");
  Serial.print("😰 Nível Estresse: "); Serial.println(map(nivelEstresse, 0, 4095, 1, 10));
  Serial.print("🔢 Checkin: "); Serial.println(CHECKIN_ID);
  Serial.print("📡 Sensor: "); Serial.println(dhtFuncionando ? "REAL" : "SIMULADO");
  Serial.print("🔁 Envio: "); Serial.println(checkinCount);
  
  // ✅ ENVIA PARA API
  enviarParaAPI(temperatura, umidade, nivelEstresse);
  
  // ✅ FEEDBACK VISUAL
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  
  // ✅ INCREMENTA CONTADOR E ESPERA
  checkinCount++;
  Serial.println("⏰ Aguardando 30 segundos...");
  Serial.println("─────────────────────────────");
  delay(30000); // 30 segundos
}