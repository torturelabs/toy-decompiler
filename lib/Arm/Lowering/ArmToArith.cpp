#include "Arm/ArmOps.h"
#include "Arm/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

// Direct two-input rewrites:
//   arm.add → arith.addi, etc. The arm op and the arith op have identical
//   operand+result shapes here, so the body is a one-liner.
template <typename ArmOp, typename ArithOp>
struct DirectBinRewrite : public OpRewritePattern<ArmOp> {
  using OpRewritePattern<ArmOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(ArmOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<ArithOp>(op, op.getLhs(), op.getRhs());
    return success();
  }
};

template <typename ArmOp, typename ArithOp>
struct DirectShiftRewrite : public OpRewritePattern<ArmOp> {
  using OpRewritePattern<ArmOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(ArmOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<ArithOp>(op, op.getValue(), op.getAmount());
    return success();
  }
};

// arm.bic %a, %b  ≡  a AND NOT b  →  arith.andi %a, (arith.xori %b, -1)
struct BicLowering : public OpRewritePattern<arm::BicOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arm::BicOp op,
                                PatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto i32 = rewriter.getI32Type();
    auto allOnes =
        arith::ConstantIntOp::create(rewriter, loc, i32, /*value=*/-1);
    auto notRhs = arith::XOrIOp::create(rewriter, loc, op.getRhs(), allOnes);
    rewriter.replaceOpWithNewOp<arith::AndIOp>(op, op.getLhs(), notRhs);
    return success();
  }
};

struct NopErasure : public OpRewritePattern<arm::NopOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arm::NopOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.eraseOp(op);
    return success();
  }
};

struct ArmToArithPass
    : public PassWrapper<ArmToArithPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ArmToArithPass)

  StringRef getArgument() const final { return "arm-to-arith"; }
  StringRef getDescription() const final {
    return "Lower no-flags arm.* ops (arithmetic, bitwise, shift, nop) to arith.*";
  }
  void getDependentDialects(DialectRegistry &registry) const final {
    registry.insert<arith::ArithDialect>();
  }

  void runOnOperation() final {
    RewritePatternSet patterns(&getContext());
    auto *ctx = &getContext();

    patterns.add<DirectBinRewrite<arm::AddOp, arith::AddIOp>>(ctx);
    patterns.add<DirectBinRewrite<arm::SubOp, arith::SubIOp>>(ctx);
    patterns.add<DirectBinRewrite<arm::MulOp, arith::MulIOp>>(ctx);
    patterns.add<DirectBinRewrite<arm::AndOp, arith::AndIOp>>(ctx);
    patterns.add<DirectBinRewrite<arm::OrrOp, arith::OrIOp>>(ctx);
    patterns.add<DirectBinRewrite<arm::EorOp, arith::XOrIOp>>(ctx);
    patterns.add<DirectShiftRewrite<arm::LslOp, arith::ShLIOp>>(ctx);
    patterns.add<DirectShiftRewrite<arm::LsrOp, arith::ShRUIOp>>(ctx);
    patterns.add<DirectShiftRewrite<arm::AsrOp, arith::ShRSIOp>>(ctx);
    patterns.add<BicLowering, NopErasure>(ctx);

    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      signalPassFailure();
  }
};

}  // namespace

namespace mlir {
namespace arm {

std::unique_ptr<Pass> createArmToArithPass() {
  return std::make_unique<ArmToArithPass>();
}

void registerArmPasses() {
  PassRegistration<ArmToArithPass>();
  registerRecoverFunctionSignaturePass();
}

}  // namespace arm
}  // namespace mlir
