# LogikaFuzzy (English)

[Bahasa Indonesia](README.md)

An Arduino **fuzzy logic** library: Mamdani and zero-order Sugeno, rules that read like a thesis report, and every stage (fuzzification, rule strength, result) readable for checking against hand calculations or MATLAB. No dynamic allocation. The API and examples are in Indonesian. This page maps every function to English.

```cpp
#include <LogikaFuzzy.h>

// 1 input, 1 output, up to 2 sets per variable, 2 rules.
LogikaFuzzy<1, 1, 2, 2> fuzzy;
uint8_t temp, fan;

void setup() {
  Serial.begin(115200);
  temp = fuzzy.tambahMasukan(0, 40);     // addInput(min, max)
  fan = fuzzy.tambahKeluaran(0, 100);    // addOutput(min, max), Mamdani

  uint8_t COLD = fuzzy.tambahHimpunan(temp, trapesium(0, 0, 10, 30));
  uint8_t HOT = fuzzy.tambahHimpunan(temp, trapesium(10, 30, 40, 40));
  uint8_t SLOW = fuzzy.tambahHimpunan(fan, trapesium(0, 0, 20, 60));
  uint8_t FAST = fuzzy.tambahHimpunan(fan, trapesium(40, 80, 100, 100));

  fuzzy.jika(COLD).maka(SLOW);           // IF cold THEN slow
  fuzzy.jika(HOT).maka(FAST);
}

void loop() {
  fuzzy.masukan(temp, 18);               // setInput
  fuzzy.hitung();                        // compute
  Serial.println(fuzzy.keluaran(fan));   // output: 44.86
  delay(1000);
}
```

## Why

- Rules are one readable line: `jika(A).dan(B).maka(C)` = IF A AND B THEN C.
- No `new` or `malloc`. Capacity is a template parameter, so RAM use is known at compile time. A 2-input × 3-set, 1-output, 9-rule system uses **227 bytes** on an Arduino Uno.
- Mamdani matches MATLAB's defaults: AND = min, OR = max, min implication, max aggregation, discrete centroid with 101 points (adjustable), inputs clamped to their range.
- Zero-order Sugeno (weighted average of constants).
- Membership degrees and rule strengths are readable for thesis reports.

Compared from source code: eFLL 1.5.1 calls `malloc` for every variable, set, and rule, and `malloc`/`free` for every composition point on each `fuzzify()`; it has no Sugeno and exposes rules only as fired/not fired. qlibs (fis) is a larger, more complete engine (Mamdani, Sugeno, Tsukamoto) within a general-purpose collection.

## Function reference

| Indonesian | English | Notes |
|---|---|---|
| `LogikaFuzzy<M, K, H, A>` | class | max inputs, outputs, sets per variable, rules |
| `segitiga(a, b, c)` | triangle | |
| `trapesium(a, b, c, d)` | trapezoid | shoulders: `a == b` or `c == d` |
| `konstanta(k)` | constant | Sugeno output |
| `h.derajat(x)` | membership degree | |
| `tambahMasukan(min, max)` | add input | returns `TIDAK_ADA` (none) when full |
| `tambahKeluaran(min, max)` | add Mamdani output | |
| `tambahKeluaranSugeno()` | add Sugeno output | |
| `tambahHimpunan(var, set)` | add set | returns a set id for rules |
| `jika(h).dan(h).maka(h)` | if … and … then | AND = min |
| `jika(h).atau(h).maka(h)` | if … or … then | OR = max; mixing AND/OR in one rule is rejected |
| `masukan(var, value)` | set input | clamped to range |
| `hitung()` | compute | `false` if an output has no active rule (result = range midpoint) |
| `keluaran(var)` | output | |
| `derajat(set)` | degree | input set: membership; output set: strongest rule into it |
| `kekuatan(i)` | rule strength | 0 = first rule |
| `jumlahAturan()` | rule count | |
| `aturResolusi(n)` | set resolution | centroid points, default 101 |

## Examples

`KipasOtomatis` (temperature → fan PWM), `PenyiramTanaman` (soil moisture + temperature → watering time, 9 rules), `CocokkanHitunganManual` (prints every stage for a report), `SugenoSederhana` (distance → robot speed, Sugeno).

## Status

Version 1.0.0 passes automated logic tests on a PC and compiles on Uno, Mega, ESP32, ESP32-C3, ESP32-S3, STM32 Blackpill F411, and Bluepill F103. It is pure software and does not depend on specific hardware.

## License

MIT © 2026 Amadeo Wisesa.
