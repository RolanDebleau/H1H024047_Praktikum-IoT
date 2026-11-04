#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>   // ArduinoJson v7 (menggunakan JsonDocument)
#include <DHT.h>           // Adafruit DHT sensor library

// ===== Konfigurasi WiFi =====
const char* WIFI_SSID     = "eduroamer";
const char* WIFI_PASSWORD = "wendygantengpoll";

// ===== Konfigurasi MQTT =====
const char* MQTT_SERVER    = "broker.hivemq.com";
const uint16_t MQTT_PORT   = 1883;
const char* TOPIC_DATA     = "unsoed/tk245004/kelompokwewenrolan/data";
const char* TOPIC_PERINTAH = "unsoed/tk245004/kelompokwewenrolan/perintah";

// ===== Pin dan sensor =====
#define DHT_PIN  4
#define DHT_TYPE DHT11
const uint8_t LED_PIN = 5;

// ===== Interval publish (non-blocking) =====
const unsigned long INTERVAL_PUBLISH = 5000;  // 5 detik
unsigned long waktuPublishTerakhir = 0;

DHT          dht(DHT_PIN, DHT_TYPE);
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- Kontrol aktuator ----------
void setAktuator(bool aktif) {
  digitalWrite(LED_PIN, aktif ? HIGH : LOW);
  Serial.println(aktif ? "Aktuator: ON" : "Aktuator: OFF");
}

// ---------- Callback: dipanggil saat ada pesan masuk ----------
void onMessage(char* topic, byte* payload, unsigned int length) {
  String pesan;
  pesan.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    pesan += static_cast<char>(payload[i]);
  }
  Serial.printf("Pesan diterima [%s]: %s\n", topic, pesan.c_str());

  JsonDocument doc;
  if (deserializeJson(doc, pesan)) {
    Serial.println("Gagal parsing JSON, pesan diabaikan");
    return;
  }

  // Contoh payload: {"perintah":"ON"}
  const char* perintah = doc["perintah"] | "";
  if (strcmp(perintah, "ON") == 0) {
    setAktuator(true);
  } else if (strcmp(perintah, "OFF") == 0) {
    setAktuator(false);
  } else {
    Serial.println("Perintah tidak dikenal atau key 'perintah' tidak ada");
  }
}

// ---------- Koneksi WiFi ----------
void connectWiFi() {
  Serial.print("Menghubungkan ke WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi terhubung, IP: ");
  Serial.println(WiFi.localIP());
}

// ---------- Koneksi MQTT ----------
void connectMQTT() {
  while (!mqtt.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }

    Serial.print("Menghubungkan ke broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("berhasil!");
      mqtt.subscribe(TOPIC_PERINTAH);
      Serial.printf("Subscribe ke topic: %s\n", TOPIC_PERINTAH);
    } else {
      Serial.printf("gagal, rc=%d. Coba lagi dalam 2 detik\n", mqtt.state());
      delay(2000);
    }
  }
}

// ---------- Baca sensor & publish ----------
void publishData() {
  float suhu = dht.readTemperature();
  if (isnan(suhu)) {
    Serial.println("Gagal membaca sensor DHT22");
    return;
  }

  JsonDocument doc;
  doc["suhu"] = suhu;

  char buffer[128];
  serializeJson(doc, buffer, sizeof(buffer));

  if (mqtt.publish(TOPIC_DATA, buffer)) {
    Serial.printf("Data terkirim: %s\n", buffer);
  } else {
    Serial.println("Gagal mengirim data");
  }
}

// ---------- Setup & Loop ----------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();  // memproses pesan masuk secara terus-menerus

  // Publish berkala tanpa memblokir proses subscribe
  unsigned long sekarang = millis();
  if (sekarang - waktuPublishTerakhir >= INTERVAL_PUBLISH) {
    waktuPublishTerakhir = sekarang;
    publishData();
  }
}