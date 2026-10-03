#include "LogikaFuzzy.h"

// Rumus sama dengan trapmf/trimf MATLAB. Sisi tegak (a == b atau c == d)
// bernilai 1 tepat di titiknya, jadi trapesium(0, 0, 10, 20) bernilai 1 di x = 0.
float Himpunan::derajat(float x) const {
  if (x < a || x > d) return 0;
  if (x < b) return (x - a) / (b - a);
  if (x <= c) return 1;
  return (d - x) / (d - c);
}
