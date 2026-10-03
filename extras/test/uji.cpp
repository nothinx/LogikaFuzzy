// Uji logika LogikaFuzzy di PC:
//   g++ -std=c++11 -Wall -Wextra -I. -I../../src uji.cpp ../../src/*.cpp -o uji && ./uji
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "LogikaFuzzy.h"

static bool dekat(float a, float b) { return fabsf(a - b) < 1e-4f; }

int main() {
  { // derajat keanggotaan di titik kunci
    Himpunan s = segitiga(0, 5, 10);
    assert(s.derajat(-1) == 0 && s.derajat(0) == 0 && s.derajat(2.5f) == 0.5f);
    assert(s.derajat(5) == 1 && s.derajat(7.5f) == 0.5f && s.derajat(10) == 0 && s.derajat(11) == 0);
    Himpunan kiri = trapesium(0, 0, 10, 20); // bahu kiri
    assert(kiri.derajat(0) == 1 && kiri.derajat(10) == 1 && kiri.derajat(15) == 0.5f && kiri.derajat(20) == 0);
    Himpunan kanan = trapesium(10, 20, 30, 30); // bahu kanan
    assert(kanan.derajat(10) == 0 && kanan.derajat(12.5f) == 0.25f && kanan.derajat(30) == 1 && kanan.derajat(31) == 0);
    Himpunan k = konstanta(5);
    assert(k.derajat(5) == 1 && k.derajat(4.9f) == 0);
  }

  // Sistem uji: suhu 0..40, kipas 0..100.
  //   DINGIN = trapesium(0, 0, 10, 30)    PANAS = trapesium(10, 30, 40, 40)
  //   PELAN  = trapesium(0, 0, 20, 60)    CEPAT = trapesium(40, 80, 100, 100)
  { // dua aturan, dihitung tangan dengan resolusi 11 titik (0, 10, ..., 100)
    LogikaFuzzy<1, 1, 2, 2> f;
    uint8_t suhu = f.tambahMasukan(0, 40), kipas = f.tambahKeluaran(0, 100);
    uint8_t DINGIN = f.tambahHimpunan(suhu, trapesium(0, 0, 10, 30));
    uint8_t PANAS = f.tambahHimpunan(suhu, trapesium(10, 30, 40, 40));
    uint8_t PELAN = f.tambahHimpunan(kipas, trapesium(0, 0, 20, 60));
    uint8_t CEPAT = f.tambahHimpunan(kipas, trapesium(40, 80, 100, 100));
    assert(f.jika(DINGIN).maka(PELAN));
    assert(f.jika(PANAS).maka(CEPAT));
    f.aturResolusi(11);
    f.masukan(suhu, 18);
    assert(f.hitung());
    // Fuzzifikasi suhu = 18:
    //   DINGIN = (30 - 18) / (30 - 10) = 0,6     PANAS = (18 - 10) / (30 - 10) = 0,4
    assert(dekat(f.derajat(DINGIN), 0.6f) && dekat(f.derajat(PANAS), 0.4f));
    assert(dekat(f.kekuatan(0), 0.6f) && dekat(f.kekuatan(1), 0.4f) && f.kekuatan(2) == 0);
    assert(dekat(f.derajat(PELAN), 0.6f) && dekat(f.derajat(CEPAT), 0.4f));
    // PELAN dipotong 0,6 dan CEPAT dipotong 0,4, lalu diambil yang terbesar:
    //   x    :  0    10   20   30   40   50    60   70   80   90   100
    //   PELAN:  0,6  0,6  0,6  0,6  0,5  0,25  0    0    0    0    0
    //   CEPAT:  0    0    0    0    0    0,25  0,4  0,4  0,4  0,4  0,4
    //   mu   :  0,6  0,6  0,6  0,6  0,5  0,25  0,4  0,4  0,4  0,4  0,4
    // jumlah(mu)     = 4 x 0,6 + 0,5 + 0,25 + 5 x 0,4 = 5,15
    // jumlah(x * mu) = 6 + 12 + 18 + 20 + 12,5 + 24 + 28 + 32 + 36 + 40 = 228,5
    // centroid       = 228,5 / 5,15 = 44,3689
    assert(dekat(f.keluaran(kipas), 228.5f / 5.15f));

    // masukan di luar semesta dipotong ke batasnya
    f.masukan(suhu, -10);
    assert(f.derajat(DINGIN) == 1 && f.derajat(PANAS) == 0);
    f.masukan(suhu, 100);
    assert(f.derajat(DINGIN) == 0 && f.derajat(PANAS) == 1);
    assert(f.hitung() && f.keluaran(kipas) > 80);
  }
  { // satu aturan: himpunan keluaran simetris, centroid tepat di puncak
    LogikaFuzzy<1, 1, 1, 1> f;
    uint8_t suhu = f.tambahMasukan(0, 40), kipas = f.tambahKeluaran(0, 100);
    uint8_t HANGAT = f.tambahHimpunan(suhu, segitiga(10, 20, 30));
    uint8_t SEDANG = f.tambahHimpunan(kipas, segitiga(20, 50, 80));
    assert(f.jika(HANGAT).maka(SEDANG));
    f.masukan(suhu, 13); // derajat 0,3
    assert(f.hitung() && dekat(f.kekuatan(0), 0.3f) && dekat(f.keluaran(kipas), 50));
    // tidak ada aturan aktif: hitung() false, hasil titik tengah semesta
    f.masukan(suhu, 35);
    assert(!f.hitung() && f.keluaran(kipas) == 50);
  }
  { // DAN = min, ATAU = max
    LogikaFuzzy<2, 1, 1, 2> f;
    uint8_t suhu = f.tambahMasukan(0, 40), lembap = f.tambahMasukan(0, 100), kipas = f.tambahKeluaran(0, 100);
    uint8_t DINGIN = f.tambahHimpunan(suhu, trapesium(0, 0, 10, 30));
    uint8_t KERING = f.tambahHimpunan(lembap, trapesium(0, 0, 30, 70));
    uint8_t PELAN = f.tambahHimpunan(kipas, trapesium(0, 0, 20, 60));
    assert(f.jika(DINGIN).dan(KERING).maka(PELAN));
    assert(f.jika(DINGIN).atau(KERING).maka(PELAN));
    f.masukan(suhu, 18);   // DINGIN = 0,6
    f.masukan(lembap, 40); // KERING = (70 - 40) / (70 - 30) = 0,75
    assert(dekat(f.kekuatan(0), 0.6f) && dekat(f.kekuatan(1), 0.75f));
    assert(dekat(f.derajat(PELAN), 0.75f)); // max dari kedua aturan
  }
  { // Sugeno orde-0: (0,6 x 20 + 0,4 x 80) / (0,6 + 0,4) = 44
    LogikaFuzzy<1, 1, 2, 2> f;
    uint8_t suhu = f.tambahMasukan(0, 40), kipas = f.tambahKeluaranSugeno();
    uint8_t DINGIN = f.tambahHimpunan(suhu, trapesium(0, 0, 10, 30));
    uint8_t PANAS = f.tambahHimpunan(suhu, trapesium(10, 30, 40, 40));
    uint8_t PELAN = f.tambahHimpunan(kipas, konstanta(20));
    uint8_t CEPAT = f.tambahHimpunan(kipas, konstanta(80));
    f.jika(DINGIN).maka(PELAN);
    f.jika(PANAS).maka(CEPAT);
    f.masukan(suhu, 18);
    assert(f.hitung() && dekat(f.keluaran(kipas), 44));
    f.masukan(suhu, 40); // hanya PANAS
    assert(f.hitung() && dekat(f.keluaran(kipas), 80));
  }
  { // kapasitas penuh dan aturan tidak sah
    LogikaFuzzy<1, 1, 2, 1> f;
    uint8_t suhu = f.tambahMasukan(0, 40), kipas = f.tambahKeluaran(0, 100);
    assert(f.tambahMasukan(0, 1) == TIDAK_ADA && f.tambahKeluaran(0, 1) == TIDAK_ADA);
    assert(f.tambahHimpunan(7, segitiga(0, 1, 2)) == TIDAK_ADA);
    uint8_t A = f.tambahHimpunan(suhu, segitiga(0, 10, 20));
    uint8_t B = f.tambahHimpunan(suhu, segitiga(10, 20, 30));
    assert(f.tambahHimpunan(suhu, segitiga(20, 30, 40)) == TIDAK_ADA);
    uint8_t P = f.tambahHimpunan(kipas, segitiga(0, 50, 100));
    assert(!f.jika(P).maka(P));        // keluaran di bagian JIKA
    assert(!f.jika(A).maka(B));        // masukan di bagian MAKA
    assert(!f.jika(A).dan(B).maka(P)); // masukan yang sama dua kali
    assert(!f.jika(TIDAK_ADA).maka(P));
    assert(!f.jika(A).maka(TIDAK_ADA));
    assert(!f.masukan(kipas, 1) && f.jumlahAturan() == 0);
    assert(f.jika(A).maka(P));
    assert(!f.jika(B).maka(P) && f.jumlahAturan() == 1); // aturan penuh
    assert(f.kekuatan(5) == 0 && f.derajat(TIDAK_ADA) == 0 && f.keluaran(suhu) == 0);
  }
  { // DAN dan ATAU tidak boleh dicampur dalam satu aturan
    LogikaFuzzy<3, 1, 1, 2> f;
    uint8_t a = f.tambahMasukan(0, 1), b = f.tambahMasukan(0, 1), c = f.tambahMasukan(0, 1);
    uint8_t k = f.tambahKeluaran(0, 1);
    uint8_t A = f.tambahHimpunan(a, segitiga(0, 0, 1)), B = f.tambahHimpunan(b, segitiga(0, 0, 1));
    uint8_t C = f.tambahHimpunan(c, segitiga(0, 0, 1)), K = f.tambahHimpunan(k, segitiga(0, 0, 1));
    assert(!f.jika(A).dan(B).atau(C).maka(K));
    assert(f.jika(A).dan(B).dan(C).maka(K));
    assert(f.jika(A).atau(B).atau(C).maka(K));
  }
  { // sistem 2 masukan x 3 himpunan, 9 aturan: makin panas, kipas tidak melambat
    LogikaFuzzy<2, 1, 3, 9> f;
    uint8_t suhu = f.tambahMasukan(0, 40), lembap = f.tambahMasukan(0, 100), kipas = f.tambahKeluaran(0, 255);
    uint8_t S[3] = {f.tambahHimpunan(suhu, trapesium(0, 0, 15, 25)), f.tambahHimpunan(suhu, segitiga(15, 25, 35)),
                    f.tambahHimpunan(suhu, trapesium(25, 35, 40, 40))};
    uint8_t L[3] = {f.tambahHimpunan(lembap, trapesium(0, 0, 30, 50)), f.tambahHimpunan(lembap, segitiga(30, 50, 70)),
                    f.tambahHimpunan(lembap, trapesium(50, 70, 100, 100))};
    uint8_t K[3] = {f.tambahHimpunan(kipas, trapesium(0, 0, 50, 120)), f.tambahHimpunan(kipas, segitiga(50, 128, 200)),
                    f.tambahHimpunan(kipas, trapesium(130, 200, 255, 255))};
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++) assert(f.jika(S[i]).dan(L[j]).maka(K[(i + j + 1) / 2]));
    assert(f.jumlahAturan() == 9);
    for (int t = 0; t <= 40; t += 5) {
      for (int l = 0; l <= 100; l += 10) {
        f.masukan(suhu, t);
        f.masukan(lembap, l);
        assert(f.hitung() && f.keluaran(kipas) > 0 && f.keluaran(kipas) < 255);
      }
    }
    f.masukan(suhu, 0); // hanya DINGIN & KERING -> PELAN
    f.masukan(lembap, 0);
    f.hitung();
    assert(dekat(f.kekuatan(0), 1) && f.kekuatan(1) == 0 && f.keluaran(kipas) < 60);
    f.masukan(suhu, 40); // hanya PANAS & BASAH -> CEPAT
    f.masukan(lembap, 100);
    f.hitung();
    assert(dekat(f.kekuatan(8), 1) && f.keluaran(kipas) > 190);
  }
  printf("Semua uji lolos\n");
  return 0;
}
