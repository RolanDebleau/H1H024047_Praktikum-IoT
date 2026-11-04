import pandas as pd

df = pd.read_csv("data_suhu.csv")
rata2 = df["suhu"].mean()
std = df["suhu"].std()

df["zscore"] = 0.0 if std == 0 else (df["suhu"] - rata2) / std
df["anomali"] = df["zscore"].abs() > 2

tabel = pd.DataFrame({
    "No": range(1, len(df) + 1),
    "Waktu Penerimaan": df["waktu"],
    "Suhu (°C)": df["suhu"],
    "Z-score": df["zscore"].round(2),
    "Status Anomali": df["anomali"].map({True: "Anomali", False: "Normal"}),
    "Status Penyimpanan CSV": "Tersimpan",
})

print(tabel.head(20).to_string(index=False))
tabel.head(20).to_csv("tabel_5_2.csv", index=False)