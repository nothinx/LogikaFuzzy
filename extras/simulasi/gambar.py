"""Compile + jalankan simulasi.cpp (kode library asli), lalu render grafik ke ../gambar/.

Jalankan dari folder ini:  python gambar.py   (butuh g++ dan matplotlib)
"""
import glob
import os
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

plt.rcParams.update({
    "figure.figsize": (8, 3.6), "figure.dpi": 100, "savefig.bbox": "tight", "savefig.pad_inches": 0.15,
    "figure.facecolor": "white", "axes.facecolor": "white", "savefig.facecolor": "white",
    "font.size": 10, "axes.titlesize": 11, "axes.titleweight": "bold", "axes.titlelocation": "left",
    "axes.spines.top": False, "axes.spines.right": False, "axes.edgecolor": "#9ca3af",
    "axes.grid": True, "grid.color": "#e5e7eb", "grid.linewidth": 0.8,
    "legend.frameon": False, "svg.fonttype": "path", "svg.hashsalt": "nothinx",
    "lines.linewidth": 1.8,
})
WARNA = {"utama": "#2563eb", "pembanding": "#dc2626", "ketiga": "#16a34a", "keempat": "#9333ea",
         "kelima": "#ea580c", "mentah": "#9ca3af", "target": "#111827"}
# Urutan himpunan rendah/tengah/tinggi, dipakai sama di masukan dan keluaran.
TIGA = [WARNA["utama"], WARNA["ketiga"], WARNA["pembanding"]]

SINI = os.path.dirname(os.path.abspath(__file__))
KELUAR = os.path.join(SINI, "..", "gambar")


def koma(x, d=1):
    return f"{x:.{d}f}".replace(".", ",")


def jalankan():
    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, "sim")
        src = glob.glob(os.path.join(SINI, "..", "..", "src", "*.cpp"))
        hasil = subprocess.run(["g++", "-std=c++11", "-O2", "-I../test", "-I../../src", "simulasi.cpp", *src,
                                "-o", exe], cwd=SINI, capture_output=True, text=True)
        if hasil.returncode:
            sys.exit(hasil.stderr)
        teks = subprocess.run([exe], check=True, capture_output=True, text=True).stdout
    data, nama = {}, None
    for baris in teks.splitlines():
        if baris.startswith("# "):
            nama, data[baris[2:]] = baris[2:], []
        else:
            data[nama].append(baris.split(","))
    # baris pertama tiap bagian = nama kolom
    return {k: {kol: np.array([float(r[i]) for r in v[1:]]) for i, kol in enumerate(v[0])} for k, v in data.items()}


def simpan(fig, nama):
    fig.savefig(os.path.join(KELUAR, nama), format="svg", metadata={"Date": None})
    plt.close(fig)


def label_himpunan(ax, x, ys, nama):
    for y, n, w in zip(ys, nama, TIGA):
        puncak = x[y >= y.max() - 1e-6]
        ax.text((puncak[0] + puncak[-1]) / 2, 1.04, n, color=w, ha="center", va="bottom", fontweight="bold")


def keanggotaan(d):
    m, k = d["keanggotaan_masukan"], d["keanggotaan_keluaran"]
    fig, (a1, a2) = plt.subplots(2, 1, figsize=(8, 5.4))
    for ax, x, ys, nama, sumbu in [
        (a1, m["suhu"], [m["dingin"], m["hangat"], m["panas"]], ["DINGIN", "HANGAT", "PANAS"], "Suhu (°C)"),
        (a2, k["pwm"], [k["mati"], k["sedang"], k["kencang"]], ["MATI", "SEDANG", "KENCANG"], "PWM kipas (0–255)"),
    ]:
        for y, w in zip(ys, TIGA):
            ax.plot(x, y, color=w)
        label_himpunan(ax, x, ys, nama)
        ax.set_xlim(x[0], x[-1])
        ax.set_ylim(0, 1.2)
        ax.set_yticks([0, 0.5, 1])
        ax.set_xlabel(sumbu)
        ax.set_ylabel("Derajat μ")
    a1.set_title("Masukan: tiga himpunan suhu saling tumpang tindih 4–5 °C")
    a2.set_title("Keluaran: tiga himpunan kecepatan kipas (contoh KipasOtomatis)")
    fig.tight_layout(h_pad=1.5)
    simpan(fig, "keanggotaan.svg")


def mamdani(d):
    c, m, k, g = d["contoh"], d["keanggotaan_masukan"], d["keanggotaan_keluaran"], d["agregasi"]
    suhu, pwm = c["suhu"][0], c["pwm"][0]
    mu = [c["mu_dingin"][0], c["mu_hangat"][0], c["mu_panas"][0]]
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(9, 3.6), gridspec_kw={"width_ratios": [1, 1.5]})

    for y, w in zip([m["dingin"], m["hangat"], m["panas"]], TIGA):
        a1.plot(m["suhu"], y, color=w, linewidth=1.2)
    label_himpunan(a1, m["suhu"], [m["dingin"], m["hangat"], m["panas"]], ["DINGIN", "HANGAT", "PANAS"])
    a1.axvline(suhu, color=WARNA["target"], linestyle="--", linewidth=1)
    for u, w in zip(mu, TIGA):
        if u > 0:
            a1.plot([suhu, 50], [u, u], color=w, linestyle=":", linewidth=1)
            a1.plot(suhu, u, "o", color=w, markersize=5)
            a1.text(49, u + 0.02, f"μ = {koma(u, 2)}", color=w, ha="right", va="bottom")
    a1.set_xlim(0, 50)
    a1.set_ylim(0, 1.2)
    a1.set_yticks([0, 0.5, 1])
    a1.set_xlabel("Suhu (°C)")
    a1.set_ylabel("Derajat μ")
    a1.set_title(f"1. Fuzzifikasi suhu {koma(suhu, 0)} °C")

    x = g["pwm"]
    for y, w in zip([k["mati"], k["sedang"], k["kencang"]], TIGA):
        a2.plot(k["pwm"], y, color=WARNA["mentah"], linewidth=1.0)
    for y, w in zip([g["mati"], g["sedang"], g["kencang"]], TIGA):
        a2.plot(x, y, color=w, linewidth=1.2)
    a2.fill_between(x, g["gabungan"], color=WARNA["utama"], alpha=0.25, linewidth=0)
    a2.plot(x, g["gabungan"], color=WARNA["utama"])
    a2.axvline(pwm, color=WARNA["utama"], linestyle="--", linewidth=1.5)
    a2.text(pwm + 4, 1.1, f"centroid = {koma(pwm)}", color=WARNA["utama"], fontweight="bold", va="center")
    # label di dalam area terarsir: tinggi potongan = kekuatan aturan
    a2.text(35, c["a_mati"][0] / 2, f"MATI\ndipotong {koma(c['a_mati'][0], 2)}", color=TIGA[0],
            ha="center", va="center")
    a2.text(150, c["a_sedang"][0] / 2, f"SEDANG\ndipotong {koma(c['a_sedang'][0], 2)}", color=TIGA[1],
            ha="center", va="center")
    a2.set_xlim(0, 255)
    a2.set_ylim(0, 1.2)
    a2.set_yticks([0, 0.5, 1])
    a2.set_xlabel("PWM kipas (0–255)")
    a2.set_title(f"2–3. Potong (min), gabung (max), centroid → PWM {koma(pwm)}")
    fig.tight_layout(w_pad=2)
    simpan(fig, "mamdani.svg")
    return suhu, mu, pwm


def kurva(d, contoh):
    k = d["kurva_kipas"]
    suhu, pwm = contoh
    fig, ax = plt.subplots()
    for a, b in [(22, 27), (30, 34)]:  # daerah dua himpunan masukan aktif bersamaan
        ax.axvspan(a, b, color=WARNA["mentah"], alpha=0.12, linewidth=0)
    ax.text(24.5, 245, "DINGIN+\nHANGAT", ha="center", va="top", color="#6b7280", fontsize=8)
    ax.text(32, 245, "HANGAT+\nPANAS", ha="center", va="top", color="#6b7280", fontsize=8)
    ax.plot(k["suhu"], k["pwm"], color=WARNA["utama"])
    ax.plot(suhu, pwm, "o", color=WARNA["utama"])
    ax.annotate(f"{koma(suhu, 0)} °C → {koma(pwm)}", (suhu, pwm), xytext=(-12, 0), textcoords="offset points",
                ha="right", va="center", color=WARNA["utama"])
    lo, hi = k["pwm"].min(), k["pwm"].max()
    ax.set_xlim(0, 50)
    ax.set_ylim(0, 255)
    ax.set_xlabel("Suhu (°C)")
    ax.set_ylabel("PWM kipas")
    ax.set_title(f"Kipas naik halus dari PWM {koma(lo)} ke {koma(hi)}, tidak pernah tepat 0 atau 255")
    simpan(fig, "kurva_kipas.svg")
    return lo, hi


def peta(d):
    p = d["peta_siram"]
    tanah, suhu = np.unique(p["tanah"]), np.unique(p["suhu"])
    z = p["detik"].reshape(len(tanah), len(suhu)).T  # baris = suhu
    fig, ax = plt.subplots(figsize=(8, 4))
    im = ax.imshow(z, origin="lower", aspect="auto", cmap="Blues", interpolation="nearest",
                   extent=[tanah[0] - 0.5, tanah[-1] + 0.5, suhu[0] - 0.25, suhu[-1] + 0.25])
    cs = ax.contour(tanah, suhu, z, levels=[3], colors=WARNA["target"], linestyles="--", linewidths=1.2)
    ax.clabel(cs, fmt="3 detik", fontsize=8)
    ax.grid(False)
    ax.set_xlabel("Kelembapan tanah (%)")
    ax.set_ylabel("Suhu (°C)")
    fig.colorbar(im, ax=ax, label="Lama siram (detik)")
    ax.set_title(f"Tanah kering + suhu hangat/panas → siram terlama ({koma(z.max())} detik)")
    simpan(fig, "peta_siram.svg")
    return z.min(), z.max()


def main():
    os.makedirs(KELUAR, exist_ok=True)
    d = jalankan()
    keanggotaan(d)
    suhu, mu, pwm = mamdani(d)
    lo, hi = kurva(d, (suhu, pwm))
    zmin, zmax = peta(d)
    print(f"contoh: suhu {suhu} -> mu {mu}, PWM {pwm:.4f}")
    print(f"kurva kipas: PWM {lo:.4f} .. {hi:.4f}")
    print(f"peta siram: {zmin:.3f} .. {zmax:.3f} detik")


if __name__ == "__main__":
    main()
