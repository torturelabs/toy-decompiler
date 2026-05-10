// RUN: %arm-lift %S/add.elf | %FileCheck %s

// Source (test/lift/add.s):
//     add:
//         add r0, r0, r1
//         bx lr

// CHECK-LABEL: func.func @add(
// CHECK-SAME:    %arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32,
// CHECK-SAME:    %arg4: i32, %arg5: i32, %arg6: i32, %arg7: i32,
// CHECK-SAME:    %arg8: i32, %arg9: i32, %arg10: i32, %arg11: i32,
// CHECK-SAME:    %arg12: i32, %arg13: i32, %arg14: i32, %arg15: i32,
// CHECK-SAME:    %arg16: i1, %arg17: i1, %arg18: i1, %arg19: i1) -> i32 {
// CHECK:         %[[R:.*]] = arm.add %arg0, %arg1 : i32
// CHECK:         return %[[R]] : i32
// CHECK:       }
