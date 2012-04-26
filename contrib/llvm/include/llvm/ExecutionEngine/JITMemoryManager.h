//===-- JITMemoryManager.h - Interface JIT uses to Allocate Mem -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTION_ENGINE_JIT_MEMMANAGER_H
#define LLVM_EXECUTION_ENGINE_JIT_MEMMANAGER_H

#include "llvm/Support/DataTypes.h"
#include <string>

namespace llvm {

  class Function;
  class GlobalValue;

/// JITMemoryManager - This interface is used by the JIT to allocate and manage
/// memory for the code generated by the JIT.  This can be reimplemented by
/// clients that have a strong desire to control how the layout of JIT'd memory
/// works.
class JITMemoryManager {
protected:
  bool HasGOT;

public:
  JITMemoryManager() : HasGOT(false) {}
  virtual ~JITMemoryManager();

  /// CreateDefaultMemManager - This is used to create the default
  /// JIT Memory Manager if the client does not provide one to the JIT.
  static JITMemoryManager *CreateDefaultMemManager();

  /// setMemoryWritable - When code generation is in progress,
  /// the code pages may need permissions changed.
  virtual void setMemoryWritable() = 0;

  /// setMemoryExecutable - When code generation is done and we're ready to
  /// start execution, the code pages may need permissions changed.
  virtual void setMemoryExecutable() = 0;

  /// setPoisonMemory - Setting this flag to true makes the memory manager
  /// garbage values over freed memory.  This is useful for testing and
  /// debugging, and may be turned on by default in debug mode.
  virtual void setPoisonMemory(bool poison) = 0;

  /// getPointerToNamedFunction - This method returns the address of the
  /// specified function. As such it is only useful for resolving library
  /// symbols, not code generated symbols.
  ///
  /// If AbortOnFailure is false and no function with the given name is
  /// found, this function silently returns a null pointer. Otherwise,
  /// it prints a message to stderr and aborts.
  ///
  virtual void *getPointerToNamedFunction(const std::string &Name,
                                          bool AbortOnFailure = true) = 0;

  //===--------------------------------------------------------------------===//
  // Global Offset Table Management
  //===--------------------------------------------------------------------===//

  /// AllocateGOT - If the current table requires a Global Offset Table, this
  /// method is invoked to allocate it.  This method is required to set HasGOT
  /// to true.
  virtual void AllocateGOT() = 0;

  /// isManagingGOT - Return true if the AllocateGOT method is called.
  bool isManagingGOT() const {
    return HasGOT;
  }

  /// getGOTBase - If this is managing a Global Offset Table, this method should
  /// return a pointer to its base.
  virtual uint8_t *getGOTBase() const = 0;

  //===--------------------------------------------------------------------===//
  // Main Allocation Functions
  //===--------------------------------------------------------------------===//

  /// startFunctionBody - When we start JITing a function, the JIT calls this
  /// method to allocate a block of free RWX memory, which returns a pointer to
  /// it.  If the JIT wants to request a block of memory of at least a certain
  /// size, it passes that value as ActualSize, and this method returns a block
  /// with at least that much space.  If the JIT doesn't know ahead of time how
  /// much space it will need to emit the function, it passes 0 for the
  /// ActualSize.  In either case, this method is required to pass back the size
  /// of the allocated block through ActualSize.  The JIT will be careful to
  /// not write more than the returned ActualSize bytes of memory.
  virtual uint8_t *startFunctionBody(const Function *F,
                                     uintptr_t &ActualSize) = 0;

  /// allocateStub - This method is called by the JIT to allocate space for a
  /// function stub (used to handle limited branch displacements) while it is
  /// JIT compiling a function.  For example, if foo calls bar, and if bar
  /// either needs to be lazily compiled or is a native function that exists too
  /// far away from the call site to work, this method will be used to make a
  /// thunk for it.  The stub should be "close" to the current function body,
  /// but should not be included in the 'actualsize' returned by
  /// startFunctionBody.
  virtual uint8_t *allocateStub(const GlobalValue* F, unsigned StubSize,
                                unsigned Alignment) = 0;

  /// endFunctionBody - This method is called when the JIT is done codegen'ing
  /// the specified function.  At this point we know the size of the JIT
  /// compiled function.  This passes in FunctionStart (which was returned by
  /// the startFunctionBody method) and FunctionEnd which is a pointer to the
  /// actual end of the function.  This method should mark the space allocated
  /// and remember where it is in case the client wants to deallocate it.
  virtual void endFunctionBody(const Function *F, uint8_t *FunctionStart,
                               uint8_t *FunctionEnd) = 0;

  /// allocateCodeSection - Allocate a memory block of (at least) the given
  /// size suitable for executable code. The SectionID is a unique identifier
  /// assigned by the JIT and passed through to the memory manager for
  /// the instance class to use if it needs to communicate to the JIT about
  /// a given section after the fact.
  virtual uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
                                       unsigned SectionID) = 0;

  /// allocateDataSection - Allocate a memory block of (at least) the given
  /// size suitable for data. The SectionID is a unique identifier
  /// assigned by the JIT and passed through to the memory manager for
  /// the instance class to use if it needs to communicate to the JIT about
  /// a given section after the fact.
  virtual uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
                                       unsigned SectionID) = 0;

  /// allocateSpace - Allocate a memory block of the given size.  This method
  /// cannot be called between calls to startFunctionBody and endFunctionBody.
  virtual uint8_t *allocateSpace(intptr_t Size, unsigned Alignment) = 0;

  /// allocateGlobal - Allocate memory for a global.
  virtual uint8_t *allocateGlobal(uintptr_t Size, unsigned Alignment) = 0;

  /// deallocateFunctionBody - Free the specified function body.  The argument
  /// must be the return value from a call to startFunctionBody() that hasn't
  /// been deallocated yet.  This is never called when the JIT is currently
  /// emitting a function.
  virtual void deallocateFunctionBody(void *Body) = 0;

  /// startExceptionTable - When we finished JITing the function, if exception
  /// handling is set, we emit the exception table.
  virtual uint8_t* startExceptionTable(const Function* F,
                                       uintptr_t &ActualSize) = 0;

  /// endExceptionTable - This method is called when the JIT is done emitting
  /// the exception table.
  virtual void endExceptionTable(const Function *F, uint8_t *TableStart,
                                 uint8_t *TableEnd, uint8_t* FrameRegister) = 0;

  /// deallocateExceptionTable - Free the specified exception table's memory.
  /// The argument must be the return value from a call to startExceptionTable()
  /// that hasn't been deallocated yet.  This is never called when the JIT is
  /// currently emitting an exception table.
  virtual void deallocateExceptionTable(void *ET) = 0;

  /// CheckInvariants - For testing only.  Return true if all internal
  /// invariants are preserved, or return false and set ErrorStr to a helpful
  /// error message.
  virtual bool CheckInvariants(std::string &) {
    return true;
  }

  /// GetDefaultCodeSlabSize - For testing only.  Returns DefaultCodeSlabSize
  /// from DefaultJITMemoryManager.
  virtual size_t GetDefaultCodeSlabSize() {
    return 0;
  }

  /// GetDefaultDataSlabSize - For testing only.  Returns DefaultCodeSlabSize
  /// from DefaultJITMemoryManager.
  virtual size_t GetDefaultDataSlabSize() {
    return 0;
  }

  /// GetDefaultStubSlabSize - For testing only.  Returns DefaultCodeSlabSize
  /// from DefaultJITMemoryManager.
  virtual size_t GetDefaultStubSlabSize() {
    return 0;
  }

  /// GetNumCodeSlabs - For testing only.  Returns the number of MemoryBlocks
  /// allocated for code.
  virtual unsigned GetNumCodeSlabs() {
    return 0;
  }

  /// GetNumDataSlabs - For testing only.  Returns the number of MemoryBlocks
  /// allocated for data.
  virtual unsigned GetNumDataSlabs() {
    return 0;
  }

  /// GetNumStubSlabs - For testing only.  Returns the number of MemoryBlocks
  /// allocated for function stubs.
  virtual unsigned GetNumStubSlabs() {
    return 0;
  }
};

} // end namespace llvm.

#endif
