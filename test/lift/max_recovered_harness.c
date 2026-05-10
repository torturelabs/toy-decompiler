#include <stdint.h>
#include <stdio.h>

// After signature recovery, maxfn takes only the two operands.
extern int32_t maxfn(int32_t a, int32_t b);

int main(void) {
  struct {
    int32_t a, b, expected;
  } cases[] = {
      {40, 2, 40},      {2, 40, 40},        {7, 7, 7},
      {-3, 5, 5},       {-100, -50, -50},   {0, 0, 0},
      {INT32_MAX, 1, INT32_MAX},
  };
  int fails = 0;
  for (size_t i = 0; i < sizeof cases / sizeof *cases; ++i) {
    int32_t got = maxfn(cases[i].a, cases[i].b);
    printf("max(%d, %d) = %d\n", cases[i].a, cases[i].b, got);
    if (got != cases[i].expected)
      fails++;
  }
  return fails;
}
