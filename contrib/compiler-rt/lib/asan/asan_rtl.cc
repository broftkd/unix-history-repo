//===-- asan_rtl.cc -------------------------------------------------------===//
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
// Main file of the ASan run-time library.
//===----------------------------------------------------------------------===//

#include "asan_activation.h"
#include "asan_allocator.h"
#include "asan_interceptors.h"
#include "asan_interface_internal.h"
#include "asan_internal.h"
#include "asan_mapping.h"
#include "asan_poisoning.h"
#include "asan_report.h"
#include "asan_stack.h"
#include "asan_stats.h"
#include "asan_suppressions.h"
#include "asan_thread.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_symbolizer.h"
#include "lsan/lsan_common.h"
#include "ubsan/ubsan_init.h"
#include "ubsan/ubsan_platform.h"

int __asan_option_detect_stack_use_after_return;  // Global interface symbol.
uptr *__asan_test_only_reported_buggy_pointer;  // Used only for testing asan.

namespace __asan {

uptr AsanMappingProfile[kAsanMappingProfileSize];

static void AsanDie() {
  static atomic_uint32_t num_calls;
  if (atomic_fetch_add(&num_calls, 1, memory_order_relaxed) != 0) {
    // Don't die twice - run a busy loop.
    while (1) { }
  }
  if (flags()->sleep_before_dying) {
    Report("Sleeping for %d second(s)\n", flags()->sleep_before_dying);
    SleepForSeconds(flags()->sleep_before_dying);
  }
  if (flags()->unmap_shadow_on_exit) {
    if (kMidMemBeg) {
      UnmapOrDie((void*)kLowShadowBeg, kMidMemBeg - kLowShadowBeg);
      UnmapOrDie((void*)kMidMemEnd, kHighShadowEnd - kMidMemEnd);
    } else {
      UnmapOrDie((void*)kLowShadowBeg, kHighShadowEnd - kLowShadowBeg);
    }
  }
}

static void AsanCheckFailed(const char *file, int line, const char *cond,
                            u64 v1, u64 v2) {
  Report("AddressSanitizer CHECK failed: %s:%d \"%s\" (0x%zx, 0x%zx)\n", file,
         line, cond, (uptr)v1, (uptr)v2);
  // FIXME: check for infinite recursion without a thread-local counter here.
  PRINT_CURRENT_STACK_CHECK();
  Die();
}

// -------------------------- Globals --------------------- {{{1
int asan_inited;
bool asan_init_is_running;

#if !ASAN_FIXED_MAPPING
uptr kHighMemEnd, kMidMemBeg, kMidMemEnd;
#endif

// -------------------------- Misc ---------------- {{{1
void ShowStatsAndAbort() {
  __asan_print_accumulated_stats();
  Die();
}

// ---------------------- mmap -------------------- {{{1
// Reserve memory range [beg, end].
// We need to use inclusive range because end+1 may not be representable.
void ReserveShadowMemoryRange(uptr beg, uptr end, const char *name) {
  CHECK_EQ((beg % GetPageSizeCached()), 0);
  CHECK_EQ(((end + 1) % GetPageSizeCached()), 0);
  uptr size = end - beg + 1;
  DecreaseTotalMmap(size);  // Don't count the shadow against mmap_limit_mb.
  void *res = MmapFixedNoReserve(beg, size, name);
  if (res != (void*)beg) {
    Report("ReserveShadowMemoryRange failed while trying to map 0x%zx bytes. "
           "Perhaps you're using ulimit -v\n", size);
    Abort();
  }
  if (common_flags()->no_huge_pages_for_shadow)
    NoHugePagesInRegion(beg, size);
  if (common_flags()->use_madv_dontdump)
    DontDumpShadowMemory(beg, size);
}

// --------------- LowLevelAllocateCallbac ---------- {{{1
static void OnLowLevelAllocate(uptr ptr, uptr size) {
  PoisonShadow(ptr, size, kAsanInternalHeapMagic);
}

// -------------------------- Run-time entry ------------------- {{{1
// exported functions
#define ASAN_REPORT_ERROR(type, is_write, size)                     \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                             \
void __asan_report_ ## type ## size(uptr addr) {                    \
  GET_CALLER_PC_BP_SP;                                              \
  ReportGenericError(pc, bp, sp, addr, is_write, size, 0, true);    \
}                                                                   \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                             \
void __asan_report_exp_ ## type ## size(uptr addr, u32 exp) {       \
  GET_CALLER_PC_BP_SP;                                              \
  ReportGenericError(pc, bp, sp, addr, is_write, size, exp, true);  \
}                                                                   \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                             \
void __asan_report_ ## type ## size ## _noabort(uptr addr) {        \
  GET_CALLER_PC_BP_SP;                                              \
  ReportGenericError(pc, bp, sp, addr, is_write, size, 0, false);   \
}                                                                   \

ASAN_REPORT_ERROR(load, false, 1)
ASAN_REPORT_ERROR(load, false, 2)
ASAN_REPORT_ERROR(load, false, 4)
ASAN_REPORT_ERROR(load, false, 8)
ASAN_REPORT_ERROR(load, false, 16)
ASAN_REPORT_ERROR(store, true, 1)
ASAN_REPORT_ERROR(store, true, 2)
ASAN_REPORT_ERROR(store, true, 4)
ASAN_REPORT_ERROR(store, true, 8)
ASAN_REPORT_ERROR(store, true, 16)

#define ASAN_REPORT_ERROR_N(type, is_write)                                 \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                                     \
void __asan_report_ ## type ## _n(uptr addr, uptr size) {                   \
  GET_CALLER_PC_BP_SP;                                                      \
  ReportGenericError(pc, bp, sp, addr, is_write, size, 0, true);            \
}                                                                           \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                                     \
void __asan_report_exp_ ## type ## _n(uptr addr, uptr size, u32 exp) {      \
  GET_CALLER_PC_BP_SP;                                                      \
  ReportGenericError(pc, bp, sp, addr, is_write, size, exp, true);          \
}                                                                           \
extern "C" NOINLINE INTERFACE_ATTRIBUTE                                     \
void __asan_report_ ## type ## _n_noabort(uptr addr, uptr size) {           \
  GET_CALLER_PC_BP_SP;                                                      \
  ReportGenericError(pc, bp, sp, addr, is_write, size, 0, false);           \
}                                                                           \

ASAN_REPORT_ERROR_N(load, false)
ASAN_REPORT_ERROR_N(store, true)

#define ASAN_MEMORY_ACCESS_CALLBACK_BODY(type, is_write, size, exp_arg, fatal) \
    uptr sp = MEM_TO_SHADOW(addr);                                             \
    uptr s = size <= SHADOW_GRANULARITY ? *reinterpret_cast<u8 *>(sp)          \
                                        : *reinterpret_cast<u16 *>(sp);        \
    if (UNLIKELY(s)) {                                                         \
      if (UNLIKELY(size >= SHADOW_GRANULARITY ||                               \
                   ((s8)((addr & (SHADOW_GRANULARITY - 1)) + size - 1)) >=     \
                       (s8)s)) {                                               \
        if (__asan_test_only_reported_buggy_pointer) {                         \
          *__asan_test_only_reported_buggy_pointer = addr;                     \
        } else {                                                               \
          GET_CALLER_PC_BP_SP;                                                 \
          ReportGenericError(pc, bp, sp, addr, is_write, size, exp_arg,        \
                              fatal);                                          \
        }                                                                      \
      }                                                                        \
    }

#define ASAN_MEMORY_ACCESS_CALLBACK(type, is_write, size)                      \
  extern "C" NOINLINE INTERFACE_ATTRIBUTE                                      \
  void __asan_##type##size(uptr addr) {                                        \
    ASAN_MEMORY_ACCESS_CALLBACK_BODY(type, is_write, size, 0, true)            \
  }                                                                            \
  extern "C" NOINLINE INTERFACE_ATTRIBUTE                                      \
  void __asan_exp_##type##size(uptr addr, u32 exp) {                           \
    ASAN_MEMORY_ACCESS_CALLBACK_BODY(type, is_write, size, exp, true)          \
  }                                                                            \
  extern "C" NOINLINE INTERFACE_ATTRIBUTE                                      \
  void __asan_##type##size ## _noabort(uptr addr) {                            \
    ASAN_MEMORY_ACCESS_CALLBACK_BODY(type, is_write, size, 0, false)           \
  }                                                                            \

ASAN_MEMORY_ACCESS_CALLBACK(load, false, 1)
ASAN_MEMORY_ACCESS_CALLBACK(load, false, 2)
ASAN_MEMORY_ACCESS_CALLBACK(load, false, 4)
ASAN_MEMORY_ACCESS_CALLBACK(load, false, 8)
ASAN_MEMORY_ACCESS_CALLBACK(load, false, 16)
ASAN_MEMORY_ACCESS_CALLBACK(store, true, 1)
ASAN_MEMORY_ACCESS_CALLBACK(store, true, 2)
ASAN_MEMORY_ACCESS_CALLBACK(store, true, 4)
ASAN_MEMORY_ACCESS_CALLBACK(store, true, 8)
ASAN_MEMORY_ACCESS_CALLBACK(store, true, 16)

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_loadN(uptr addr, uptr size) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, false, size, 0, true);
  }
}

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_exp_loadN(uptr addr, uptr size, u32 exp) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, false, size, exp, true);
  }
}

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_loadN_noabort(uptr addr, uptr size) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, false, size, 0, false);
  }
}

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_storeN(uptr addr, uptr size) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, true, size, 0, true);
  }
}

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_exp_storeN(uptr addr, uptr size, u32 exp) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, true, size, exp, true);
  }
}

extern "C"
NOINLINE INTERFACE_ATTRIBUTE
void __asan_storeN_noabort(uptr addr, uptr size) {
  if (__asan_region_is_poisoned(addr, size)) {
    GET_CALLER_PC_BP_SP;
    ReportGenericError(pc, bp, sp, addr, true, size, 0, false);
  }
}

// Force the linker to keep the symbols for various ASan interface functions.
// We want to keep those in the executable in order to let the instrumented
// dynamic libraries access the symbol even if it is not used by the executable
// itself. This should help if the build system is removing dead code at link
// time.
static NOINLINE void force_interface_symbols() {
  volatile int fake_condition = 0;  // prevent dead condition elimination.
  // __asan_report_* functions are noreturn, so we need a switch to prevent
  // the compiler from removing any of them.
  switch (fake_condition) {
    case 1: __asan_report_load1(0); break;
    case 2: __asan_report_load2(0); break;
    case 3: __asan_report_load4(0); break;
    case 4: __asan_report_load8(0); break;
    case 5: __asan_report_load16(0); break;
    case 6: __asan_report_load_n(0, 0); break;
    case 7: __asan_report_store1(0); break;
    case 8: __asan_report_store2(0); break;
    case 9: __asan_report_store4(0); break;
    case 10: __asan_report_store8(0); break;
    case 11: __asan_report_store16(0); break;
    case 12: __asan_report_store_n(0, 0); break;
    case 13: __asan_report_exp_load1(0, 0); break;
    case 14: __asan_report_exp_load2(0, 0); break;
    case 15: __asan_report_exp_load4(0, 0); break;
    case 16: __asan_report_exp_load8(0, 0); break;
    case 17: __asan_report_exp_load16(0, 0); break;
    case 18: __asan_report_exp_load_n(0, 0, 0); break;
    case 19: __asan_report_exp_store1(0, 0); break;
    case 20: __asan_report_exp_store2(0, 0); break;
    case 21: __asan_report_exp_store4(0, 0); break;
    case 22: __asan_report_exp_store8(0, 0); break;
    case 23: __asan_report_exp_store16(0, 0); break;
    case 24: __asan_report_exp_store_n(0, 0, 0); break;
    case 25: __asan_register_globals(nullptr, 0); break;
    case 26: __asan_unregister_globals(nullptr, 0); break;
    case 27: __asan_set_death_callback(nullptr); break;
    case 28: __asan_set_error_report_callback(nullptr); break;
    case 29: __asan_handle_no_return(); break;
    case 30: __asan_address_is_poisoned(nullptr); break;
    case 31: __asan_poison_memory_region(nullptr, 0); break;
    case 32: __asan_unpoison_memory_region(nullptr, 0); break;
    case 34: __asan_before_dynamic_init(nullptr); break;
    case 35: __asan_after_dynamic_init(); break;
    case 36: __asan_poison_stack_memory(0, 0); break;
    case 37: __asan_unpoison_stack_memory(0, 0); break;
    case 38: __asan_region_is_poisoned(0, 0); break;
    case 39: __asan_describe_address(0); break;
  }
}

static void asan_atexit() {
  Printf("AddressSanitizer exit stats:\n");
  __asan_print_accumulated_stats();
  // Print AsanMappingProfile.
  for (uptr i = 0; i < kAsanMappingProfileSize; i++) {
    if (AsanMappingProfile[i] == 0) continue;
    Printf("asan_mapping.h:%zd -- %zd\n", i, AsanMappingProfile[i]);
  }
}

static void InitializeHighMemEnd() {
#if !ASAN_FIXED_MAPPING
  kHighMemEnd = GetMaxVirtualAddress();
  // Increase kHighMemEnd to make sure it's properly
  // aligned together with kHighMemBeg:
  kHighMemEnd |= SHADOW_GRANULARITY * GetPageSizeCached() - 1;
#endif  // !ASAN_FIXED_MAPPING
  CHECK_EQ((kHighMemBeg % GetPageSizeCached()), 0);
}

static void ProtectGap(uptr addr, uptr size) {
  if (!flags()->protect_shadow_gap)
    return;
  void *res = MmapNoAccess(addr, size, "shadow gap");
  if (addr == (uptr)res)
    return;
  // A few pages at the start of the address space can not be protected.
  // But we really want to protect as much as possible, to prevent this memory
  // being returned as a result of a non-FIXED mmap().
  if (addr == kZeroBaseShadowStart) {
    uptr step = GetPageSizeCached();
    while (size > step && addr < kZeroBaseMaxShadowStart) {
      addr += step;
      size -= step;
      void *res = MmapNoAccess(addr, size, "shadow gap");
      if (addr == (uptr)res)
        return;
    }
  }

  Report("ERROR: Failed to protect the shadow gap. "
         "ASan cannot proceed correctly. ABORTING.\n");
  DumpProcessMap();
  Die();
}

static void PrintAddressSpaceLayout() {
  Printf("|| `[%p, %p]` || HighMem    ||\n",
         (void*)kHighMemBeg, (void*)kHighMemEnd);
  Printf("|| `[%p, %p]` || HighShadow ||\n",
         (void*)kHighShadowBeg, (void*)kHighShadowEnd);
  if (kMidMemBeg) {
    Printf("|| `[%p, %p]` || ShadowGap3 ||\n",
           (void*)kShadowGap3Beg, (void*)kShadowGap3End);
    Printf("|| `[%p, %p]` || MidMem     ||\n",
           (void*)kMidMemBeg, (void*)kMidMemEnd);
    Printf("|| `[%p, %p]` || ShadowGap2 ||\n",
           (void*)kShadowGap2Beg, (void*)kShadowGap2End);
    Printf("|| `[%p, %p]` || MidShadow  ||\n",
           (void*)kMidShadowBeg, (void*)kMidShadowEnd);
  }
  Printf("|| `[%p, %p]` || ShadowGap  ||\n",
         (void*)kShadowGapBeg, (void*)kShadowGapEnd);
  if (kLowShadowBeg) {
    Printf("|| `[%p, %p]` || LowShadow  ||\n",
           (void*)kLowShadowBeg, (void*)kLowShadowEnd);
    Printf("|| `[%p, %p]` || LowMem     ||\n",
           (void*)kLowMemBeg, (void*)kLowMemEnd);
  }
  Printf("MemToShadow(shadow): %p %p %p %p",
         (void*)MEM_TO_SHADOW(kLowShadowBeg),
         (void*)MEM_TO_SHADOW(kLowShadowEnd),
         (void*)MEM_TO_SHADOW(kHighShadowBeg),
         (void*)MEM_TO_SHADOW(kHighShadowEnd));
  if (kMidMemBeg) {
    Printf(" %p %p",
           (void*)MEM_TO_SHADOW(kMidShadowBeg),
           (void*)MEM_TO_SHADOW(kMidShadowEnd));
  }
  Printf("\n");
  Printf("redzone=%zu\n", (uptr)flags()->redzone);
  Printf("max_redzone=%zu\n", (uptr)flags()->max_redzone);
  Printf("quarantine_size_mb=%zuM\n", (uptr)flags()->quarantine_size_mb);
  Printf("malloc_context_size=%zu\n",
         (uptr)common_flags()->malloc_context_size);

  Printf("SHADOW_SCALE: %d\n", (int)SHADOW_SCALE);
  Printf("SHADOW_GRANULARITY: %d\n", (int)SHADOW_GRANULARITY);
  Printf("SHADOW_OFFSET: 0x%zx\n", (uptr)SHADOW_OFFSET);
  CHECK(SHADOW_SCALE >= 3 && SHADOW_SCALE <= 7);
  if (kMidMemBeg)
    CHECK(kMidShadowBeg > kLowShadowEnd &&
          kMidMemBeg > kMidShadowEnd &&
          kHighShadowBeg > kMidMemEnd);
}

static void AsanInitInternal() {
  if (LIKELY(asan_inited)) return;
  SanitizerToolName = "AddressSanitizer";
  CHECK(!asan_init_is_running && "ASan init calls itself!");
  asan_init_is_running = true;

  CacheBinaryName();

  // Initialize flags. This must be done early, because most of the
  // initialization steps look at flags().
  InitializeFlags();

  AsanCheckIncompatibleRT();
  AsanCheckDynamicRTPrereqs();

  SetCanPoisonMemory(flags()->poison_heap);
  SetMallocContextSize(common_flags()->malloc_context_size);

  InitializeHighMemEnd();

  // Make sure we are not statically linked.
  AsanDoesNotSupportStaticLinkage();

  // Install tool-specific callbacks in sanitizer_common.
  AddDieCallback(AsanDie);
  SetCheckFailedCallback(AsanCheckFailed);
  SetPrintfAndReportCallback(AppendToErrorMessageBuffer);

  __sanitizer_set_report_path(common_flags()->log_path);

  // Enable UAR detection, if required.
  __asan_option_detect_stack_use_after_return =
      flags()->detect_stack_use_after_return;

  // Re-exec ourselves if we need to set additional env or command line args.
  MaybeReexec();

  // Setup internal allocator callback.
  SetLowLevelAllocateCallback(OnLowLevelAllocate);

  InitializeAsanInterceptors();

  // Enable system log ("adb logcat") on Android.
  // Doing this before interceptors are initialized crashes in:
  // AsanInitInternal -> android_log_write -> __interceptor_strcmp
  AndroidLogInit();

  ReplaceSystemMalloc();

  uptr shadow_start = kLowShadowBeg;
  if (kLowShadowBeg)
    shadow_start -= GetMmapGranularity();
  bool full_shadow_is_available =
      MemoryRangeIsAvailable(shadow_start, kHighShadowEnd);

#if SANITIZER_LINUX && defined(__x86_64__) && defined(_LP64) &&                \
    !ASAN_FIXED_MAPPING
  if (!full_shadow_is_available) {
    kMidMemBeg = kLowMemEnd < 0x3000000000ULL ? 0x3000000000ULL : 0;
    kMidMemEnd = kLowMemEnd < 0x3000000000ULL ? 0x4fffffffffULL : 0;
  }
#endif

  if (Verbosity()) PrintAddressSpaceLayout();

  DisableCoreDumperIfNecessary();

  if (full_shadow_is_available) {
    // mmap the low shadow plus at least one page at the left.
    if (kLowShadowBeg)
      ReserveShadowMemoryRange(shadow_start, kLowShadowEnd, "low shadow");
    // mmap the high shadow.
    ReserveShadowMemoryRange(kHighShadowBeg, kHighShadowEnd, "high shadow");
    // protect the gap.
    ProtectGap(kShadowGapBeg, kShadowGapEnd - kShadowGapBeg + 1);
    CHECK_EQ(kShadowGapEnd, kHighShadowBeg - 1);
  } else if (kMidMemBeg &&
      MemoryRangeIsAvailable(shadow_start, kMidMemBeg - 1) &&
      MemoryRangeIsAvailable(kMidMemEnd + 1, kHighShadowEnd)) {
    CHECK(kLowShadowBeg != kLowShadowEnd);
    // mmap the low shadow plus at least one page at the left.
    ReserveShadowMemoryRange(shadow_start, kLowShadowEnd, "low shadow");
    // mmap the mid shadow.
    ReserveShadowMemoryRange(kMidShadowBeg, kMidShadowEnd, "mid shadow");
    // mmap the high shadow.
    ReserveShadowMemoryRange(kHighShadowBeg, kHighShadowEnd, "high shadow");
    // protect the gaps.
    ProtectGap(kShadowGapBeg, kShadowGapEnd - kShadowGapBeg + 1);
    ProtectGap(kShadowGap2Beg, kShadowGap2End - kShadowGap2Beg + 1);
    ProtectGap(kShadowGap3Beg, kShadowGap3End - kShadowGap3Beg + 1);
  } else {
    Report("Shadow memory range interleaves with an existing memory mapping. "
           "ASan cannot proceed correctly. ABORTING.\n");
    Report("ASan shadow was supposed to be located in the [%p-%p] range.\n",
           shadow_start, kHighShadowEnd);
    DumpProcessMap();
    Die();
  }

  AsanTSDInit(PlatformTSDDtor);
  InstallDeadlySignalHandlers(AsanOnDeadlySignal);

  AllocatorOptions allocator_options;
  allocator_options.SetFrom(flags(), common_flags());
  InitializeAllocator(allocator_options);

  MaybeStartBackgroudThread();
  SetSoftRssLimitExceededCallback(AsanSoftRssLimitExceededCallback);

  // On Linux AsanThread::ThreadStart() calls malloc() that's why asan_inited
  // should be set to 1 prior to initializing the threads.
  asan_inited = 1;
  asan_init_is_running = false;

  if (flags()->atexit)
    Atexit(asan_atexit);

  InitializeCoverage(common_flags()->coverage, common_flags()->coverage_dir);

  // Now that ASan runtime is (mostly) initialized, deactivate it if
  // necessary, so that it can be re-activated when requested.
  if (flags()->start_deactivated)
    AsanDeactivate();

  // interceptors
  InitTlsSize();

  // Create main thread.
  AsanThread *main_thread = AsanThread::Create(
      /* start_routine */ nullptr, /* arg */ nullptr, /* parent_tid */ 0,
      /* stack */ nullptr, /* detached */ true);
  CHECK_EQ(0, main_thread->tid());
  SetCurrentThread(main_thread);
  main_thread->ThreadStart(internal_getpid(),
                           /* signal_thread_is_registered */ nullptr);
  force_interface_symbols();  // no-op.
  SanitizerInitializeUnwinder();

#if CAN_SANITIZE_LEAKS
  __lsan::InitCommonLsan();
  if (common_flags()->detect_leaks && common_flags()->leak_check_at_exit) {
    Atexit(__lsan::DoLeakCheck);
  }
#endif  // CAN_SANITIZE_LEAKS

#if CAN_SANITIZE_UB
  __ubsan::InitAsPlugin();
#endif

  InitializeSuppressions();

  VReport(1, "AddressSanitizer Init done\n");
}

// Initialize as requested from some part of ASan runtime library (interceptors,
// allocator, etc).
void AsanInitFromRtl() {
  AsanInitInternal();
}

#if ASAN_DYNAMIC
// Initialize runtime in case it's LD_PRELOAD-ed into unsanitized executable
// (and thus normal initializers from .preinit_array or modules haven't run).

class AsanInitializer {
public:  // NOLINT
  AsanInitializer() {
    AsanInitFromRtl();
  }
};

static AsanInitializer asan_initializer;
#endif  // ASAN_DYNAMIC

} // namespace __asan

// ---------------------- Interface ---------------- {{{1
using namespace __asan;  // NOLINT

void NOINLINE __asan_handle_no_return() {
  int local_stack;
  AsanThread *curr_thread = GetCurrentThread();
  uptr PageSize = GetPageSizeCached();
  uptr top, bottom;
  if (curr_thread) {
    top = curr_thread->stack_top();
    bottom = ((uptr)&local_stack - PageSize) & ~(PageSize - 1);
  } else {
    // If we haven't seen this thread, try asking the OS for stack bounds.
    uptr tls_addr, tls_size, stack_size;
    GetThreadStackAndTls(/*main=*/false, &bottom, &stack_size, &tls_addr,
                         &tls_size);
    top = bottom + stack_size;
  }
  static const uptr kMaxExpectedCleanupSize = 64 << 20;  // 64M
  if (top - bottom > kMaxExpectedCleanupSize) {
    static bool reported_warning = false;
    if (reported_warning)
      return;
    reported_warning = true;
    Report("WARNING: ASan is ignoring requested __asan_handle_no_return: "
           "stack top: %p; bottom %p; size: %p (%zd)\n"
           "False positive error reports may follow\n"
           "For details see "
           "https://github.com/google/sanitizers/issues/189\n",
           top, bottom, top - bottom, top - bottom);
    return;
  }
  PoisonShadow(bottom, top - bottom, 0);
  if (curr_thread && curr_thread->has_fake_stack())
    curr_thread->fake_stack()->HandleNoReturn();
}

void NOINLINE __asan_set_death_callback(void (*callback)(void)) {
  SetUserDieCallback(callback);
}

// Initialize as requested from instrumented application code.
// We use this call as a trigger to wake up ASan from deactivated state.
void __asan_init() {
  AsanActivate();
  AsanInitInternal();
}

void __asan_version_mismatch_check() {
  // Do nothing.
}
