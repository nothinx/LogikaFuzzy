// Kipas otomatis: suhu -> kecepatan kipas (PWM), metode Mamdani.
//
// Sambungan: sensor LM35 ke A0, driver motor/MOSFET kipas ke pin 9.
// Rumus suhu di bawah untuk LM35 di board 5 V dengan ADC 10 bit.
// Ganti dengan sensor lain (DHT22, DS18B20) bila perlu, cukup isi variabel c.
#include <LogikaFuzzy.h>

// 1 masukan, 1 keluaran, maksimal 3 himpunan per variabel, 3 aturan.
LogikaFuzzy<1, 1, 3, 3> fuzzy;
uint8_t suhu, kipas;

const uint8_t PIN_KIPAS = 9;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_KIPAS, OUTPUT);

  suhu = fuzzy.tambahMasukan(0, 50);      // semesta 0..50 °C
  kipas = fuzzy.tambahKeluaran(0, 255);   // PWM 0..255

  uint8_t DINGIN = fuzzy.tambahHimpunan(suhu, trapesium(0, 0, 20, 27));
  uint8_t HANGAT = fuzzy.tambahHimpunan(suhu, segitiga(22, 28, 34));
  uint8_t PANAS = fuzzy.tambahHimpunan(suhu, trapesium(30, 37, 50, 50));

  uint8_t MATI = fuzzy.tambahHimpunan(kipas, trapesium(0, 0, 30, 90));
  uint8_t SEDANG = fuzzy.tambahHimpunan(kipas, segitiga(60, 140, 220));
  uint8_t KENCANG = fuzzy.tambahHimpunan(kipas, trapesium(180, 230, 255, 255));

  fuzzy.jika(DINGIN).maka(MATI);
  fuzzy.jika(HANGAT).maka(SEDANG);
  fuzzy.jika(PANAS).maka(KENCANG);
}

void loop() {
  static uint32_t terakhir = 0;
  if (millis() - terakhir < 500) return;
  terakhir = millis();

  float c = analogRead(A0) * 500.0 / 1023; // LM35: 10 mV per °C
  fuzzy.masukan(suhu, c);
  fuzzy.hitung();
  int pwm = fuzzy.keluaran(kipas);
  analogWrite(PIN_KIPAS, pwm);

  Serial.print("Suhu ");
  Serial.print(c, 1);
  Serial.print(" C -> PWM ");
  Serial.println(pwm);
}
