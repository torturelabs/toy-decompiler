#ifndef ARM_OPS_TD
#define ARM_OPS_TD

include "Arm/ArmDialect.td"
include "mlir/IR/EnumAttr.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

//===----------------------------------------------------------------------===//
// Condition codes
//===----------------------------------------------------------------------===//

def Arm_CondAttr : I32EnumAttr<"Cond", "ARM condition predicate", [
    I32EnumAttrCase<"EQ",  0, "eq">,
    I32EnumAttrCase<"NE",  1, "ne">,
    I32EnumAttrCase<"CS",  2, "cs">,
    I32EnumAttrCase<"CC",  3, "cc">,
    I32EnumAttrCase<"MI",  4, "mi">,
    I32EnumAttrCase<"PL",  5, "pl">,
    I32EnumAttrCase<"VS",  6, "vs">,
    I32EnumAttrCase<"VC",  7, "vc">,
    I32EnumAttrCase<"HI",  8, "hi">,
    I32EnumAttrCase<"LS",  9, "ls">,
    I32EnumAttrCase<"GE", 10, "ge">,
    I32EnumAttrCase<"LT", 11, "lt">,
    I32EnumAttrCase<"GT", 12, "gt">,
    I32EnumAttrCase<"LE", 13, "le">,
    I32EnumAttrCase<"AL", 14, "al">,
    I32EnumAttrCase<"NV", 15, "nv">
]> {
  let cppNamespace = "::mlir::arm";
}

//===----------------------------------------------------------------------===//
// Op base classes
//===----------------------------------------------------------------------===//

class Arm_PureBinOp<string mnemonic> : Arm_Op<mnemonic, [Pure]> {
  let arguments = (ins I32:$lhs, I32:$rhs);
  let results   = (outs I32:$result);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

class Arm_FlagsBinOp<string mnemonic> : Arm_Op<mnemonic, [Pure]> {
  let arguments = (ins I32:$lhs, I32:$rhs);
  let results   = (outs I32:$result, I1:$n, I1:$z, I1:$c, I1:$v);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

class Arm_ShiftOp<string mnemonic> : Arm_Op<mnemonic, [Pure]> {
  let arguments = (ins I32:$value, I32:$amount);
  let results   = (outs I32:$result);
  let assemblyFormat = "$value `,` $amount attr-dict `:` type($result)";
}

class Arm_CmpLikeOp<string mnemonic> : Arm_Op<mnemonic, [Pure]> {
  let arguments = (ins I32:$lhs, I32:$rhs);
  let results   = (outs I1:$n, I1:$z, I1:$c, I1:$v);
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($lhs)";
}

//===----------------------------------------------------------------------===//
// Pure arithmetic (no flag effect)
//===----------------------------------------------------------------------===//

def Arm_AddOp : Arm_PureBinOp<"add">  { let summary = "ARM ADD"; }
def Arm_SubOp : Arm_PureBinOp<"sub">  { let summary = "ARM SUB"; }
def Arm_MulOp : Arm_PureBinOp<"mul">  { let summary = "ARM MUL"; }
def Arm_AndOp : Arm_PureBinOp<"and">  { let summary = "ARM AND"; }
def Arm_OrrOp : Arm_PureBinOp<"orr">  { let summary = "ARM ORR"; }
def Arm_EorOp : Arm_PureBinOp<"eor">  { let summary = "ARM EOR"; }
def Arm_BicOp : Arm_PureBinOp<"bic">  { let summary = "ARM BIC (a AND NOT b)"; }

//===----------------------------------------------------------------------===//
// Flag-setting arithmetic
//===----------------------------------------------------------------------===//

def Arm_AddsOp : Arm_FlagsBinOp<"adds"> { let summary = "ARM ADDS"; }
def Arm_SubsOp : Arm_FlagsBinOp<"subs"> { let summary = "ARM SUBS"; }
def Arm_AndsOp : Arm_FlagsBinOp<"ands"> { let summary = "ARM ANDS"; }
def Arm_OrrsOp : Arm_FlagsBinOp<"orrs"> { let summary = "ARM ORRS"; }
def Arm_EorsOp : Arm_FlagsBinOp<"eors"> { let summary = "ARM EORS"; }
def Arm_BicsOp : Arm_FlagsBinOp<"bics"> { let summary = "ARM BICS"; }

//===----------------------------------------------------------------------===//
// Shifts
//===----------------------------------------------------------------------===//

def Arm_LslOp : Arm_ShiftOp<"lsl"> { let summary = "ARM LSL (logical left)"; }
def Arm_LsrOp : Arm_ShiftOp<"lsr"> { let summary = "ARM LSR (logical right)"; }
def Arm_AsrOp : Arm_ShiftOp<"asr"> { let summary = "ARM ASR (arithmetic right)"; }
def Arm_RorOp : Arm_ShiftOp<"ror"> { let summary = "ARM ROR (rotate right)"; }

//===----------------------------------------------------------------------===//
// Compares (produce flags only, no value)
//===----------------------------------------------------------------------===//

def Arm_CmpOp : Arm_CmpLikeOp<"cmp"> { let summary = "ARM CMP (a - b → flags)"; }
def Arm_TstOp : Arm_CmpLikeOp<"tst"> { let summary = "ARM TST (a AND b → flags)"; }

//===----------------------------------------------------------------------===//
// Move helpers (immediate halves; full 32-bit immediates use arith.constant)
//===----------------------------------------------------------------------===//

def Arm_MovwOp : Arm_Op<"movw", [Pure]> {
  let summary = "ARM MOVW (set lower 16 bits, zero upper)";
  let arguments = (ins I32Attr:$imm);
  let results   = (outs I32:$result);
  let assemblyFormat = "$imm attr-dict `:` type($result)";
}

def Arm_MovtOp : Arm_Op<"movt", [Pure]> {
  let summary = "ARM MOVT (set upper 16 bits, preserve lower)";
  let arguments = (ins I32:$lower, I32Attr:$imm);
  let results   = (outs I32:$result);
  let assemblyFormat = "$lower `,` $imm attr-dict `:` type($result)";
}

//===----------------------------------------------------------------------===//
// Memory
//
// Why custom ops instead of `memref`? `memref.load` carries an in-bounds
// invariant that lowers to LLVM `inbounds`/UB-on-OOB. That's wrong for raw
// guest memory, where loads outside any tracked allocation are routine.
// We keep memory effects explicit and lower to `llvm.load`/`llvm.store`
// directly in M6.
//===----------------------------------------------------------------------===//

def Arm_LoadOp : Arm_Op<"load",
    [MemoryEffects<[MemRead]>]> {
  let summary = "ARM LDR (32-bit load)";
  let arguments = (ins I32:$addr);
  let results   = (outs I32:$value);
  let assemblyFormat = "$addr attr-dict `:` type($addr) `->` type($value)";
}

def Arm_LoadByteOp : Arm_Op<"loadb",
    [MemoryEffects<[MemRead]>]> {
  let summary = "ARM LDRB (zero-extending byte load into i32)";
  let arguments = (ins I32:$addr);
  let results   = (outs I32:$value);
  let assemblyFormat = "$addr attr-dict `:` type($addr) `->` type($value)";
}

def Arm_StoreOp : Arm_Op<"store",
    [MemoryEffects<[MemWrite]>]> {
  let summary = "ARM STR (32-bit store)";
  let arguments = (ins I32:$value, I32:$addr);
  let assemblyFormat = "$value `,` $addr attr-dict `:` type($value) `,` type($addr)";
}

def Arm_StoreByteOp : Arm_Op<"storeb",
    [MemoryEffects<[MemWrite]>]> {
  let summary = "ARM STRB (byte store; low 8 bits of value)";
  let arguments = (ins I32:$value, I32:$addr);
  let assemblyFormat = "$value `,` $addr attr-dict `:` type($value) `,` type($addr)";
}

//===----------------------------------------------------------------------===//
// Condition predicate materialization
//
// Takes the four NZCV flag SSA values plus a condition attribute, returns an
// i1 carrying the predicate's truth. The lifter emits these immediately
// before a conditional branch / conditionally-executed instruction; M4
// canonicalizes them down to direct boolean expressions on flags.
//===----------------------------------------------------------------------===//

def Arm_CondCheckOp : Arm_Op<"cond_check", [Pure]> {
  let summary = "Materialize ARM condition predicate as i1";
  let arguments = (ins Arm_CondAttr:$cond,
                       I1:$n, I1:$z, I1:$c, I1:$v);
  let results   = (outs I1:$result);
  let assemblyFormat = "$cond `,` $n `,` $z `,` $c `,` $v attr-dict";
}

//===----------------------------------------------------------------------===//
// Misc
//===----------------------------------------------------------------------===//

def Arm_NopOp : Arm_Op<"nop", [Pure]> {
  let summary = "ARM NOP";
  let assemblyFormat = "attr-dict";
}

// `arm.unhandled` is a placeholder for an A32 instruction we have decoded
// (we know its mnemonic and raw bits) but haven't taught the lifter to
// translate yet. Conservative memory effects so DCE leaves it alone.
def Arm_UnhandledOp : Arm_Op<"unhandled",
    [MemoryEffects<[MemRead, MemWrite]>]> {
  let summary = "Placeholder for an unimplemented A32 instruction";
  let arguments = (ins StrAttr:$mnemonic, I32Attr:$pc, I32Attr:$insn);
  let assemblyFormat = "$mnemonic `pc` $pc `insn` $insn attr-dict";
}

#endif // ARM_OPS_TD
