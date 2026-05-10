// RUN: %arm-lift %S/add.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-emitc \
// RUN:           --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp | %FileCheck %s

// Headline pipeline: ARM32 ELF → MLIR `arm` → `arith` → `emitc` → C source.
// The casts are EmitC's standard rendering for the unsigned-by-default
// `arith.addi` lowering; functional equivalence is checked by the runtime
// test next door.
//
// CHECK-LABEL: int32_t add(
// CHECK:         uint32_t {{.*}} = (uint32_t) {{.*}};
// CHECK:         uint32_t {{.*}} = (uint32_t) {{.*}};
// CHECK:         uint32_t {{.*}} = {{.*}} + {{.*}};
// CHECK:         return
// CHECK:       }
