#include "LogikaFuzzy.h"

// Rumus sama dengan trapmf/trimf MATLAB. Sisi tegak (a == b atau c == d)
// bernilai 1 tepat di titiknya, jadi trapesium(0, 0, 10, 20) bernilai 1 di x = 0.
float Himpunan::derajat(float x) const {
  if (x < a || x > d) return 0;
  if (x < b) return (x - a) / (b - a);
  if (x <= c) return 1;
  return (d - x) / (d - c);
}

// Centroid diskret: jumlah(x * mu(x)) / jumlah(mu(x)) pada titik merata
// x(t) = min + langkah * t, mu(x) = max_j min(alfa_j, derajat_j(x)).
// Hasil sama dengan menghitung derajat() di setiap titik untuk setiap himpunan,
// tetapi lebih cepat:
// - hanya himpunan yang terkena aturan (alfa > 0) dan hanya titik di dalam alasnya;
// - batas sisi tiap himpunan diubah sekali menjadi nomor titik, jadi di dalam
//   loop cukup pembanding bilangan bulat, dan sisi miring menjadi p + q * t
//   (tanpa pembagian dan tanpa pembanding float);
// - himpunan yang potongannya (alfa) tidak melebihi mu sementara dilewati.
void logikafuzzy::centroid(const Himpunan *hp, uint8_t jumlah, const float *alfa, float min, float max,
                           uint16_t resolusi, Potong *h, float &atas, float &bawah) {
  const uint16_t akhir = resolusi - 1;
  const float langkah = (max - min) / akhir;
  auto titik = [&](uint16_t t) { return t == akhir ? max : min + langkah * t; };
  // Titik pertama dengan x(t) >= nilai (lebih = false) atau x(t) > nilai (lebih = true);
  // akhir + 1 jika tidak ada. Dibandingkan dengan x(t) asli agar sisi tegak tepat.
  auto cari = [&](float nilai, bool lebih) -> uint16_t {
    float f = langkah > 0 ? (nilai - min) / langkah : 0;
    uint16_t t = f <= 0 ? 0 : f >= akhir ? akhir : (uint16_t)f;
    while (t > 0 && (lebih ? titik(t - 1) > nilai : titik(t - 1) >= nilai)) t--;
    while (t <= akhir && !(lebih ? titik(t) > nilai : titik(t) >= nilai)) t++;
    return t;
  };
  uint8_t n = 0;
  uint16_t t0 = akhir + 1, t1 = 0;
  for (uint8_t j = 0; j < jumlah; j++) {
    if (alfa[j] <= 0) continue;
    const Himpunan &s = hp[j];
    Potong &p = h[n++];
    p.a = cari(s.a, false);
    p.b = cari(s.b, false);
    p.c = cari(s.c, true);
    p.d = cari(s.d, true);
    p.alfa = alfa[j];
    float naik = s.b > s.a ? 1 / (s.b - s.a) : 0, turun = s.d > s.c ? 1 / (s.d - s.c) : 0;
    p.p1 = (min - s.a) * naik;
    p.q1 = langkah * naik;
    p.p2 = (s.d - min) * turun;
    p.q2 = -langkah * turun;
    if (p.a < t0) t0 = p.a;
    if (p.d > t1) t1 = p.d;
  }
  float jumlahTMu = 0; // jumlah(t * mu); jumlah(x * mu) = min * jumlah(mu) + langkah * jumlah(t * mu)
  for (uint16_t t = t0; t < t1; t++) {
    float tf = t, mu = 0;
    for (uint8_t i = 0; i < n; i++) {
      const Potong &p = h[i];
      if (t < p.a || t >= p.d || p.alfa <= mu) continue;
      float d = t < p.b ? p.p1 + p.q1 * tf : t < p.c ? 1 : p.p2 + p.q2 * tf;
      if (d > p.alfa) d = p.alfa;
      if (d > mu) mu = d;
    }
    jumlahTMu += tf * mu;
    bawah += mu;
  }
  atas = min * bawah + langkah * jumlahTMu;
}
