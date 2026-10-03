// Simulasi LogikaFuzzy di PC: memakai kode library asli (../../src) dan
// mencetak data untuk gambar.py. Sistem sama persis dengan contoh
// KipasOtomatis dan PenyiramTanaman.
//   g++ -std=c++11 -O2 -I../test -I../../src simulasi.cpp ../../src/*.cpp -o sim && ./sim
// Keluaran: bagian diawali "# nama", lalu baris CSV.
#include <stdio.h>
#include "LogikaFuzzy.h"

// --- KipasOtomatis: suhu 0..50 °C -> PWM 0..255 ---
LogikaFuzzy<1, 1, 3, 3> kipasF;
uint8_t suhu, kipas, DINGIN, HANGAT, PANAS, MATI, SEDANG, KENCANG;

void susunKipas() {
  suhu = kipasF.tambahMasukan(0, 50);
  kipas = kipasF.tambahKeluaran(0, 255);
  DINGIN = kipasF.tambahHimpunan(suhu, trapesium(0, 0, 20, 27));
  HANGAT = kipasF.tambahHimpunan(suhu, segitiga(22, 28, 34));
  PANAS = kipasF.tambahHimpunan(suhu, trapesium(30, 37, 50, 50));
  MATI = kipasF.tambahHimpunan(kipas, trapesium(0, 0, 30, 90));
  SEDANG = kipasF.tambahHimpunan(kipas, segitiga(60, 140, 220));
  KENCANG = kipasF.tambahHimpunan(kipas, trapesium(180, 230, 255, 255));
  kipasF.jika(DINGIN).maka(MATI);
  kipasF.jika(HANGAT).maka(SEDANG);
  kipasF.jika(PANAS).maka(KENCANG);
}

// --- PenyiramTanaman: tanah 0..100 % + suhu 0..45 °C -> siram 0..30 detik ---
LogikaFuzzy<2, 1, 3, 9> siramF;
uint8_t tanah, suhu2, siram;

void susunSiram() {
  tanah = siramF.tambahMasukan(0, 100);
  suhu2 = siramF.tambahMasukan(0, 45);
  siram = siramF.tambahKeluaran(0, 30);
  uint8_t KERING = siramF.tambahHimpunan(tanah, trapesium(0, 0, 25, 45));
  uint8_t LEMBAP = siramF.tambahHimpunan(tanah, segitiga(30, 50, 70));
  uint8_t BASAH = siramF.tambahHimpunan(tanah, trapesium(55, 75, 100, 100));
  uint8_t SEJUK = siramF.tambahHimpunan(suhu2, trapesium(0, 0, 22, 28));
  uint8_t NORMAL = siramF.tambahHimpunan(suhu2, segitiga(24, 29, 34));
  uint8_t PANAS2 = siramF.tambahHimpunan(suhu2, trapesium(30, 36, 45, 45));
  uint8_t TIDAK = siramF.tambahHimpunan(siram, trapesium(0, 0, 2, 6));
  uint8_t SEBENTAR = siramF.tambahHimpunan(siram, segitiga(4, 10, 18));
  uint8_t LAMA = siramF.tambahHimpunan(siram, trapesium(14, 22, 30, 30));
  siramF.jika(KERING).dan(SEJUK).maka(SEBENTAR);
  siramF.jika(KERING).dan(NORMAL).maka(LAMA);
  siramF.jika(KERING).dan(PANAS2).maka(LAMA);
  siramF.jika(LEMBAP).dan(SEJUK).maka(TIDAK);
  siramF.jika(LEMBAP).dan(NORMAL).maka(SEBENTAR);
  siramF.jika(LEMBAP).dan(PANAS2).maka(SEBENTAR);
  siramF.jika(BASAH).dan(SEJUK).maka(TIDAK);
  siramF.jika(BASAH).dan(NORMAL).maka(TIDAK);
  siramF.jika(BASAH).dan(PANAS2).maka(TIDAK);
}

// Himpunan disimpan di dalam library; untuk menggambar bentuknya kita
// buat ulang dengan parameter yang sama dan pakai Himpunan::derajat() asli.
const Himpunan HM[3] = {trapesium(0, 0, 20, 27), segitiga(22, 28, 34), trapesium(30, 37, 50, 50)};
const Himpunan HK[3] = {trapesium(0, 0, 30, 90), segitiga(60, 140, 220), trapesium(180, 230, 255, 255)};

int main() {
  susunKipas();
  susunSiram();

  printf("# keanggotaan_masukan\nsuhu,dingin,hangat,panas\n");
  for (int i = 0; i <= 500; i++) {
    float x = i * 0.1f;
    printf("%.1f,%.4f,%.4f,%.4f\n", x, HM[0].derajat(x), HM[1].derajat(x), HM[2].derajat(x));
  }
  printf("# keanggotaan_keluaran\npwm,mati,sedang,kencang\n");
  for (int x = 0; x <= 255; x++)
    printf("%d,%.4f,%.4f,%.4f\n", x, HK[0].derajat(x), HK[1].derajat(x), HK[2].derajat(x));

  // Satu masukan contoh: setiap tahap dibaca lewat derajat() dan keluaran().
  const float CONTOH = 25; // DINGIN dan HANGAT sama-sama aktif
  kipasF.masukan(suhu, CONTOH);
  kipasF.hitung();
  printf("# contoh\nsuhu,mu_dingin,mu_hangat,mu_panas,a_mati,a_sedang,a_kencang,pwm\n");
  printf("%.1f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", CONTOH, kipasF.derajat(DINGIN), kipasF.derajat(HANGAT),
         kipasF.derajat(PANAS), kipasF.derajat(MATI), kipasF.derajat(SEDANG), kipasF.derajat(KENCANG),
         kipasF.keluaran(kipas));
  // Himpunan keluaran yang terpotong (implikasi min) dan agregasinya (max),
  // sama dengan yang dihitung hitung() di tiap titik centroid.
  printf("# agregasi\npwm,mati,sedang,kencang,gabungan\n");
  const uint8_t HK_ID[3] = {MATI, SEDANG, KENCANG};
  for (int x = 0; x <= 255; x++) {
    float mu[3], maks = 0;
    for (int j = 0; j < 3; j++) {
      float a = kipasF.derajat(HK_ID[j]), d = HK[j].derajat(x);
      mu[j] = d < a ? d : a;
      if (mu[j] > maks) maks = mu[j];
    }
    printf("%d,%.4f,%.4f,%.4f,%.4f\n", x, mu[0], mu[1], mu[2], maks);
  }

  printf("# kurva_kipas\nsuhu,pwm\n");
  for (int i = 0; i <= 500; i++) {
    kipasF.masukan(suhu, i * 0.1f);
    kipasF.hitung();
    printf("%.1f,%.4f\n", i * 0.1f, kipasF.keluaran(kipas));
  }

  printf("# peta_siram\ntanah,suhu,detik\n");
  for (int t = 0; t <= 100; t++)
    for (int s = 0; s <= 90; s++) {
      siramF.masukan(tanah, t);
      siramF.masukan(suhu2, s * 0.5f);
      siramF.hitung();
      printf("%d,%.1f,%.3f\n", t, s * 0.5f, siramF.keluaran(siram));
    }
  return 0;
}
