#ifndef ARM_DIALECT_TD
#define ARM_DIALECT_TD

include "mlir/IR/OpBase.td"

def Arm_Dialect : Dialect {
  let name = "arm";
  let cppNamespace = "::mlir::arm";
  let summary = "Toy ARM32 (A32) machine-semantics dialect";
  let description = [{
    The `arm` dialect models ARM32 (A32) machine semantics for a toy
    decompiler pipeline. Operations represent exact instruction behavior
    including general-purpose registers, condition flags (NZCV), and memory
    effects. The dialect is intended as the *first stop* after raw decoding;
    later passes lower it into `arith`, `cf`, `scf`, and finally either
    `llvm` or `emitc`.
  }];
}

class Arm_Op<string mnemonic, list<Trait> traits = []>
    : Op<Arm_Dialect, mnemonic, traits>;

#endif // ARM_DIALECT_TD
