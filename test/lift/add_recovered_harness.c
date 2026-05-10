#include <stdint.h>
#include <stdio.h>

extern int32_t add(int32_t a, int32_t b);

int main(void) {
  int32_t r = add(40, 2);
  printf("add(40, 2) = %d\n", r);
  return r != 42;
}
