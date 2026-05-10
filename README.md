# toy-decompiler

A toy ARM32 (A32) decompiler built on **MLIR** + **LLVM** + ideas vendored
from **QEMU**. The full pipeline:

```
ARM32 ELF
   ↓  arm-lift                                 (QEMU decodetree-driven)
MLIR `arm` dialect, multi-block CFG with cf.br / cf.cond_br
   ↓  arm-opt --arm-to-arith
   ↓          --lift-cf-to-scf                 (stock MLIR — recovers structure!)
   ↓          --convert-scf-to-emitc
   ↓          --convert-arith-to-emitc
   ↓          --convert-func-to-emitc
   ↓  mlir-translate --mlir-to-cpp
C source ──► gcc ──► native binary that matches qemu-arm output

…or, for an LLVM-IR oracle path:

   ↓  arm-opt --arm-to-arith
   ↓          --convert-arith-to-llvm
   ↓          --convert-cf-to-llvm
   ↓          --convert-func-to-llvm
   ↓          --reconcile-unrealized-casts
   ↓  mlir-translate --mlir-to-llvmir
LLVM IR ──► clang ──► native binary
```

## Demo

A real ARM32 ELF compiled from this assembly:

```asm
@ pick(sel, a, b): if sel >= 0 return a; else return b.
pick:
    cmp r0, #0
    bge .La
    mov r0, r2
    b   .Ldone
.La:
    mov r0, r1
.Ldone:
    bx lr
```

Lifted to MLIR with full CFG, then structure-recovered + signature-recovered
+ EmitC-rendered:

```c
int32_t pick(int32_t v1, int32_t v2, int32_t v3) {
  int32_t v6 = 0;
  bool v7 = v1 < v6;            // sel < 0
  bool v8 = v7 == false;        // !(sel < 0)  ≡  sel >= 0
  int32_t v9;
  if (v8) {
    v9 = v2;                    // sel >= 0  →  return a
  } else {
    v9 = v3;                    // sel <  0  →  return b
  }
  return v9;
}
```

The function name `pick` is recovered from the ELF symbol table; the
3-argument signature is recovered by dropping the 17 dead register-file
slots that the lifter starts with.

Three test programs covering different ARM patterns are checked end-to-end:
| Source                         | What it exercises                        |
|--------------------------------|------------------------------------------|
| `int add(int, int)`            | Linear straight-line code, BX-as-return  |
| `int max(int, int)`            | Conditional execution (`MOVLT`), CondGuard, full NZCV derivation |
| `int pick(int, int, int)`      | Multi-block CFG with `BGE` + unconditional `B` |

Each is verified by **runtime equivalence**: the lifted-and-lowered code is
recompiled (clang for LLVM IR, gcc for the recovered C) and run against a
harness that checks output for ~6 cases including signed extremes.

## What's implemented

| Milestone | Status | Notes |
|-----------|--------|-------|
| M0 — project skeleton | ✓ | CMake, dialect stub, lit harness |
| M1 — vendor QEMU `decodetree.py` + `a32.decode` | ✓ | 243 trans_* dispatch entries auto-generated at build time |
| M2 — full ODS for ~30 ops | ✓ | add/sub/mul/and/orr/eor/bic + flag-setting variants, shifts, cmp/tst, load{,b}/store{,b}, movw/movt, cond_check, nop, unhandled |
| M3 — ELF reader + linear-block lifter | ✓ | Real semantics for MOV_rxi/MOV_rxri/ADD_rrri/BX-as-return |
| M3-ext — conditional execution + CMP | ✓ | `CondGuard` RAII helper for non-AL conds; full NZCV derivation; SUB added |
| M3-ext-2 — multi-block CFG + branches | ✓ | `BlockBuilder` for leader discovery, `trans_B` for `cf.br`/`cf.cond_br` |
| M4 — ArmToArith lowering pass | ✓ | Pure arithmetic / bitwise / shift / nop |
| M5 — `cf` → `scf` structure recovery | ✓ | Stock MLIR `--lift-cf-to-scf`; the diamond CFG of `pick` collapses to a single `scf.if` |
| M6 — LLVM IR oracle path | ✓ | Composed from stock MLIR passes |
| M7 — EmitC C-source path | ✓ | Recovered structure renders as readable `if`/`else` |
| M8 — README + LICENSE | ✓ | This file |
| M9 — signature + symbol-name recovery | ✓ | ELF symbol table → function name; `--arm-recover-function-signature` drops dead args (pick: 20 → 3) |

**Out of scope** (not done): BL-as-call (would need symbol recovery and
`func.call` plumbing), LDR/STR/MUL/AND/ORR/EOR/BIC real semantics (currently
fall through to `arm.unhandled` placeholders), Thumb/T16/T32, VFP, NEON,
exceptions, type recovery, variable-name recovery, irreducible CFG handling.

## Build & test

```bash
cmake -S . -B build -G Ninja
cmake --build build
cmake --build build --target check-toy-decompiler
```

Requires installed MLIR + LLVM ≥ 22.x.

The test ELFs are checked in pre-built (rebuilding them needs an ARM32
cross-compiler — the project was developed against
`/opt/armv7-eabihf--uclibc--stable-2020.08-1/bin/arm-buildroot-linux-uclibcgnueabihf-gcc`).

## Trying it on your own input

```bash
# 1. Cross-compile a tiny ARM function with -marm.
arm-linux-gnueabi-gcc -marm -c your.c -o your.o
arm-linux-gnueabi-ld  -Ttext=0x10000 your.o -o your.elf

# 2. Lift to MLIR.
./build/tools/arm-lift/arm-lift your.elf > stage1.mlir

# 3a. Recover structure + signature, emit C source.
./build/tools/arm-opt/arm-opt --arm-to-arith --lift-cf-to-scf \
    --arm-recover-function-signature \
    --convert-scf-to-emitc --convert-arith-to-emitc \
    --convert-func-to-emitc stage1.mlir |
  mlir-translate --mlir-to-cpp > out.c

# 3b. Or emit LLVM IR (no structure recovery, but optimisable).
./build/tools/arm-opt/arm-opt --arm-to-arith \
    --convert-arith-to-llvm --convert-cf-to-llvm --convert-func-to-llvm \
    --reconcile-unrealized-casts stage1.mlir |
  mlir-translate --mlir-to-llvmir > out.ll
```

If you hand `arm-lift` an instruction it doesn't yet implement, you'll get
`arm.unhandled "MNEMONIC" pc=PC insn=INSN` placeholders in the IR rather
than wrong code — the conservative-effects op survives every downstream
pass so it's easy to spot.

## Design notes

The non-obvious decisions are recorded as kaeru memories; the headlines:

- **Decoder = vendored QEMU.** `decodetree.py` + `a32.decode` produce a
  C dispatch table at build time. We provide our own `trans_*` callbacks
  that emit MLIR ops instead of TCG ops. No QEMU runtime dependency.
- **Register file = function/block arguments** (16 GPRs as `i32` + NZCV
  as `i1`). SSA-clean, no mutable state. Inter-block flow plumbs the
  20-element state through block args; structure recovery removes most
  of it as dead via standard dataflow.
- **Memory ops are custom** (`arm.load`/`arm.store`) rather than `memref`,
  to avoid the in-bounds invariant baked into `memref.load`'s lowering.
- **Conditional execution = CondGuard RAII.** Non-AL conds wrap their
  body in `cf.cond_br %cond, ^body, ^after(<old state>)` + body emits
  the op + `cf.br ^after(<new state>)`. Both branches converge in
  ^after via 20-arg block parameters.
- **Inter-block branches:** a small bit-pattern classifier (no full
  decoder) discovers leaders by walking once for direct branches; pass 2
  allocates one block per leader and the lifter switches builder
  insertion at every leader boundary.
- **Structure recovery is free** — MLIR's `--lift-cf-to-scf` does the
  heavy lifting. The toy doesn't need a custom restructuring algorithm.
- **Signature recovery = dead-arg elimination + ELF symbol table.** Run
  AFTER `--lift-cf-to-scf` (before that, every arg looks live because
  cf.br operand chains carry the full 20-element register state).

## Project size

```
$ find . -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.td' \
                  -o -name '*.py' -o -name 'CMakeLists.txt' \
                  -o -name '*.mlir' -o -name '*.def' \) \
       ! -path './build/*' ! -path './third_party/qemu/decodetree.py' \
       ! -path './third_party/qemu/a32*.decode' | xargs wc -l
```

About 2000 LOC of project code (plus ~28K LOC vendored from QEMU).
**17/17 lit tests pass**, including 6 end-to-end runtime equivalence tests
across 3 source programs and 2 output paths.

## License

GPLv2 (forced by vendoring two files from QEMU). See `LICENSE` and
`third_party/qemu/README.md`.

The original design plan and milestone reasoning lives at
`~/.claude/plans/pickup-tmp-mlir-md-as-reference-radiant-lerdorf.md`.
