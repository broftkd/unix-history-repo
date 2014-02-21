//===-- MipsMachineFunctionInfo.h - Private data used for Mips ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mips specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MIPS_MACHINE_FUNCTION_INFO_H
#define MIPS_MACHINE_FUNCTION_INFO_H

#include "MipsSubtarget.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include <utility>

namespace llvm {

/// \brief A class derived from PseudoSourceValue that represents a GOT entry
/// resolved by lazy-binding.
class MipsCallEntry : public PseudoSourceValue {
public:
  explicit MipsCallEntry(const StringRef &N);
  explicit MipsCallEntry(const GlobalValue *V);
  virtual bool isConstant(const MachineFrameInfo *) const;
  virtual bool isAliased(const MachineFrameInfo *) const;
  virtual bool mayAlias(const MachineFrameInfo *) const;

private:
  virtual void printCustom(raw_ostream &O) const;
#ifndef NDEBUG
  std::string Name;
  const GlobalValue *Val;
#endif
};

/// MipsFunctionInfo - This class is derived from MachineFunction private
/// Mips target-specific information for each MachineFunction.
class MipsFunctionInfo : public MachineFunctionInfo {
public:
  MipsFunctionInfo(MachineFunction& MF)
   : MF(MF), SRetReturnReg(0), GlobalBaseReg(0), Mips16SPAliasReg(0),
     VarArgsFrameIndex(0), CallsEhReturn(false)
  {}

  ~MipsFunctionInfo();

  unsigned getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

  bool globalBaseRegSet() const;
  unsigned getGlobalBaseReg();

  bool mips16SPAliasRegSet() const;
  unsigned getMips16SPAliasReg();

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  bool hasByvalArg() const { return HasByvalArg; }
  void setFormalArgInfo(unsigned Size, bool HasByval) {
    IncomingArgSize = Size;
    HasByvalArg = HasByval;
  }

  unsigned getIncomingArgSize() const { return IncomingArgSize; }

  bool callsEhReturn() const { return CallsEhReturn; }
  void setCallsEhReturn() { CallsEhReturn = true; }

  void createEhDataRegsFI();
  int getEhDataRegFI(unsigned Reg) const { return EhDataRegFI[Reg]; }
  bool isEhDataRegFI(int FI) const;

  /// \brief Create a MachinePointerInfo that has a MipsCallEntr object
  /// representing a GOT entry for an external function.
  MachinePointerInfo callPtrInfo(const StringRef &Name);

  /// \brief Create a MachinePointerInfo that has a MipsCallEntr object
  /// representing a GOT entry for a global function.
  MachinePointerInfo callPtrInfo(const GlobalValue *Val);

private:
  virtual void anchor();

  MachineFunction& MF;
  /// SRetReturnReg - Some subtargets require that sret lowering includes
  /// returning the value of the returned struct in a register. This field
  /// holds the virtual register into which the sret argument is passed.
  unsigned SRetReturnReg;

  /// GlobalBaseReg - keeps track of the virtual register initialized for
  /// use as the global base register. This is used for PIC in some PIC
  /// relocation models.
  unsigned GlobalBaseReg;

  /// Mips16SPAliasReg - keeps track of the virtual register initialized for
  /// use as an alias for SP for use in load/store of halfword/byte from/to
  /// the stack
  unsigned Mips16SPAliasReg;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

  /// True if function has a byval argument.
  bool HasByvalArg;

  /// Size of incoming argument area.
  unsigned IncomingArgSize;

  /// CallsEhReturn - Whether the function calls llvm.eh.return.
  bool CallsEhReturn;

  /// Frame objects for spilling eh data registers.
  int EhDataRegFI[4];

  /// MipsCallEntry maps.
  StringMap<const MipsCallEntry *> ExternalCallEntries;
  ValueMap<const GlobalValue *, const MipsCallEntry *> GlobalCallEntries;
};

} // end of namespace llvm

#endif // MIPS_MACHINE_FUNCTION_INFO_H
