//===- DLL.h ----------------------------------------------------*- C++ -*-===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_COFF_DLL_H
#define LLD_COFF_DLL_H

#include "Chunks.h"
#include "Symbols.h"

namespace lld {
namespace coff {

// Windows-specific.
// IdataContents creates all chunks for the DLL import table.
// You are supposed to call add() to add symbols and then
// call getChunks() to get a list of chunks.
class IdataContents {
public:
  void add(DefinedImportData *Sym) { Imports.push_back(Sym); }
  bool empty() { return Imports.empty(); }
  std::vector<Chunk *> getChunks();

  uint64_t getDirRVA() { return Dirs[0]->getRVA(); }
  uint64_t getDirSize();
  uint64_t getIATRVA() { return Addresses[0]->getRVA(); }
  uint64_t getIATSize();

private:
  void create();

  std::vector<DefinedImportData *> Imports;
  std::vector<std::unique_ptr<Chunk>> Dirs;
  std::vector<std::unique_ptr<Chunk>> Lookups;
  std::vector<std::unique_ptr<Chunk>> Addresses;
  std::vector<std::unique_ptr<Chunk>> Hints;
  std::map<StringRef, std::unique_ptr<Chunk>> DLLNames;
};

// Windows-specific.
// DelayLoadContents creates all chunks for the delay-load DLL import table.
class DelayLoadContents {
public:
  void add(DefinedImportData *Sym) { Imports.push_back(Sym); }
  bool empty() { return Imports.empty(); }
  void create(Defined *Helper);
  std::vector<Chunk *> getChunks();
  std::vector<Chunk *> getDataChunks();
  std::vector<std::unique_ptr<Chunk>> &getCodeChunks() { return Thunks; }

  uint64_t getDirRVA() { return Dirs[0]->getRVA(); }
  uint64_t getDirSize();

private:
  Chunk *newThunkChunk(DefinedImportData *S, Chunk *Dir);

  Defined *Helper;
  std::vector<DefinedImportData *> Imports;
  std::vector<std::unique_ptr<Chunk>> Dirs;
  std::vector<std::unique_ptr<Chunk>> ModuleHandles;
  std::vector<std::unique_ptr<Chunk>> Addresses;
  std::vector<std::unique_ptr<Chunk>> Names;
  std::vector<std::unique_ptr<Chunk>> HintNames;
  std::vector<std::unique_ptr<Chunk>> Thunks;
  std::map<StringRef, std::unique_ptr<Chunk>> DLLNames;
};

// Windows-specific.
// EdataContents creates all chunks for the DLL export table.
class EdataContents {
public:
  EdataContents();
  std::vector<std::unique_ptr<Chunk>> Chunks;
};

} // namespace coff
} // namespace lld

#endif
