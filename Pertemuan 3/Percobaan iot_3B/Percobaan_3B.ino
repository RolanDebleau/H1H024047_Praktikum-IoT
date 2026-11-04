#include <ESP8266WiFi.h>       
#include <PubSubClient.h>  
#include <WiFiClientSecure.h>   
#include <ArduinoJson.h>

const char* ssid = "eduroamer";
const char* password = "wendygantengpoll";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttTopic = "unsoed/tk245004/kelompokwewenroland/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi menghubungkan ESP32 ke WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi berhasil terhubung!");
}

// Fungsi menghubungkan ESP32 ke broker MQTT
void hubungkanMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");

    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("berhasil terhubung!");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 2 detik");

      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Menghubungkan ke WiFi
  hubungkanWiFi();

  // Mengatur server MQTT
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  // Memastikan ESP32 terhubung ke MQTT
  if (!client.connected()) {
    hubungkanMQTT();
  }

  client.loop();

  // Membuat data sensor dalam format JSON
  JsonDocument doc;

  doc["suhu"] = 100000;
  doc["kelembaban"] = 65.0;

  // Mengubah JSON menjadi String/char array
  char buffer[128];
  serializeJson(doc, buffer);

  // Mempublikasikan data ke topic MQTT
  client.publish(mqttTopic, buffer);

  Serial.print("Data terkirim ke topic ");
  Serial.print(mqttTopic);
  Serial.print(": ");
  Serial.println(buffer);

  // Publish data setiap 5 detik
  delay(5000);
}