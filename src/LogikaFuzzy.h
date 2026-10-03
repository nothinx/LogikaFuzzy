// LogikaFuzzy - logika fuzzy Mamdani dan Sugeno orde-0 tanpa alokasi dinamis.
// Aturan ditulis seperti di laporan: jika(DINGIN).dan(KERING).maka(PELAN).
// Copyright (c) 2026 Amadeo Wisesa. Lisensi MIT.
//
// - Kapasitas tetap lewat template: LogikaFuzzy<masukan, keluaran, himpunan, aturan>.
// - Metode sama dengan bawaan MATLAB (Mamdani): DAN = min, ATAU = max,
//   implikasi min, agregasi max, centroid diskret 101 titik, masukan dipotong
//   ke semesta. Hasil bisa dicocokkan dengan hitungan manual atau MATLAB.
// - Setiap tahap bisa dibaca: derajat() untuk fuzzifikasi, kekuatan() untuk aturan.
#pragma once
#include <Arduino.h>

// Himpunan fuzzy berbentuk trapesium a <= b <= c <= d. Segitiga: b == c.
struct Himpunan {
  float a, b, c, d;
  float derajat(float x) const; // derajat keanggotaan x, 0..1
};

inline Himpunan segitiga(float a, float b, float c) { return Himpunan{a, b, b, c}; }
inline Himpunan trapesium(float a, float b, float c, float d) { return Himpunan{a, b, c, d}; }
// Keluaran Sugeno orde-0: nilai tetap.
inline Himpunan konstanta(float k) { return Himpunan{k, k, k, k}; }

const uint8_t TIDAK_ADA = 0xFF; // dikembalikan fungsi tambah...() saat kapasitas penuh

template <uint8_t MASUKAN, uint8_t KELUARAN, uint8_t HIMPUNAN, uint8_t ATURAN>
class LogikaFuzzy {
  static_assert(MASUKAN && KELUARAN && HIMPUNAN && ATURAN, "kapasitas tidak boleh 0");
  static_assert((MASUKAN + KELUARAN) * HIMPUNAN < 255, "terlalu banyak himpunan");

  struct Variabel {
    float min, max;
    uint8_t jumlah;
    bool sugeno;
    Himpunan h[HIMPUNAN];
  };
  struct Aturan {
    uint8_t syarat[MASUKAN]; // nomor himpunan + 1 per masukan, 0 = tidak dipakai
    uint8_t keluaran;        // id himpunan keluaran
    uint8_t atau;            // 0 = DAN, 1 = ATAU, 2 = belum ditentukan
  };

public:
  // Dikembalikan jika(); tidak perlu dipakai langsung.
  class Pembuat {
  public:
    Pembuat &dan(uint8_t himpunan) { return tambah(himpunan, 0); }
    Pembuat &atau(uint8_t himpunan) { return tambah(himpunan, 1); }
    // Menyimpan aturan. false jika aturan penuh atau tidak sah.
    bool maka(uint8_t himpunan) {
      uint8_t v = himpunan / HIMPUNAN;
      if (!_sah || !_f.adaHimpunan(himpunan) || v < MASUKAN || _f._jumlahAturan >= ATURAN) return false;
      _a.keluaran = himpunan;
      if (_a.atau == 2) _a.atau = 0;
      _f._aturan[_f._jumlahAturan++] = _a;
      return true;
    }

  private:
    friend class LogikaFuzzy;
    Pembuat(LogikaFuzzy &f, uint8_t himpunan) : _f(f), _a(), _sah(true) {
      _a.atau = 2;
      tambah(himpunan, 2);
    }
    Pembuat &tambah(uint8_t himpunan, uint8_t atau) {
      uint8_t v = himpunan / HIMPUNAN;
      // Hanya himpunan masukan, satu kali per masukan, dan satu jenis operator per aturan.
      if (!_f.adaHimpunan(himpunan) || v >= MASUKAN || _a.syarat[v] ||
          (atau != 2 && _a.atau != 2 && _a.atau != atau)) {
        _sah = false;
      } else {
        _a.syarat[v] = himpunan % HIMPUNAN + 1;
        if (atau != 2) _a.atau = atau;
      }
      return *this;
    }
    LogikaFuzzy &_f;
    Aturan _a;
    bool _sah;
  };

  // --- Menyusun sistem (di setup) ---
  // Mengembalikan id variabel, atau TIDAK_ADA jika penuh.
  uint8_t tambahMasukan(float min, float max) {
    if (_jumlahMasukan >= MASUKAN) return TIDAK_ADA;
    return buatVariabel(_jumlahMasukan++, min, max, false);
  }
  // Keluaran Mamdani dengan semesta min..max.
  uint8_t tambahKeluaran(float min, float max) {
    if (_jumlahKeluaran >= KELUARAN) return TIDAK_ADA;
    return buatVariabel(MASUKAN + _jumlahKeluaran++, min, max, false);
  }
  // Keluaran Sugeno orde-0: himpunannya konstanta(k).
  uint8_t tambahKeluaranSugeno() {
    if (_jumlahKeluaran >= KELUARAN) return TIDAK_ADA;
    return buatVariabel(MASUKAN + _jumlahKeluaran++, 0, 0, true);
  }
  // Mengembalikan id himpunan untuk aturan, atau TIDAK_ADA jika penuh.
  uint8_t tambahHimpunan(uint8_t variabel, Himpunan h) {
    if (!adaVariabel(variabel)) return TIDAK_ADA;
    Variabel &v = _var[variabel];
    if (v.jumlah >= HIMPUNAN) return TIDAK_ADA;
    v.h[v.jumlah] = h;
    return variabel * HIMPUNAN + v.jumlah++;
  }
  // Awal aturan: jika(A).dan(B).maka(C) atau jika(A).atau(B).maka(C).
  Pembuat jika(uint8_t himpunan) { return Pembuat(*this, himpunan); }

  // --- Menghitung (di loop) ---
  // Nilai di luar semesta dipotong ke batasnya. false jika variabel bukan masukan.
  bool masukan(uint8_t variabel, float nilai) {
    if (variabel >= _jumlahMasukan) return false;
    const Variabel &v = _var[variabel];
    _nilai[variabel] = nilai < v.min ? v.min : nilai > v.max ? v.max : nilai;
    return true;
  }
  // false jika ada keluaran yang tidak terkena aturan apa pun (hasilnya titik
  // tengah semesta untuk Mamdani, 0 untuk Sugeno).
  bool hitung();
  float keluaran(uint8_t variabel) const { // hasil hitung() terakhir
    return variabel >= MASUKAN && adaVariabel(variabel) ? _hasil[variabel - MASUKAN] : 0;
  }

  // --- Membaca tiap tahap (untuk laporan) ---
  // Himpunan masukan: derajat keanggotaan nilai masukan saat ini.
  // Himpunan keluaran: derajat terbesar dari aturan yang menuju himpunan itu.
  float derajat(uint8_t himpunan) const;
  float kekuatan(uint8_t aturan) const; // 0 = aturan pertama
  uint8_t jumlahAturan() const { return _jumlahAturan; }

  // Jumlah titik centroid (default 101, sama dengan MATLAB). Minimal 2.
  void aturResolusi(uint16_t titik) { _resolusi = titik < 2 ? 2 : titik; }

private:
  uint8_t buatVariabel(uint8_t i, float min, float max, bool sugeno) {
    _var[i].min = min;
    _var[i].max = max;
    _var[i].sugeno = sugeno;
    if (i < MASUKAN) _nilai[i] = min;
    return i;
  }
  bool adaVariabel(uint8_t v) const {
    return v < _jumlahMasukan || (v >= MASUKAN && v < MASUKAN + _jumlahKeluaran);
  }
  bool adaHimpunan(uint8_t h) const {
    return h != TIDAK_ADA && adaVariabel(h / HIMPUNAN) && h % HIMPUNAN < _var[h / HIMPUNAN].jumlah;
  }

  Variabel _var[MASUKAN + KELUARAN] = {};
  Aturan _aturan[ATURAN];
  float _nilai[MASUKAN] = {};
  float _hasil[KELUARAN] = {};
  uint16_t _resolusi = 101;
  uint8_t _jumlahMasukan = 0, _jumlahKeluaran = 0, _jumlahAturan = 0;
};

template <uint8_t M, uint8_t K, uint8_t H, uint8_t A>
float LogikaFuzzy<M, K, H, A>::kekuatan(uint8_t i) const {
  if (i >= _jumlahAturan) return 0;
  const Aturan &r = _aturan[i];
  float w = r.atau ? 0 : 1;
  for (uint8_t v = 0; v < M; v++) {
    if (!r.syarat[v]) continue;
    float d = _var[v].h[r.syarat[v] - 1].derajat(_nilai[v]);
    w = r.atau ? (d > w ? d : w) : (d < w ? d : w);
  }
  return w;
}

template <uint8_t M, uint8_t K, uint8_t H, uint8_t A>
float LogikaFuzzy<M, K, H, A>::derajat(uint8_t h) const {
  if (!adaHimpunan(h)) return 0;
  uint8_t v = h / H;
  if (v < M) return _var[v].h[h % H].derajat(_nilai[v]);
  float d = 0;
  for (uint8_t i = 0; i < _jumlahAturan; i++) {
    if (_aturan[i].keluaran != h) continue;
    float w = kekuatan(i);
    if (w > d) d = w;
  }
  return d;
}

template <uint8_t M, uint8_t K, uint8_t H, uint8_t A>
bool LogikaFuzzy<M, K, H, A>::hitung() {
  bool semua = true;
  for (uint8_t k = 0; k < _jumlahKeluaran; k++) {
    const Variabel &v = _var[M + k];
    float atas = 0, bawah = 0;
    if (v.sugeno) {
      // Rata-rata berbobot: jumlah(w * z) / jumlah(w), satu suku per aturan.
      for (uint8_t i = 0; i < _jumlahAturan; i++) {
        uint8_t h = _aturan[i].keluaran;
        if (h / H != M + k) continue;
        float w = kekuatan(i);
        atas += w * v.h[h % H].a;
        bawah += w;
      }
    } else {
      float alfa[H];
      for (uint8_t j = 0; j < v.jumlah; j++) alfa[j] = derajat((M + k) * H + j);
      // Centroid diskret: jumlah(x * mu(x)) / jumlah(mu(x)) pada titik merata.
      for (uint16_t t = 0; t < _resolusi; t++) {
        float x = v.min + (v.max - v.min) * t / (_resolusi - 1);
        float mu = 0;
        for (uint8_t j = 0; j < v.jumlah; j++) {
          float d = v.h[j].derajat(x);
          if (d > alfa[j]) d = alfa[j];
          if (d > mu) mu = d;
        }
        atas += x * mu;
        bawah += mu;
      }
    }
    if (bawah > 0) {
      _hasil[k] = atas / bawah;
    } else {
      _hasil[k] = (v.min + v.max) / 2;
      semua = false;
    }
  }
  return semua;
}
