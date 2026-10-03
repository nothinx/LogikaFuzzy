// Penyiram tanaman: kelembapan tanah + suhu -> lama penyiraman (detik).
// 2 masukan x 3 himpunan = 9 aturan, metode Mamdani.
//
// Sambungan: sensor kelembapan tanah ke A0, LM35 ke A3, relay pompa ke pin 7.
// Kalibrasi sensor tanah: catat nilai ADC saat kering (di udara) dan basah
// (di dalam air), lalu isi ADC_KERING dan ADC_BASAH.
#include <LogikaFuzzy.h>

LogikaFuzzy<2, 1, 3, 9> fuzzy;
uint8_t tanah, suhu, siram;

const uint8_t PIN_POMPA = 7;
const int ADC_KERING = 850, ADC_BASAH = 400;
const uint32_t JEDA_CEK = 60000; // cek tanah tiap 1 menit

bool menyiram = false;
uint32_t waktu = 0, lama = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_POMPA, OUTPUT);

  tanah = fuzzy.tambahMasukan(0, 100);  // kelembapan tanah, %
  suhu = fuzzy.tambahMasukan(0, 45);    // °C
  siram = fuzzy.tambahKeluaran(0, 30);  // lama siram, detik

  uint8_t KERING = fuzzy.tambahHimpunan(tanah, trapesium(0, 0, 25, 45));
  uint8_t LEMBAP = fuzzy.tambahHimpunan(tanah, segitiga(30, 50, 70));
  uint8_t BASAH = fuzzy.tambahHimpunan(tanah, trapesium(55, 75, 100, 100));

  uint8_t SEJUK = fuzzy.tambahHimpunan(suhu, trapesium(0, 0, 22, 28));
  uint8_t NORMAL = fuzzy.tambahHimpunan(suhu, segitiga(24, 29, 34));
  uint8_t PANAS = fuzzy.tambahHimpunan(suhu, trapesium(30, 36, 45, 45));

  uint8_t TIDAK = fuzzy.tambahHimpunan(siram, trapesium(0, 0, 2, 6));
  uint8_t SEBENTAR = fuzzy.tambahHimpunan(siram, segitiga(4, 10, 18));
  uint8_t LAMA = fuzzy.tambahHimpunan(siram, trapesium(14, 22, 30, 30));

  // Tabel aturan (baris = tanah, kolom = suhu):
  //            SEJUK     NORMAL    PANAS
  //   KERING   SEBENTAR  LAMA      LAMA
  //   LEMBAP   TIDAK     SEBENTAR  SEBENTAR
  //   BASAH    TIDAK     TIDAK     TIDAK
  fuzzy.jika(KERING).dan(SEJUK).maka(SEBENTAR);
  fuzzy.jika(KERING).dan(NORMAL).maka(LAMA);
  fuzzy.jika(KERING).dan(PANAS).maka(LAMA);
  fuzzy.jika(LEMBAP).dan(SEJUK).maka(TIDAK);
  fuzzy.jika(LEMBAP).dan(NORMAL).maka(SEBENTAR);
  fuzzy.jika(LEMBAP).dan(PANAS).maka(SEBENTAR);
  fuzzy.jika(BASAH).dan(SEJUK).maka(TIDAK);
  fuzzy.jika(BASAH).dan(NORMAL).maka(TIDAK);
  fuzzy.jika(BASAH).dan(PANAS).maka(TIDAK);

  waktu = millis() - JEDA_CEK; // langsung cek saat menyala
}

void loop() {
  uint32_t sekarang = millis();

  if (menyiram) {
    if (sekarang - waktu >= lama) {
      digitalWrite(PIN_POMPA, LOW);
      menyiram = false;
      waktu = sekarang;
      Serial.println("Pompa mati");
    }
    return;
  }
  if (sekarang - waktu < JEDA_CEK) return;
  waktu = sekarang;

  float persen = (float)(ADC_KERING - analogRead(A0)) * 100 / (ADC_KERING - ADC_BASAH);
  float c = analogRead(A3) * 500.0 / 1023; // LM35 di board 5 V
  fuzzy.masukan(tanah, persen);              // di luar 0..100 otomatis dipotong
  fuzzy.masukan(suhu, c);
  fuzzy.hitung();
  float detik = fuzzy.keluaran(siram);

  Serial.print("Tanah ");
  Serial.print(persen, 0);
  Serial.print("%, suhu ");
  Serial.print(c, 1);
  Serial.print(" C -> siram ");
  Serial.print(detik, 1);
  Serial.println(" detik");

  if (detik >= 3) { // di bawah 3 detik tidak perlu menyiram
    lama = detik * 1000;
    menyiram = true;
    digitalWrite(PIN_POMPA, HIGH);
  }
}
