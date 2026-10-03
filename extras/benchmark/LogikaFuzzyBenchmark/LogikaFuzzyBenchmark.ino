// Benchmark LogikaFuzzy di simulator simavr (ATmega328P 16 MHz, akurat per siklus).
// Siklus.h mengukur dengan Timer1; tiap baris keluaran: "BENCH nama siklus_per_panggilan".
// Sistem sama persis dengan contoh KipasOtomatis dan PenyiramTanaman.
#include <LogikaFuzzy.h>
#include "Siklus.h"

LogikaFuzzy<1, 1, 3, 3> kipasF;
LogikaFuzzy<2, 1, 3, 9> siramF;
uint8_t suhu, kipas, tanah, suhu2, siram;
volatile float masuk1 = 25, masuk2 = 40, masuk3 = 27, hasil;

void susun() {
  suhu = kipasF.tambahMasukan(0, 50);
  kipas = kipasF.tambahKeluaran(0, 255);
  uint8_t DINGIN = kipasF.tambahHimpunan(suhu, trapesium(0, 0, 20, 27));
  uint8_t HANGAT = kipasF.tambahHimpunan(suhu, segitiga(22, 28, 34));
  uint8_t PANAS = kipasF.tambahHimpunan(suhu, trapesium(30, 37, 50, 50));
  uint8_t MATI = kipasF.tambahHimpunan(kipas, trapesium(0, 0, 30, 90));
  uint8_t SEDANG = kipasF.tambahHimpunan(kipas, segitiga(60, 140, 220));
  uint8_t KENCANG = kipasF.tambahHimpunan(kipas, trapesium(180, 230, 255, 255));
  kipasF.jika(DINGIN).maka(MATI);
  kipasF.jika(HANGAT).maka(SEDANG);
  kipasF.jika(PANAS).maka(KENCANG);

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

void cetak(const __FlashStringHelper *nama, float nilai) {
  Serial.print(F("BENCH "));
  Serial.print(nama);
  Serial.print(' ');
  Serial.println(nilai, 4);
}

void setup() {
  Serial.begin(115200);
  susun();

  // KipasOtomatis, suhu 25 °C: 2 dari 3 aturan aktif.
  kipasF.masukan(suhu, masuk1);
  UKUR("kipas_hitung_25C", 20, kipasF.hitung());
  cetak(F("kipas_hasil_25C"), kipasF.keluaran(kipas));
  // Suhu 10 °C: hanya 1 aturan aktif.
  kipasF.masukan(suhu, 10);
  UKUR("kipas_hitung_10C", 20, kipasF.hitung());
  UKUR("kipas_masukan", 1000, kipasF.masukan(suhu, masuk1));

  // PenyiramTanaman, tanah 40 %, suhu 27 °C: 4 dari 9 aturan aktif.
  siramF.masukan(tanah, masuk2);
  siramF.masukan(suhu2, masuk3);
  UKUR("siram_hitung", 20, siramF.hitung());
  cetak(F("siram_hasil"), siramF.keluaran(siram));
  UKUR("siram_keluaran", 1000, hasil = siramF.keluaran(siram));

  cetak(F("ram_objek_kipas"), sizeof(kipasF));
  cetak(F("ram_objek_siram"), sizeof(siramF));
  selesai();
}

void loop() {}
