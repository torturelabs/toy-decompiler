// RUN: %arm-lift --decode-only 2>&1 | %FileCheck %s

// `--decode-only` runs the QEMU decode tree on hand-encoded A32 words and
// logs whichever pattern matches. Hand-implemented mnemonics short-circuit
// silently — they're covered by the lift e2e tests. The samples here are
// deliberately drawn from instruction families we still ship as auto-stubs.
//
// CHECK: pc=0x1000 insn=0xe1800001 ORR_rrri
// CHECK: pc=0x1004 insn=0xe0010291 MUL
// CHECK: pc=0x1008 insn=0xe5901000 LDR_ri
// CHECK: pc=0x100c insn=0xe5801000 STR_ri
