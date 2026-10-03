// Mencetak setiap tahap perhitungan fuzzy Mamdani di Serial Monitor (115200)
// untuk dicocokkan dengan hitungan manual atau MATLAB di laporan skripsi.
// Tidak butuh sensor: nilai masukan ditulis langsung di bawah.
//
// Di MATLAB (Fuzzy Logic Designer), pakai pengaturan bawaan Mamdani:
// And = min, Or = max, Implication = min, Aggregation = max, Defuzz = centroid.
#include <LogikaFuzzy.h>

const float NILAI_SUHU = 18;   // °C
const float NILAI_LEMBAP = 40; // %

LogikaFuzzy<2, 1, 2, 4> fuzzy;
uint8_t suhu, lembap, kipas;
uint8_t DINGIN, PANAS, KERING, BASAH, PELAN, CEPAT;

void cetakDerajat(const char *nama, uint8_t himpunan) {
  Serial.print("  mu ");
  Serial.print(nama);
  Serial.print(" = ");
  Serial.println(fuzzy.derajat(himpunan), 4);
}

void setup() {
  Serial.begin(115200);

  suhu = fuzzy.tambahMasukan(0, 40);
  lembap = fuzzy.tambahMasukan(0, 100);
  kipas = fuzzy.tambahKeluaran(0, 100);

  DINGIN = fuzzy.tambahHimpunan(suhu, trapesium(0, 0, 10, 30));
  PANAS = fuzzy.tambahHimpunan(suhu, trapesium(10, 30, 40, 40));
  KERING = fuzzy.tambahHimpunan(lembap, trapesium(0, 0, 30, 70));
  BASAH = fuzzy.tambahHimpunan(lembap, trapesium(30, 70, 100, 100));
  PELAN = fuzzy.tambahHimpunan(kipas, trapesium(0, 0, 20, 60));
  CEPAT = fuzzy.tambahHimpunan(kipas, trapesium(40, 80, 100, 100));

  fuzzy.jika(DINGIN).dan(KERING).maka(PELAN);  // R1
  fuzzy.jika(DINGIN).dan(BASAH).maka(PELAN);   // R2
  fuzzy.jika(PANAS).dan(KERING).maka(CEPAT);   // R3
  fuzzy.jika(PANAS).dan(BASAH).maka(CEPAT);    // R4

  fuzzy.masukan(suhu, NILAI_SUHU);
  fuzzy.masukan(lembap, NILAI_LEMBAP);
  fuzzy.hitung();

  Serial.println("1. Fuzzifikasi");
  cetakDerajat("DINGIN", DINGIN);
  cetakDerajat("PANAS", PANAS);
  cetakDerajat("KERING", KERING);
  cetakDerajat("BASAH", BASAH);

  Serial.println("2. Kekuatan aturan (DAN = min)");
  for (uint8_t i = 0; i < fuzzy.jumlahAturan(); i++) {
    Serial.print("  R");
    Serial.print(i + 1);
    Serial.print(" = ");
    Serial.println(fuzzy.kekuatan(i), 4);
  }

  Serial.println("3. Komposisi (max per himpunan keluaran)");
  cetakDerajat("PELAN", PELAN);
  cetakDerajat("CEPAT", CEPAT);

  Serial.println("4. Defuzzifikasi centroid (101 titik)");
  Serial.print("  kipas = ");
  Serial.println(fuzzy.keluaran(kipas), 4);
}

void loop() {}
