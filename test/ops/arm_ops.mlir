// RUN: %arm-opt %s | %arm-opt | %FileCheck %s

// Round-trip every op family. We pipe through `arm-opt` twice to make sure the
// custom assembly format is both parseable and printable.

// CHECK-LABEL: func.func @arith_no_flags
func.func @arith_no_flags(%a: i32, %b: i32) -> i32 {
  // CHECK: %[[ADD:.*]] = arm.add %arg0, %arg1 : i32
  // CHECK: %[[SUB:.*]] = arm.sub %[[ADD]], %arg1 : i32
  // CHECK: %[[MUL:.*]] = arm.mul %[[SUB]], %arg0 : i32
  // CHECK: %[[AND:.*]] = arm.and %[[MUL]], %arg0 : i32
  // CHECK: %[[ORR:.*]] = arm.orr %[[AND]], %arg1 : i32
  // CHECK: %[[EOR:.*]] = arm.eor %[[ORR]], %arg0 : i32
  // CHECK: %[[BIC:.*]] = arm.bic %[[EOR]], %arg1 : i32
  %0 = arm.add %a, %b : i32
  %1 = arm.sub %0, %b : i32
  %2 = arm.mul %1, %a : i32
  %3 = arm.and %2, %a : i32
  %4 = arm.orr %3, %b : i32
  %5 = arm.eor %4, %a : i32
  %6 = arm.bic %5, %b : i32
  return %6 : i32
}

// CHECK-LABEL: func.func @arith_with_flags
func.func @arith_with_flags(%a: i32, %b: i32) -> (i32, i1, i1, i1, i1) {
  // CHECK: arm.adds %arg0, %arg1 : i32
  %r, %n, %z, %c, %v = arm.adds %a, %b : i32
  return %r, %n, %z, %c, %v : i32, i1, i1, i1, i1
}

// CHECK-LABEL: func.func @shifts
func.func @shifts(%v: i32, %s: i32) -> i32 {
  // CHECK: arm.lsl
  // CHECK: arm.lsr
  // CHECK: arm.asr
  // CHECK: arm.ror
  %0 = arm.lsl %v, %s : i32
  %1 = arm.lsr %0, %s : i32
  %2 = arm.asr %1, %s : i32
  %3 = arm.ror %2, %s : i32
  return %3 : i32
}

// CHECK-LABEL: func.func @compares
func.func @compares(%a: i32, %b: i32) -> (i1, i1, i1, i1) {
  // CHECK: arm.cmp %arg0, %arg1 : i32
  %n, %z, %c, %v = arm.cmp %a, %b : i32
  return %n, %z, %c, %v : i1, i1, i1, i1
}

// CHECK-LABEL: func.func @memory
func.func @memory(%addr: i32, %val: i32) -> i32 {
  // CHECK: %[[V:.*]] = arm.load %arg0 : i32 -> i32
  %0 = arm.load %addr : i32 -> i32
  // CHECK: %[[B:.*]] = arm.loadb %arg0 : i32 -> i32
  %1 = arm.loadb %addr : i32 -> i32
  // CHECK: arm.store %[[V]], %arg0 : i32, i32
  arm.store %0, %addr : i32, i32
  // CHECK: arm.storeb %[[B]], %arg0 : i32, i32
  arm.storeb %1, %addr : i32, i32
  return %0 : i32
}

// CHECK-LABEL: func.func @move_immediates
func.func @move_immediates() -> i32 {
  // CHECK: %[[LO:.*]] = arm.movw 65535 : i32
  %lo = arm.movw 65535 : i32
  // CHECK: %[[HI:.*]] = arm.movt %[[LO]], 4660 : i32
  %hi = arm.movt %lo, 4660 : i32
  return %hi : i32
}

// CHECK-LABEL: func.func @cond_predicate
func.func @cond_predicate(%n: i1, %z: i1, %c: i1, %v: i1) -> i1 {
  // CHECK: arm.cond_check eq, %arg0, %arg1, %arg2, %arg3
  %0 = arm.cond_check eq, %n, %z, %c, %v
  return %0 : i1
}

// CHECK-LABEL: func.func @nop
func.func @nop() {
  // CHECK: arm.nop
  arm.nop
  return
}
