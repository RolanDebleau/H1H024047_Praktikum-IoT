#include <ESP8266WiFi.h>        
#include <ESP8266HTTPClient.h>  
#include <WiFiClientSecure.h>   
#include <ArduinoJson.h>

const char* ssid = "eduroamer";
const char* password = "wendygantengpoll";
const char* serverUrl = "https://httpbin.org/post"; 

void setup() {
  Serial.begin(115200);

  // Menghubungkan ESP8266 ke WiFi
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi berhasil terhubung!");
}

void loop() {
  // Memastikan ESP8266 masih terhubung ke WiFi
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;   // tambahan -> objek koneksi aman untuk HTTPS
    client.setInsecure();      // tambahan -> lewati verifikasi sertifikat SSL (untuk keperluan praktikum)

    HTTPClient http;

    // Memulai koneksi HTTP
    http.begin(client, serverUrl);  // diubah -> di ESP8266 wajib menyertakan objek client untuk HTTPS

    // Menentukan tipe data yang dikirim
    http.addHeader("Content-Type", "application/json");

    // Membuat objek data sensor dalam format JSON
    JsonDocument doc;

    doc["suhu"] = 28.5;        // Contoh data suhu (°C)
    doc["kelembaban"] = 65.0;  // Contoh data kelembaban (%)

    // Mengubah data JSON menjadi String
    String requestBody;
    serializeJson(doc, requestBody);

    Serial.print("Mengirim data: ");
    Serial.println(requestBody);

    // Mengirim data menggunakan HTTP POST
    int httpResponseCode = http.POST(requestBody);

    // Memeriksa hasil pengiriman
    if (httpResponseCode > 0) {
      Serial.print("Kode Response HTTP: ");
      Serial.println(httpResponseCode);

      Serial.println("Isi Response:");
      Serial.println(http.getString());
    } else {
      Serial.print("Pengiriman gagal, kode error: ");
      Serial.println(httpResponseCode);
    }

    // Mengakhiri koneksi HTTP
    http.end();
  }

  // Mengirim data setiap 10 detik
  delay(10000);
}