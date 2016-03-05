//===-- asan_report.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// ASan-private header for error reporting functions.
//===----------------------------------------------------------------------===//

#include "asan_allocator.h"
#include "asan_internal.h"
#include "asan_thread.h"

namespace __asan {

struct StackVarDescr {
  uptr beg;
  uptr size;
  const char *name_pos;
  uptr name_len;
};

struct AddressDescription {
  char *name;
  uptr name_size;
  uptr region_address;
  uptr region_size;
  const char *region_kind;
};

// Returns the number of globals close to the provided address and copies
// them to "globals" array.
int GetGlobalsForAddress(uptr addr, __asan_global *globals, u32 *reg_sites,
                         int max_globals);
bool GetInfoForAddressIfGlobal(uptr addr, AddressDescription *descr);
// The following functions prints address description depending
// on the memory type (shadow/heap/stack/global).
void DescribeHeapAddress(uptr addr, uptr access_size);
bool DescribeAddressIfShadow(uptr addr, AddressDescription *descr = nullptr,
                             bool print = true);
bool ParseFrameDescription(const char *frame_descr,
                           InternalMmapVector<StackVarDescr> *vars);
bool DescribeAddressIfStack(uptr addr, uptr access_size);
void DescribeThread(AsanThreadContext *context);

// Different kinds of error reports.
void ReportGenericError(uptr pc, uptr bp, uptr sp, uptr addr, bool is_write,
                        uptr access_size, u32 exp, bool fatal);
void ReportStackOverflow(const SignalContext &sig);
void ReportDeadlySignal(const char *description, const SignalContext &sig);
void ReportNewDeleteSizeMismatch(uptr addr, uptr delete_size,
                                 BufferedStackTrace *free_stack);
void ReportDoubleFree(uptr addr, BufferedStackTrace *free_stack);
void ReportFreeNotMalloced(uptr addr, BufferedStackTrace *free_stack);
void ReportAllocTypeMismatch(uptr addr, BufferedStackTrace *free_stack,
                             AllocType alloc_type,
                             AllocType dealloc_type);
void ReportMallocUsableSizeNotOwned(uptr addr, BufferedStackTrace *stack);
void ReportSanitizerGetAllocatedSizeNotOwned(uptr addr,
                                             BufferedStackTrace *stack);
void ReportStringFunctionMemoryRangesOverlap(const char *function,
                                             const char *offset1, uptr length1,
                                             const char *offset2, uptr length2,
                                             BufferedStackTrace *stack);
void ReportStringFunctionSizeOverflow(uptr offset, uptr size,
                                      BufferedStackTrace *stack);
void ReportBadParamsToAnnotateContiguousContainer(uptr beg, uptr end,
                                                  uptr old_mid, uptr new_mid,
                                                  BufferedStackTrace *stack);

void ReportODRViolation(const __asan_global *g1, u32 stack_id1,
                        const __asan_global *g2, u32 stack_id2);

// Mac-specific errors and warnings.
void ReportMacMzReallocUnknown(uptr addr, uptr zone_ptr,
                               const char *zone_name,
                               BufferedStackTrace *stack);
void ReportMacCfReallocUnknown(uptr addr, uptr zone_ptr,
                               const char *zone_name,
                               BufferedStackTrace *stack);

}  // namespace __asan
