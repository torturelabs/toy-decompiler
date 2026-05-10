#include <stdint.h>
#include <stdio.h>

// After signature recovery, pick takes its three real arguments and
// nothing else.
extern int32_t pick(int32_t sel, int32_t a, int32_t b);

int main(void) {
  struct {
    int32_t sel, a, b, expected;
  } cases[] = {
      {1, 100, 200, 100},
      {0, 100, 200, 100},
      {-1, 100, 200, 200},
      {-99, 7, 13, 13},
      {INT32_MIN, 1, 2, 2},
      {INT32_MAX, 1, 2, 1},
  };
  int fails = 0;
  for (size_t i = 0; i < sizeof cases / sizeof *cases; ++i) {
    int32_t got = pick(cases[i].sel, cases[i].a, cases[i].b);
    printf("pick(%d, %d, %d) = %d\n", cases[i].sel, cases[i].a, cases[i].b,
           got);
    if (got != cases[i].expected)
      fails++;
  }
  return fails;
}
