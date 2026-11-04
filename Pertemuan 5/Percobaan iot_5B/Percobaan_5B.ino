#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 4
#define DHTTYPE DHT22

const char* ssid = "eduroamer";            // ganti dengan WiFi Anda (2.4 GHz)
const char* password = "wendygantengpoll";    // ganti dengan password WiFi

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topicData = "unsoed/tk245004/kelompokwewenroland/data";  // harus sama dengan TOPIC di Python

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long terakhirKirim = 0;
const unsigned long intervalKirim = 5000;  // publish tiap 5 detik

void setupWiFi() {
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung, IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "esp32-" + String(random(0xffff), HEX);  // ID acak agar tidak bentrok
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung");
    } else {
      Serial.print("gagal, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setupWiFi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  if (millis() - terakhirKirim >= intervalKirim) {
    terakhirKirim = millis();

    float suhu = dht.readTemperature();
    if (isnan(suhu)) {
      Serial.println("Pembacaan gagal, data diabaikan");
      return;
    }

    StaticJsonDocument<64> doc;
    doc["suhu"] = suhu;                 // key "suhu" yang dibaca Python
    char buffer[64];
    serializeJson(doc, buffer);

    client.publish(topicData, buffer);
    Serial.println(String("Publish: ") + buffer);
  }
}