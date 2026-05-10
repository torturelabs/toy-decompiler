#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern int32_t pick(int32_t r0, int32_t r1, int32_t r2, int32_t r3,
                      int32_t r4, int32_t r5, int32_t r6, int32_t r7,
                      int32_t r8, int32_t r9, int32_t r10, int32_t r11,
                      int32_t r12, int32_t r13, int32_t r14, int32_t r15,
                      bool n, bool z, bool c, bool v);

int main(void) {
  struct {
    int32_t sel, a, b, expected;
  } cases[] = {
      {1, 100, 200, 100},
      {0, 100, 200, 100},  /* BGE includes equal */
      {-1, 100, 200, 200},
      {-99, 7, 13, 13},
      {INT32_MIN, 1, 2, 2},
      {INT32_MAX, 1, 2, 1},
  };
  int fails = 0;
  for (size_t i = 0; i < sizeof cases / sizeof *cases; ++i) {
    int32_t got = pick(cases[i].sel, cases[i].a, cases[i].b,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0);
    printf("pick(%d, %d, %d) = %d\n", cases[i].sel, cases[i].a, cases[i].b,
           got);
    if (got != cases[i].expected)
      fails++;
  }
  return fails;
}
