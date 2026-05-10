// RUN: %arm-opt --arm-to-arith %s | %FileCheck %s

// Each lowered op is exercised in isolation so the greedy driver can't
// fold pairs like (a+b)-b into nothing.

// CHECK-LABEL: func.func @check_add
// CHECK-NOT:     arm.add
// CHECK:         arith.addi
func.func @check_add(%a: i32, %b: i32) -> i32 {
  %0 = arm.add %a, %b : i32
  return %0 : i32
}

// CHECK-LABEL: func.func @check_sub
// CHECK-NOT:     arm.sub
// CHECK:         arith.subi
func.func @check_sub(%a: i32, %b: i32) -> i32 {
  %0 = arm.sub %a, %b : i32
  return %0 : i32
}

// CHECK-LABEL: func.func @check_mul
// CHECK-NOT:     arm.mul
// CHECK:         arith.muli
func.func @check_mul(%a: i32, %b: i32) -> i32 {
  %0 = arm.mul %a, %b : i32
  return %0 : i32
}

// CHECK-LABEL: func.func @check_bitwise
// CHECK-NOT:     arm.and
// CHECK-NOT:     arm.orr
// CHECK-NOT:     arm.eor
// CHECK-DAG:     arith.andi
// CHECK-DAG:     arith.ori
// CHECK-DAG:     arith.xori
func.func @check_bitwise(%a: i32, %b: i32, %c: i32) -> (i32, i32, i32) {
  %0 = arm.and %a, %b : i32
  %1 = arm.orr %a, %c : i32
  %2 = arm.eor %b, %c : i32
  return %0, %1, %2 : i32, i32, i32
}

// BIC = a AND NOT b. We materialize NOT-b via XOR with all-ones.
// CHECK-LABEL: func.func @check_bic
// CHECK-NOT:     arm.bic
// CHECK-DAG:     %[[ALL_ONES:.*]] = arith.constant -1
// CHECK-DAG:     %[[NOT_B:.*]] = arith.xori %arg1, %[[ALL_ONES]]
// CHECK:         arith.andi %arg0, %[[NOT_B]]
func.func @check_bic(%a: i32, %b: i32) -> i32 {
  %0 = arm.bic %a, %b : i32
  return %0 : i32
}

// CHECK-LABEL: func.func @check_shifts
// CHECK-NOT:     arm.lsl
// CHECK-NOT:     arm.lsr
// CHECK-NOT:     arm.asr
// CHECK-DAG:     arith.shli
// CHECK-DAG:     arith.shrui
// CHECK-DAG:     arith.shrsi
func.func @check_shifts(%v: i32, %s: i32) -> (i32, i32, i32) {
  %0 = arm.lsl %v, %s : i32
  %1 = arm.lsr %v, %s : i32
  %2 = arm.asr %v, %s : i32
  return %0, %1, %2 : i32, i32, i32
}

// CHECK-LABEL: func.func @check_nop_erased
// CHECK-NEXT:    return
// CHECK-NEXT:  }
func.func @check_nop_erased() {
  arm.nop
  arm.nop
  arm.nop
  return
}

// Flag-setting ops are M4-out-of-scope: they should pass through untouched.
// CHECK-LABEL: func.func @check_flag_setting_passthrough
// CHECK:         arm.adds
func.func @check_flag_setting_passthrough(%a: i32, %b: i32) -> i32 {
  %r, %n, %z, %c, %v = arm.adds %a, %b : i32
  return %r : i32
}

// Memory ops (load/store) are M4-out-of-scope: they should pass through.
// CHECK-LABEL: func.func @check_memory_passthrough
// CHECK:         arm.load
// CHECK:         arm.store
func.func @check_memory_passthrough(%addr: i32) -> i32 {
  %v = arm.load %addr : i32 -> i32
  arm.store %v, %addr : i32, i32
  return %v : i32
}
