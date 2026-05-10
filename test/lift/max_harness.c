#include <stdint.h>
#include <stdio.h>

extern int32_t maxfn(int32_t r0, int32_t r1, int32_t r2, int32_t r3,
                      int32_t r4, int32_t r5, int32_t r6, int32_t r7,
                      int32_t r8, int32_t r9, int32_t r10, int32_t r11,
                      int32_t r12, int32_t r13, int32_t r14, int32_t r15,
                      _Bool n, _Bool z, _Bool c, _Bool v);

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
    int32_t got = maxfn(cases[i].a, cases[i].b,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0);
    printf("max(%d, %d) = %d\n", cases[i].a, cases[i].b, got);
    if (got != cases[i].expected) fails++;
  }
  return fails;
}
