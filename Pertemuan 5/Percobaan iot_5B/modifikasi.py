# =====================================================
# Modul 5 - Percobaan 5B (Modifikasi)
# Moving average (jendela 5) pada data CSV sebelum z-score,
# lalu membandingkan jumlah anomali
# =====================================================

import pandas as pd
import matplotlib.pyplot as plt

JENDELA = 5                                   # [BARU] ukuran jendela moving average
BATAS_Z = 2                                   # [BARU] batas |z| untuk anomali

df = pd.read_csv("data_suhu.csv")             # [BARU] baca data hasil Tahap 2 (bukan dari MQTT lagi)


def hitung_zscore(seri):                      # [BARU] fungsi z-score agar bisa dipakai dua kali
    rata2 = seri.mean()
    std = seri.std()
    if std == 0:                              # hindari pembagian dengan nol
        return pd.Series(0.0, index=seri.index)
    return (seri - rata2) / std


# --- Metode A: z-score pada data mentah (cara awal) ---
df["z_mentah"] = hitung_zscore(df["suhu"])                    # [BARU]
df["anomali_mentah"] = df["z_mentah"].abs() > BATAS_Z         # [BARU]

# --- Metode B: moving average dulu, baru z-score ---
df["suhu_ma"] = df["suhu"].rolling(window=JENDELA, min_periods=1).mean()  # [BARU] rata-rata 5 data terakhir
df["z_ma"] = hitung_zscore(df["suhu_ma"])                     # [BARU] z-score dari data hasil filter
df["anomali_ma"] = df["z_ma"].abs() > BATAS_Z                 # [BARU]

# --- Perbandingan ---
print(f"Jumlah data              : {len(df)}")                                  # [BARU]
print(f"Anomali data mentah      : {df['anomali_mentah'].sum()}")               # [BARU]
print(f"Anomali setelah MA ({JENDELA}) : {df['anomali_ma'].sum()}")             # [BARU]
print(f"Std data mentah          : {df['suhu'].std():.4f}")                     # [BARU]
print(f"Std data setelah MA      : {df['suhu_ma'].std():.4f}")                  # [BARU]

df.to_csv("data_suhu_ma.csv", index=False)    # [BARU] simpan hasil lengkap
print("Hasil tersimpan di data_suhu_ma.csv")  # [BARU]

# --- Visualisasi perbandingan ---
fig, ax = plt.subplots(2, 1, figsize=(10, 7), sharex=True)    # [BARU] dua grafik atas-bawah

ax[0].plot(df.index, df["suhu"], marker="o", label="Suhu mentah")
a1 = df[df["anomali_mentah"]]
ax[0].scatter(a1.index, a1["suhu"], color="red", s=100, label="Anomali", zorder=3)
ax[0].axhline(df["suhu"].mean(), color="gray", linestyle="--", label="Rata-rata")
ax[0].set_title("Z-score pada data mentah")
ax[0].set_ylabel("Suhu (°C)")
ax[0].legend()

ax[1].plot(df.index, df["suhu_ma"], marker="o", color="green", label=f"Suhu MA ({JENDELA})")
a2 = df[df["anomali_ma"]]
ax[1].scatter(a2.index, a2["suhu_ma"], color="red", s=100, label="Anomali", zorder=3)
ax[1].axhline(df["suhu_ma"].mean(), color="gray", linestyle="--", label="Rata-rata")
ax[1].set_title(f"Z-score setelah moving average (jendela {JENDELA})")
ax[1].set_xlabel("Urutan data")
ax[1].set_ylabel("Suhu (°C)")
ax[1].legend()

plt.tight_layout()
plt.savefig("grafik_perbandingan.png")        # [BARU]
plt.show()