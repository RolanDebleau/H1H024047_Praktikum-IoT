# =====================================================
# Modul 5 - Percobaan 5B
# Akuisisi (MQTT), penyimpanan CSV, analisis statistik,
# deteksi anomali z-score, dan visualisasi
# =====================================================

import json                              # parsing payload JSON
import time                              # timestamp waktu penerimaan
import paho.mqtt.client as mqtt          # klien MQTT
import pandas as pd                      # pengolahan data tabular
import matplotlib.pyplot as plt          # visualisasi

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "unsoed/tk245004/kelompokwewenroland/data"   # ganti sesuai topicData pada ESP32
JUMLAH_SAMPEL = 30                            # jumlah data yang dikumpulkan

data = []                                     # penampung data yang diterima


def on_connect(client, userdata, flags, reason_code, properties):
    print("Terhubung ke broker, kode:", reason_code)
    client.subscribe(TOPIC)                   # berlangganan ke topic data


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())   # decode bytes -> JSON
        suhu = float(payload["suhu"])                # ambil nilai suhu
    except (ValueError, KeyError):                   # JSON rusak / key tidak ada
        print("Data tidak valid, diabaikan")
        return

    data.append({"waktu": time.strftime("%Y-%m-%d %H:%M:%S"), "suhu": suhu})
    print(f"[{len(data)}/{JUMLAH_SAMPEL}] suhu = {suhu}")

    if len(data) >= JUMLAH_SAMPEL:            # target tercapai -> berhenti
        client.disconnect()


# --- Tahap 1: akuisisi data dari MQTT ---
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_forever()                         # blok sampai disconnect()

# --- Tahap 2: simpan ke CSV ---
df = pd.DataFrame(data)
df.to_csv("data_suhu.csv", index=False)
print("Data tersimpan di data_suhu.csv")

# --- Tahap 3: analisis statistik dan deteksi anomali (z-score) ---
rata2 = df["suhu"].mean()
std = df["suhu"].std()

if std == 0:                                  # hindari pembagian dengan nol
    df["zscore"] = 0.0
else:
    df["zscore"] = (df["suhu"] - rata2) / std
df["anomali"] = df["zscore"].abs() > 2        # anomali jika |z| > 2

print(f"Rata-rata : {rata2:.2f}")
print(f"Std deviasi: {std:.2f}")
print(f"Minimum : {df['suhu'].min():.2f}")
print(f"Maksimum : {df['suhu'].max():.2f}")
print(f"Jumlah anomali: {df['anomali'].sum()}")

# --- Tahap 4: visualisasi ---
plt.figure(figsize=(10, 4))
plt.plot(df.index, df["suhu"], marker="o", label="Suhu")
anom = df[df["anomali"]]
plt.scatter(anom.index, anom["suhu"], color="red", s=100, label="Anomali", zorder=3)
plt.axhline(rata2, color="gray", linestyle="--", label="Rata-rata")
plt.xlabel("Urutan data")
plt.ylabel("Suhu (°C)")
plt.title("Data Suhu dan Deteksi Anomali")
plt.legend()
plt.tight_layout()
plt.savefig("grafik_suhu.png")
plt.show()