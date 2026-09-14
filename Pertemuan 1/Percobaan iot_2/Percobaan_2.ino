#include <DHT.h>

#define DHTPIN 4          // Pin data DHT11 terhubung ke GPIO 4
#define DHTTYPE DHT11     // Tipe sensor yang digunakan
#define RELAYPIN 26       // Pin kendali relay/LED indikator

DHT dht(DHTPIN, DHTTYPE);

const float suhuThreshold = 30.0;  // Ambang batas suhu (°C)

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);  // Pastikan aktuator mati di awal
}

void loop() {
  float suhu = dht.readTemperature();

  if (isnan(suhu)) {
    Serial.println("Gagal membaca data sensor!");
  } 
  else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C -> ");

    // Kendali aktuator berdasarkan hasil pembacaan sensor
    if (suhu > suhuThreshold) {
      digitalWrite(RELAYPIN, HIGH);  // Aktifkan relay/LED
      Serial.println("Aktuator: ON");
    } 
    else {
      digitalWrite(RELAYPIN, LOW);   // Matikan relay/LED
      Serial.println("Aktuator: OFF");
    }
  }

  delay(2000);
}
