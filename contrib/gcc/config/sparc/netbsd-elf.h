/* Definitions of target machine for GNU compiler, for ELF on NetBSD/sparc
   and NetBSD/sparc64.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Contributed by Matthew Green (mrg@eterna.com.au).

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Make sure these are undefined.  */
#undef MD_EXEC_PREFIX
#undef MD_STARTFILE_PREFIX

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-D__sparc__ -D__NetBSD__ -D__ELF__ \
-Asystem=unix -Asystem=NetBSD"

/* CPP defines used for 64 bit code.  */
#undef CPP_SUBTARGET_SPEC64
#define CPP_SUBTARGET_SPEC64 \
  "-D__sparc64__ -D__arch64__ -D__sparc_v9__ %{posix:-D_POSIX_SOURCE}"

/* CPP defines used for 32 bit code.  */
#undef CPP_SUBTARGET_SPEC32
#define CPP_SUBTARGET_SPEC32 "-D__sparc %{posix:-D_POSIX_SOURCE}"

/* SIZE_TYPE and PTRDIFF_TYPE are wrong from sparc/sparc.h.  */
#undef SIZE_TYPE
#define SIZE_TYPE "long unsigned int"

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE "long int"

#undef PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG

/* This is the char to use for continuation (in case we need to turn
   continuation back on).  */
#undef DBX_CONTIN_CHAR
#define DBX_CONTIN_CHAR '?'

#undef DBX_REGISTER_NUMBER
#define DBX_REGISTER_NUMBER(REGNO) \
  (TARGET_FLAT && REGNO == HARD_FRAME_POINTER_REGNUM ? 31 : REGNO)

#undef  LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX  "."

/* This is how to output a definition of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.  */

#undef  ASM_OUTPUT_INTERNAL_LABEL
#define ASM_OUTPUT_INTERNAL_LABEL(FILE,PREFIX,NUM)	\
  fprintf (FILE, ".L%s%d:\n", PREFIX, NUM)

/* This is how to output a reference to an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.  */

#undef  ASM_OUTPUT_INTERNAL_LABELREF
#define ASM_OUTPUT_INTERNAL_LABELREF(FILE,PREFIX,NUM)	\
  fprintf (FILE, ".L%s%d", PREFIX, NUM)

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#undef  ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)	\
  sprintf ((LABEL), "*.L%s%ld", (PREFIX), (long)(NUM))

#undef USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""

#undef ASM_SPEC
#define ASM_SPEC "%{fpic:-K PIC} %{fPIC:-K PIC} %{V} %{v:%{!V:-V}} \
%{mlittle-endian:-EL} \
%(asm_cpu) %(asm_arch) %(asm_relax)"

#undef STDC_0_IN_SYSTEM_HEADERS

#undef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (%s)", TARGET_NAME);

/*
 * Clean up afterwards generic SPARC ELF configuration.
 */

#undef TRANSFER_FROM_TRAMPOLINE
#define TRANSFER_FROM_TRAMPOLINE

/* FIXME: Aren't these supposed to be available for SPARC ELF?  */
#undef MULDI3_LIBCALL
#undef DIVDI3_LIBCALL
#undef UDIVDI3_LIBCALL
#undef MODDI3_LIBCALL
#undef UMODDI3_LIBCALL
#undef INIT_SUBTARGET_OPTABS  
#define INIT_SUBTARGET_OPTABS  

/* Below here exists the merged NetBSD/sparc & NetBSD/sparc64 compiler
   description, allowing one to build 32 bit or 64 bit applications
   on either.  We define the sparc & sparc64 versions of things,
   occasionally a neutral version (should be the same as "netbsd-elf.h")
   and then based on SPARC_BI_ARCH, DEFAULT_ARCH32_P, and TARGET_CPU_DEFAULT,
   we choose the correct version.  */

/* We use the default NetBSD ELF STARTFILE_SPEC and ENDFILE_SPEC
   definitions, even for the SPARC_BI_ARCH compiler, because NetBSD does
   not have a default place to find these libraries..  */

/* Name the port(s).  */
#define TARGET_NAME64     "NetBSD/sparc64 ELF"
#define TARGET_NAME32     "NetBSD/sparc ELF"

/* TARGET_CPU_DEFAULT is set in Makefile.in.  We test for 64-bit default
   platform here.  */

#if TARGET_CPU_DEFAULT == TARGET_CPU_v9 \
 || TARGET_CPU_DEFAULT == TARGET_CPU_ultrasparc
/* A 64 bit v9 compiler with stack-bias,
   in a Medium/Low code model environment.  */

#undef TARGET_DEFAULT
#define TARGET_DEFAULT \
  (MASK_V9 + MASK_PTR64 + MASK_64BIT /* + MASK_HARD_QUAD */ \
   + MASK_STACK_BIAS + MASK_APP_REGS + MASK_FPU + MASK_LONG_DOUBLE_128)

#undef SPARC_DEFAULT_CMODEL
#define SPARC_DEFAULT_CMODEL CM_MEDANY

#endif

/* CC1_SPEC for NetBSD/sparc.  */
#define CC1_SPEC32 \
 "%{sun4:} %{target:} \
  %{mcypress:-mcpu=cypress} \
  %{msparclite:-mcpu=sparclite} %{mf930:-mcpu=f930} %{mf934:-mcpu=f934} \
  %{mv8:-mcpu=v8} %{msupersparc:-mcpu=supersparc} \
  %{m32:%{m64:%emay not use both -m32 and -m64}} \
  %{m64: \
    -mptr64 -mstack-bias -mno-v8plus -mlong-double-128 \
    %{!mcpu*: \
      %{!mcypress: \
        %{!msparclite: \
	  %{!mf930: \
	    %{!mf934: \
	      %{!mv8*: \
	        %{!msupersparc:-mcpu=ultrasparc}}}}}}} \
    %{!mno-vis:%{!mcpu=v9:-mvis}} \
    %{p:-mcmodel=medlow} \
    %{pg:-mcmodel=medlow}}"

#define CC1_SPEC64 \
 "%{sun4:} %{target:} \
  %{mcypress:-mcpu=cypress} \
  %{msparclite:-mcpu=sparclite} %{mf930:-mcpu=f930} %{mf934:-mcpu=f934} \
  %{mv8:-mcpu=v8} %{msupersparc:-mcpu=supersparc} \
  %{m32:%{m64:%emay not use both -m32 and -m64}} \
  %{m32: \
    -mptr32 -mno-stack-bias \
    %{!mlong-double-128:-mlong-double-64} \
    %{!mcpu*: \
      %{!mcypress: \
	%{!msparclite: \
	  %{!mf930: \
	    %{!mf934: \
	      %{!mv8*: \
		%{!msupersparc:-mcpu=cypress}}}}}}}} \
  %{!m32: \
    %{p:-mcmodel=medlow} \
    %{pg:-mcmodel=medlow}}"

/* Make sure we use the right output format.  Pick a default and then
   make sure -m32/-m64 switch to the right one.  */

#define LINK_ARCH32_SPEC \
 "%-m elf32_sparc \
  %{assert*} %{R*} %{V} %{v:%{!V:-V}} \
  %{shared:-shared} \
  %{!shared: \
    -dp \
    %{!nostdlib:%{!r*:%{!e*:-e __start}}} \
    %{!static: \
      -dy %{rdynamic:-export-dynamic} \
      %{!dynamic-linker:-dynamic-linker /usr/libexec/ld.elf_so}} \
    %{static:-static}}"

#define LINK_ARCH64_SPEC \
 "%-m elf64_sparc \
  %{assert*} %{R*} %{V} %{v:%{!V:-V}} \
  %{shared:-shared} \
  %{!shared: \
    -dp \
    %{!nostdlib:%{!r*:%{!e*:-e __start}}} \
    %{!static: \
      -dy %{rdynamic:-export-dynamic} \
      %{!dynamic-linker:-dynamic-linker /usr/libexec/ld.elf_so}} \
    %{static:-static}}"

#define LINK_ARCH_SPEC "\
%{m32:%(link_arch32)} \
%{m64:%(link_arch64)} \
%{!m32:%{!m64:%(link_arch_default)}} \
"

#if DEFAULT_ARCH32_P
#define LINK_ARCH_DEFAULT_SPEC LINK_ARCH32_SPEC
#else
#define LINK_ARCH_DEFAULT_SPEC LINK_ARCH64_SPEC
#endif

/* What extra spec entries do we need?  */
#undef SUBTARGET_EXTRA_SPECS
#define SUBTARGET_EXTRA_SPECS \
  { "link_arch32",		LINK_ARCH32_SPEC }, \
  { "link_arch64",		LINK_ARCH64_SPEC }, \
  { "link_arch_default",	LINK_ARCH_DEFAULT_SPEC }, \
  { "link_arch",		LINK_ARCH_SPEC }, \
  { "cpp_subtarget_spec32",	CPP_SUBTARGET_SPEC32 }, \
  { "cpp_subtarget_spec64",	CPP_SUBTARGET_SPEC64 },


/* What extra switches do we need?  */
#undef  SUBTARGET_SWITCHES
#define SUBTARGET_SWITCHES \
  {"long-double-64", -MASK_LONG_DOUBLE_128, N_("Use 64 bit long doubles") }, \
  {"long-double-128", MASK_LONG_DOUBLE_128, N_("Use 128 bit long doubles") },


/* Build a compiler that supports -m32 and -m64?  */

#ifdef SPARC_BI_ARCH

#undef LONG_DOUBLE_TYPE_SIZE
#define LONG_DOUBLE_TYPE_SIZE (TARGET_LONG_DOUBLE_128 ? 128 : 64)

#undef MAX_LONG_DOUBLE_TYPE_SIZE
#define MAX_LONG_DOUBLE_TYPE_SIZE 128

#if defined(__arch64__) || defined(__LONG_DOUBLE_128__)
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 128
#else
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 64
#endif

#undef  CC1_SPEC
#if DEFAULT_ARCH32_P
#define CC1_SPEC CC1_SPEC32
#else
#define CC1_SPEC CC1_SPEC64
#endif

#if DEFAULT_ARCH32_P
#define MULTILIB_DEFAULTS { "m32" }
#else
#define MULTILIB_DEFAULTS { "m64" }
#endif

#undef CPP_SUBTARGET_SPEC
#if DEFAULT_ARCH32_P
#define CPP_SUBTARGET_SPEC \
  "%{m64:%(cpp_subtarget_spec64)}%{!m64:%(cpp_subtarget_spec32)}"
#else
#define CPP_SUBTARGET_SPEC \
  "%{!m32:%(cpp_subtarget_spec64)}%{m32:%(cpp_subtarget_spec32)}"
#endif

/* Restore this from sparc/sparc.h, netbsd.h changes it.  */
#undef CPP_SPEC
#define CPP_SPEC "%(cpp_cpu) %(cpp_arch) %(cpp_endian) %(cpp_subtarget)"

/* Name the port. */
#undef TARGET_NAME
#define TARGET_NAME     (DEFAULT_ARCH32_P ? TARGET_NAME32 : TARGET_NAME64)

#else	/* SPARC_BI_ARCH */

#if TARGET_CPU_DEFAULT == TARGET_CPU_v9 \
 || TARGET_CPU_DEFAULT == TARGET_CPU_ultrasparc

#undef LONG_DOUBLE_TYPE_SIZE
#define LONG_DOUBLE_TYPE_SIZE 128

#undef MAX_LONG_DOUBLE_TYPE_SIZE
#define MAX_LONG_DOUBLE_TYPE_SIZE 128

#undef LIBGCC2_LONG_DOUBLE_TYPE_SIZE
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 128

#undef  CC1_SPEC
#define CC1_SPEC CC1_SPEC64

#undef CPP_SUBTARGET_SPEC
#define CPP_SUBTARGET_SPEC CPP_SUBTARGET_SPEC64

#undef TARGET_NAME
#define TARGET_NAME     TARGET_NAME64

#else	/* TARGET_CPU_DEFAULT == TARGET_CPU_v9 \
	|| TARGET_CPU_DEFAULT == TARGET_CPU_ultrasparc */

/* A 32-bit only compiler.  NetBSD don't support 128 bit `long double'
   for 32-bit code, unlike Solaris.  */

#undef LONG_DOUBLE_TYPE_SIZE
#define LONG_DOUBLE_TYPE_SIZE 64

#undef MAX_LONG_DOUBLE_TYPE_SIZE
#define MAX_LONG_DOUBLE_TYPE_SIZE 64

#undef LIBGCC2_LONG_DOUBLE_TYPE_SIZE
#define LIBGCC2_LONG_DOUBLE_TYPE_SIZE 64

#undef CPP_SUBTARGET_SPEC
#define CPP_SUBTARGET_SPEC CPP_SUBTARGET_SPEC32

#undef  CC1_SPEC
#define CC1_SPEC CC1_SPEC32

#undef TARGET_NAME
#define TARGET_NAME     TARGET_NAME32

#endif	/* TARGET_CPU_DEFAULT == TARGET_CPU_v9 \
	|| TARGET_CPU_DEFAULT == TARGET_CPU_ultrasparc */

#endif	/* SPARC_BI_ARCH */

/* We use GNU ld so undefine this so that attribute((init_priority)) works.  */
#undef CTORS_SECTION_ASM_OP
#undef DTORS_SECTION_ASM_OP
