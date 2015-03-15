//===-- ForwardControlFlowIntegrity.h: Forward-Edge CFI ---------*- C++ -*-===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass instruments indirect calls with checks to ensure that these calls
// pass through the appropriate jump-instruction table generated by
// JumpInstrTables.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_FORWARDCONTROLFLOWINTEGRITY_H
#define LLVM_CODEGEN_FORWARDCONTROLFLOWINTEGRITY_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetOptions.h"
#include <string>

namespace llvm {

class AnalysisUsage;
class BasicBlock;
class Constant;
class Function;
class Instruction;
class Module;
class Value;

/// ForwardControlFlowIntegrity uses the information from JumpInstrTableInfo to
/// prepend checks to indirect calls to make sure that these calls target valid
/// locations.
class ForwardControlFlowIntegrity : public ModulePass {
public:
  static char ID;

  ForwardControlFlowIntegrity();
  ForwardControlFlowIntegrity(JumpTable::JumpTableType JTT,
                              CFIntegrity CFIType,
                              bool CFIEnforcing, std::string CFIFuncName);
  ~ForwardControlFlowIntegrity() override;

  /// Runs the CFI pass on a given module. This works best if the module in
  /// question is the result of link-time optimization (see lib/LTO).
  bool runOnModule(Module &M) override;
  const char *getPassName() const override {
    return "Forward Control-Flow Integrity";
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  typedef SmallVector<Instruction *, 64> CallSet;

  /// A structure that is used to keep track of constant table information.
  struct CFIConstants {
    Constant *StartValue;
    Constant *MaskValue;
    Constant *Size;
  };

  /// A map from function type to the base of the table for this type and a mask
  /// for the table
  typedef DenseMap<FunctionType *, CFIConstants> CFITables;

  CallSet IndirectCalls;

  /// The type of jumptable implementation.
  JumpTable::JumpTableType JTType;

  /// The type of CFI check to add before each indirect call.
  CFIntegrity CFIType;

  /// A value that controls whether or not CFI violations cause a halt.
  bool CFIEnforcing;

  /// The name of the function to call in case of a CFI violation when
  /// CFIEnforcing is false. There is a default function that ignores
  /// violations.
  std::string CFIFuncName;

  /// The alignment of each entry in the table, from JumpInstrTableInfo. The
  /// JumpInstrTableInfo class always makes this a power of two.
  uint64_t ByteAlignment;

  /// The base-2 logarithm of ByteAlignment, needed for some of the transforms
  /// (like CFIntegrity::Ror)
  unsigned LogByteAlignment;

  /// Adds checks to each indirect call site to make sure that it is calling a
  /// function in our jump table.
  void updateIndirectCalls(Module &M, CFITables &CFIT);

  /// Walks the instructions to find all the indirect calls.
  void getIndirectCalls(Module &M);

  /// Adds a function that handles violations in non-enforcing mode
  /// (!CFIEnforcing). The default warning function simply returns, since the
  /// exact details of how to handle CFI violations depend on the application.
  void addWarningFunction(Module &M);

  /// Rewrites a function pointer in a call/invoke instruction to force it into
  /// a table.
  void rewriteFunctionPointer(Module &M, Instruction *I, Value *FunPtr,
                              Constant *JumpTableStart, Constant *JumpTableMask,
                              Constant *JumpTableSize);

  /// Inserts a check and a call to a warning function at a given instruction
  /// that must be an indirect call.
  void insertWarning(Module &M, BasicBlock *Block, Instruction *I,
                     Value *FunPtr);
};

ModulePass *
createForwardControlFlowIntegrityPass(JumpTable::JumpTableType JTT,
                                      CFIntegrity CFIType,
                                      bool CFIEnforcing, StringRef CFIFuncName);
}

#endif // LLVM_CODEGEN_FORWARDCONTROLFLOWINTEGRITY_H
