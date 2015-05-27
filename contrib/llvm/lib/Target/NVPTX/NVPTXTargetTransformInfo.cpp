//===-- NVPTXTargetTransformInfo.cpp - NVPTX specific TTI -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NVPTXTargetTransformInfo.h"
#include "NVPTXUtilities.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/CostTable.h"
#include "llvm/Target/TargetLowering.h"
using namespace llvm;

#define DEBUG_TYPE "NVPTXtti"

// Whether the given intrinsic reads threadIdx.x/y/z.
static bool readsThreadIndex(const IntrinsicInst *II) {
  switch (II->getIntrinsicID()) {
    default: return false;
    case Intrinsic::nvvm_read_ptx_sreg_tid_x:
    case Intrinsic::nvvm_read_ptx_sreg_tid_y:
    case Intrinsic::nvvm_read_ptx_sreg_tid_z:
      return true;
  }
}

static bool readsLaneId(const IntrinsicInst *II) {
  return II->getIntrinsicID() == Intrinsic::ptx_read_laneid;
}

// Whether the given intrinsic is an atomic instruction in PTX.
static bool isNVVMAtomic(const IntrinsicInst *II) {
  switch (II->getIntrinsicID()) {
    default: return false;
    case Intrinsic::nvvm_atomic_load_add_f32:
    case Intrinsic::nvvm_atomic_load_inc_32:
    case Intrinsic::nvvm_atomic_load_dec_32:
      return true;
  }
}

bool NVPTXTTIImpl::isSourceOfDivergence(const Value *V) {
  // Without inter-procedural analysis, we conservatively assume that arguments
  // to __device__ functions are divergent.
  if (const Argument *Arg = dyn_cast<Argument>(V))
    return !isKernelFunction(*Arg->getParent());

  if (const Instruction *I = dyn_cast<Instruction>(V)) {
    // Without pointer analysis, we conservatively assume values loaded from
    // generic or local address space are divergent.
    if (const LoadInst *LI = dyn_cast<LoadInst>(I)) {
      unsigned AS = LI->getPointerAddressSpace();
      return AS == ADDRESS_SPACE_GENERIC || AS == ADDRESS_SPACE_LOCAL;
    }
    // Atomic instructions may cause divergence. Atomic instructions are
    // executed sequentially across all threads in a warp. Therefore, an earlier
    // executed thread may see different memory inputs than a later executed
    // thread. For example, suppose *a = 0 initially.
    //
    //   atom.global.add.s32 d, [a], 1
    //
    // returns 0 for the first thread that enters the critical region, and 1 for
    // the second thread.
    if (I->isAtomic())
      return true;
    if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(I)) {
      // Instructions that read threadIdx are obviously divergent.
      if (readsThreadIndex(II) || readsLaneId(II))
        return true;
      // Handle the NVPTX atomic instrinsics that cannot be represented as an
      // atomic IR instruction.
      if (isNVVMAtomic(II))
        return true;
    }
    // Conservatively consider the return value of function calls as divergent.
    // We could analyze callees with bodies more precisely using
    // inter-procedural analysis.
    if (isa<CallInst>(I))
      return true;
  }

  return false;
}

unsigned NVPTXTTIImpl::getArithmeticInstrCost(
    unsigned Opcode, Type *Ty, TTI::OperandValueKind Opd1Info,
    TTI::OperandValueKind Opd2Info, TTI::OperandValueProperties Opd1PropInfo,
    TTI::OperandValueProperties Opd2PropInfo) {
  // Legalize the type.
  std::pair<unsigned, MVT> LT = TLI->getTypeLegalizationCost(Ty);

  int ISD = TLI->InstructionOpcodeToISD(Opcode);

  switch (ISD) {
  default:
    return BaseT::getArithmeticInstrCost(Opcode, Ty, Opd1Info, Opd2Info,
                                         Opd1PropInfo, Opd2PropInfo);
  case ISD::ADD:
  case ISD::MUL:
  case ISD::XOR:
  case ISD::OR:
  case ISD::AND:
    // The machine code (SASS) simulates an i64 with two i32. Therefore, we
    // estimate that arithmetic operations on i64 are twice as expensive as
    // those on types that can fit into one machine register.
    if (LT.second.SimpleTy == MVT::i64)
      return 2 * LT.first;
    // Delegate other cases to the basic TTI.
    return BaseT::getArithmeticInstrCost(Opcode, Ty, Opd1Info, Opd2Info,
                                         Opd1PropInfo, Opd2PropInfo);
  }
}
