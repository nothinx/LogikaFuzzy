// Sugeno orde-0: jarak halangan -> kecepatan robot.
// Keluaran Sugeno berupa konstanta, hasilnya rata-rata berbobot:
//   kecepatan = jumlah(kekuatan x konstanta) / jumlah(kekuatan)
// Lebih ringan daripada Mamdani karena tidak ada centroid.
//
// Tanpa sensor: jarak disapu 0..100 cm dan hasilnya dicetak di Serial Monitor (115200).
// Untuk robot sungguhan, isi jarak dari sensor ultrasonik lalu kirim hasilnya ke PWM motor.
#include <LogikaFuzzy.h>

LogikaFuzzy<1, 1, 3, 3> fuzzy;
uint8_t jarak, kecepatan;

void setup() {
  Serial.begin(115200);

  jarak = fuzzy.tambahMasukan(0, 100);       // cm
  kecepatan = fuzzy.tambahKeluaranSugeno();  // PWM

  uint8_t DEKAT = fuzzy.tambahHimpunan(jarak, trapesium(0, 0, 15, 35));
  uint8_t SEDANG = fuzzy.tambahHimpunan(jarak, segitiga(20, 45, 70));
  uint8_t JAUH = fuzzy.tambahHimpunan(jarak, trapesium(55, 80, 100, 100));

  uint8_t BERHENTI = fuzzy.tambahHimpunan(kecepatan, konstanta(0));
  uint8_t PELAN = fuzzy.tambahHimpunan(kecepatan, konstanta(120));
  uint8_t CEPAT = fuzzy.tambahHimpunan(kecepatan, konstanta(255));

  fuzzy.jika(DEKAT).maka(BERHENTI);
  fuzzy.jika(SEDANG).maka(PELAN);
  fuzzy.jika(JAUH).maka(CEPAT);
}

void loop() {
  static uint32_t terakhir = 0;
  static uint8_t cm = 0;
  if (millis() - terakhir < 200) return;
  terakhir = millis();

  fuzzy.masukan(jarak, cm);
  fuzzy.hitung();
  Serial.print("Jarak ");
  Serial.print(cm);
  Serial.print(" cm -> kecepatan ");
  Serial.println(fuzzy.keluaran(kecepatan), 1);

  cm = cm >= 100 ? 0 : cm + 5;
}
