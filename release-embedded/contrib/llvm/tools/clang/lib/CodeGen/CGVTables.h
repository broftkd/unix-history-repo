//===--- CGVTables.h - Emit LLVM Code for C++ vtables -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with C++ code generation of virtual tables.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CODEGEN_CGVTABLE_H
#define CLANG_CODEGEN_CGVTABLE_H

#include "clang/AST/BaseSubobject.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/GlobalDecl.h"
#include "clang/AST/VTableBuilder.h"
#include "clang/Basic/ABI.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/GlobalVariable.h"

namespace clang {
  class CXXRecordDecl;

namespace CodeGen {
  class CodeGenModule;

class CodeGenVTables {
  CodeGenModule &CGM;

  // FIXME: Consider moving ItaniumVTContext and MicrosoftVTContext into
  // respective CXXABI classes?
  ItaniumVTableContext ItaniumVTContext;
  OwningPtr<MicrosoftVTableContext> MicrosoftVTContext;
  
  /// VTableAddressPointsMapTy - Address points for a single vtable.
  typedef llvm::DenseMap<BaseSubobject, uint64_t> VTableAddressPointsMapTy;

  typedef std::pair<const CXXRecordDecl *, BaseSubobject> BaseSubobjectPairTy;
  typedef llvm::DenseMap<BaseSubobjectPairTy, uint64_t> SubVTTIndiciesMapTy;
  
  /// SubVTTIndicies - Contains indices into the various sub-VTTs.
  SubVTTIndiciesMapTy SubVTTIndicies;

  typedef llvm::DenseMap<BaseSubobjectPairTy, uint64_t>
    SecondaryVirtualPointerIndicesMapTy;

  /// SecondaryVirtualPointerIndices - Contains the secondary virtual pointer
  /// indices.
  SecondaryVirtualPointerIndicesMapTy SecondaryVirtualPointerIndices;

  /// emitThunk - Emit a single thunk.
  void emitThunk(GlobalDecl GD, const ThunkInfo &Thunk, bool ForVTable);

  /// maybeEmitThunkForVTable - Emit the given thunk for the vtable if needed by
  /// the ABI.
  void maybeEmitThunkForVTable(GlobalDecl GD, const ThunkInfo &Thunk);

public:
  /// CreateVTableInitializer - Create a vtable initializer for the given record
  /// decl.
  /// \param Components - The vtable components; this is really an array of
  /// VTableComponents.
  llvm::Constant *CreateVTableInitializer(const CXXRecordDecl *RD,
                                          const VTableComponent *Components, 
                                          unsigned NumComponents,
                                const VTableLayout::VTableThunkTy *VTableThunks,
                                          unsigned NumVTableThunks);

  CodeGenVTables(CodeGenModule &CGM);

  ItaniumVTableContext &getItaniumVTableContext() { return ItaniumVTContext; }

  MicrosoftVTableContext &getMicrosoftVTableContext() {
    return *MicrosoftVTContext.get();
  }

  /// getSubVTTIndex - Return the index of the sub-VTT for the base class of the
  /// given record decl.
  uint64_t getSubVTTIndex(const CXXRecordDecl *RD, BaseSubobject Base);
  
  /// getSecondaryVirtualPointerIndex - Return the index in the VTT where the
  /// virtual pointer for the given subobject is located.
  uint64_t getSecondaryVirtualPointerIndex(const CXXRecordDecl *RD,
                                           BaseSubobject Base);

  /// getAddressPoint - Get the address point of the given subobject in the
  /// class decl.
  uint64_t getAddressPoint(BaseSubobject Base, const CXXRecordDecl *RD);
  
  /// GenerateConstructionVTable - Generate a construction vtable for the given 
  /// base subobject.
  llvm::GlobalVariable *
  GenerateConstructionVTable(const CXXRecordDecl *RD, const BaseSubobject &Base, 
                             bool BaseIsVirtual, 
                             llvm::GlobalVariable::LinkageTypes Linkage,
                             VTableAddressPointsMapTy& AddressPoints);

    
  /// GetAddrOfVTable - Get the address of the VTT for the given record decl.
  llvm::GlobalVariable *GetAddrOfVTT(const CXXRecordDecl *RD);

  /// EmitVTTDefinition - Emit the definition of the given vtable.
  void EmitVTTDefinition(llvm::GlobalVariable *VTT,
                         llvm::GlobalVariable::LinkageTypes Linkage,
                         const CXXRecordDecl *RD);

  /// EmitThunks - Emit the associated thunks for the given global decl.
  void EmitThunks(GlobalDecl GD);
    
  /// GenerateClassData - Generate all the class data required to be
  /// generated upon definition of a KeyFunction.  This includes the
  /// vtable, the RTTI data structure (if RTTI is enabled) and the VTT
  /// (if the class has virtual bases).
  void GenerateClassData(const CXXRecordDecl *RD);

  bool isVTableExternal(const CXXRecordDecl *RD);
};

} // end namespace CodeGen
} // end namespace clang
#endif
