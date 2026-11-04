// =====================================================
// Modul 5 - Percobaan 5A
// Pengolahan Data Sensor pada ESP32 (Edge Processing)
// Moving Average (N = 5) + pencatatan suhu Min/Max
// =====================================================

#include <DHT.h>              // pustaka sensor DHT

#define DHTPIN 4              // sensor DHT22 dihubungkan ke GPIO 4
#define DHTTYPE DHT22         // tipe sensor yang digunakan

DHT dht(DHTPIN, DHTTYPE);     // membuat objek sensor DHT

const int N = 5;              // jumlah sampel moving average
float buffer[N];              // menyimpan N pembacaan terakhir
int indeks = 0;               // posisi penulisan berikutnya pada buffer
int jumlah = 0;               // jumlah sampel yang sudah terisi

float suhuMin = 1000.0;       // nilai awal besar agar pembacaan pertama menjadi minimum
float suhuMax = -1000.0;      // nilai awal kecil agar pembacaan pertama menjadi maksimum

// Menghitung rata-rata bergerak dari N pembacaan terakhir
float hitungMovingAverage(float nilaiBaru) {
  buffer[indeks] = nilaiBaru;        // simpan nilai baru ke buffer
  indeks = (indeks + 1) % N;         // geser indeks, kembali ke 0 jika mencapai N (circular buffer)
  if (jumlah < N) jumlah++;          // tambah jumlah sampel sampai buffer penuh

  float total = 0;
  for (int i = 0; i < jumlah; i++) { // jumlahkan seluruh isi buffer yang sudah terisi
    total += buffer[i];
  }
  return total / jumlah;             // rata-rata
}

void setup() {
  Serial.begin(115200);              // baud rate Serial Monitor
  dht.begin();                       // inisialisasi sensor
}

void loop() {
  float suhu = dht.readTemperature();   // baca suhu (°C)

  if (isnan(suhu)) {                    // pembacaan gagal -> abaikan
    Serial.println("Pembacaan gagal, data diabaikan");
  } else {
    float suhuFilter = hitungMovingAverage(suhu);  // data hasil filter

    if (suhu < suhuMin) suhuMin = suhu; // perbarui nilai minimum
    if (suhu > suhuMax) suhuMax = suhu; // perbarui nilai maksimum

    Serial.print("Mentah: ");
    Serial.print(suhu);
    Serial.print(" | Filter: ");
    Serial.print(suhuFilter);
    Serial.print(" | Min: ");
    Serial.print(suhuMin);
    Serial.print(" | Max: ");
    Serial.println(suhuMax);
  }

  delay(2000);                          // pembacaan setiap 2 detik
}