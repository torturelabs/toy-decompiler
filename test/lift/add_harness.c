#include <stdint.h>
#include <stdio.h>

// The lifter exposes its function as `lifted(r0..r15, n, z, c, v)`.
extern int32_t add(int32_t r0, int32_t r1, int32_t r2, int32_t r3,
                      int32_t r4, int32_t r5, int32_t r6, int32_t r7,
                      int32_t r8, int32_t r9, int32_t r10, int32_t r11,
                      int32_t r12, int32_t r13, int32_t r14, int32_t r15,
                      _Bool n, _Bool z, _Bool c, _Bool v);

int main(void) {
  int32_t r = add(40, 2,
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0);
  printf("lifted(40, 2) = %d\n", r);
  return r != 42;
}
