// RUN: %arm-opt %s | %FileCheck %s

// CHECK-LABEL: module
module {
  // CHECK: arm.nop
  arm.nop
}
