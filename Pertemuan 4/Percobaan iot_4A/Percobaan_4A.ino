#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>   // ArduinoJson v7 (menggunakan JsonDocument)

// ===== Konfigurasi WiFi =====
const char* WIFI_SSID     = "eduroamer";
const char* WIFI_PASSWORD = "wendygantengpoll";

// ===== Konfigurasi MQTT =====
const char* MQTT_SERVER    = "broker.hivemq.com";
const uint16_t MQTT_PORT   = 1883;
const char* TOPIC_PERINTAH = "unsoed/tk245004/kelompokwewenrolan/perintah";

// ===== Pin aktuator =====
const uint8_t LED_PIN = 5;

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// ---------- Kontrol aktuator ----------
void setAktuator(bool aktif) {
  digitalWrite(LED_PIN, aktif ? HIGH : LOW);
  Serial.println(aktif ? "Aktuator: ON" : "Aktuator: OFF");
}

// ---------- Callback: dipanggil otomatis saat ada pesan masuk ----------
void onMessage(char* topic, byte* payload, unsigned int length) {
  String pesan;
  pesan.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    pesan += static_cast<char>(payload[i]);
  }

  Serial.printf("Pesan diterima [%s]: %s\n", topic, pesan.c_str());

  // Parsing JSON, contoh payload: {"perintah":"ON"}
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, pesan);
  if (err) {
    Serial.printf("Gagal parsing JSON: %s\n", err.c_str());
    return;
  }

  // Pastikan key "perintah" ada (mencegah crash akibat pointer null)
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
    // Pastikan WiFi tetap tersambung sebelum mencoba MQTT
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

// ---------- Setup & Loop ----------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();  // wajib dipanggil terus-menerus agar pesan dapat diterima
}