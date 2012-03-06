/*	$NetBSD: mips_reloc.c,v 1.58 2010/01/14 11:57:06 skrll Exp $	*/

/*
 * Copyright 1997 Michael L. Hitch <mhitch@montana.edu>
 * Portions copyright 2002 Charles M. Hannum <root@ihack.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/endian.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <machine/sysarch.h>
#include <machine/tls.h>

#include "debug.h"
#include "rtld.h"

#ifdef __mips_n64
#define	GOT1_MASK	0x8000000000000000UL
#else
#define	GOT1_MASK	0x80000000UL
#endif

void
init_pltgot(Obj_Entry *obj)
{
	if (obj->pltgot != NULL) {
		obj->pltgot[0] = (Elf_Addr) &_rtld_bind_start;
		if (obj->pltgot[1] & 0x80000000)
			obj->pltgot[1] = (Elf_Addr) obj | GOT1_MASK;
	}
}

int
do_copy_relocations(Obj_Entry *dstobj)
{
	/* Do nothing */
	return 0;
}

void _rtld_relocate_nonplt_self(Elf_Dyn *, Elf_Addr);

/*
 * It is possible for the compiler to emit relocations for unaligned data.
 * We handle this situation with these inlines.
 */
#ifdef __mips_n64
/*
 * ELF64 MIPS encodes the relocs uniquely.  The first 32-bits of info contain
 * the symbol index.  The top 32-bits contain three relocation types encoded
 * in big-endian integer with first relocation in LSB.  This means for little
 * endian we have to byte swap that integer (r_type).
 */
#define	Elf_Sxword			Elf64_Sxword
#define	ELF_R_NXTTYPE_64_P(r_type)	((((r_type) >> 8) & 0xff) == R_TYPE(64))
#if BYTE_ORDER == LITTLE_ENDIAN
#undef ELF_R_SYM
#undef ELF_R_TYPE
#define ELF_R_SYM(r_info)		((r_info) & 0xffffffff)
#define ELF_R_TYPE(r_info)		bswap32((r_info) >> 32)
#endif
#else
#define	ELF_R_NXTTYPE_64_P(r_type)	(0)
#define	Elf_Sxword			Elf32_Sword
#endif

static __inline Elf_Sxword
load_ptr(void *where, size_t len)
{
	Elf_Sxword val;

	if (__predict_true(((uintptr_t)where & (len - 1)) == 0)) {
#ifdef __mips_n64
		if (len == sizeof(Elf_Sxword))
			return *(Elf_Sxword *)where;
#endif
		return *(Elf_Sword *)where;
	}

	val = 0;
#if BYTE_ORDER == LITTLE_ENDIAN
	(void)memcpy(&val, where, len);
#endif
#if BYTE_ORDER == BIG_ENDIAN
	(void)memcpy((uint8_t *)((&val)+1) - len, where, len);
#endif
	return (len == sizeof(Elf_Sxword)) ? val : (Elf_Sword)val;
}

static __inline void
store_ptr(void *where, Elf_Sxword val, size_t len)
{
	if (__predict_true(((uintptr_t)where & (len - 1)) == 0)) {
#ifdef __mips_n64
		if (len == sizeof(Elf_Sxword)) {
			*(Elf_Sxword *)where = val;
			return;
		}
#endif
		*(Elf_Sword *)where = val;
		return;
	}
#if BYTE_ORDER == LITTLE_ENDIAN
	(void)memcpy(where, &val, len);
#endif
#if BYTE_ORDER == BIG_ENDIAN
	(void)memcpy(where, (const uint8_t *)((&val)+1) - len, len);
#endif
}

void
_rtld_relocate_nonplt_self(Elf_Dyn *dynp, Elf_Addr relocbase)
{
	const Elf_Rel *rel = 0, *rellim;
	Elf_Addr relsz = 0;
	const Elf_Sym *symtab = NULL, *sym;
	Elf_Addr *where;
	Elf_Addr *got = NULL;
	Elf_Word local_gotno = 0, symtabno = 0, gotsym = 0;
	size_t i;

	for (; dynp->d_tag != DT_NULL; dynp++) {
		switch (dynp->d_tag) {
		case DT_REL:
			rel = (const Elf_Rel *)(relocbase + dynp->d_un.d_ptr);
			break;
		case DT_RELSZ:
			relsz = dynp->d_un.d_val;
			break;
		case DT_SYMTAB:
			symtab = (const Elf_Sym *)(relocbase + dynp->d_un.d_ptr);
			break;
		case DT_PLTGOT:
			got = (Elf_Addr *)(relocbase + dynp->d_un.d_ptr);
			break;
		case DT_MIPS_LOCAL_GOTNO:
			local_gotno = dynp->d_un.d_val;
			break;
		case DT_MIPS_SYMTABNO:
			symtabno = dynp->d_un.d_val;
			break;
		case DT_MIPS_GOTSYM:
			gotsym = dynp->d_un.d_val;
			break;
		}
	}

	i = (got[1] & GOT1_MASK) ? 2 : 1;
	/* Relocate the local GOT entries */
	got += i;
	for (; i < local_gotno; i++) {
		*got++ += relocbase;
	}

	sym = symtab + gotsym;
	/* Now do the global GOT entries */
	for (i = gotsym; i < symtabno; i++) {
		*got = sym->st_value + relocbase;
		++sym;
		++got;
	}

	rellim = (const Elf_Rel *)((caddr_t)rel + relsz);
	for (; rel < rellim; rel++) {
		Elf_Word r_symndx, r_type;

		where = (void *)(relocbase + rel->r_offset);

		r_symndx = ELF_R_SYM(rel->r_info);
		r_type = ELF_R_TYPE(rel->r_info);

		switch (r_type & 0xff) {
		case R_TYPE(REL32): {
			const size_t rlen =
			    ELF_R_NXTTYPE_64_P(r_type)
				? sizeof(Elf_Sxword)
				: sizeof(Elf_Sword);
			Elf_Sxword old = load_ptr(where, rlen);
			Elf_Sxword val = old;
#ifdef __mips_n64
			assert(r_type == R_TYPE(REL32)
			    || r_type == (R_TYPE(REL32)|(R_TYPE(64) << 8)));
#endif
			assert(r_symndx < gotsym);
			sym = symtab + r_symndx;
			assert(ELF_ST_BIND(sym->st_info) == STB_LOCAL);
			val += relocbase;
			store_ptr(where, val, sizeof(Elf_Sword));
			dbg("REL32/L(%p) %p -> %p in <self>",
			    where, (void *)old, (void *)val);
			store_ptr(where, val, rlen);
			break;
		}

		case R_TYPE(GPREL32):
		case R_TYPE(NONE):
			break;


		default:
			abort();
			break;
		}
	}
}

Elf_Addr
_mips_rtld_bind(Obj_Entry *obj, Elf_Size reloff)
{
        Elf_Addr *got = obj->pltgot;
        const Elf_Sym *def;
        const Obj_Entry *defobj;
        Elf_Addr target;

        def = find_symdef(reloff, obj, &defobj, SYMLOOK_IN_PLT, NULL,
	    NULL);
        if (def == NULL)
		_rtld_error("bind failed no symbol");

        target = (Elf_Addr)(defobj->relocbase + def->st_value);
        dbg("bind now/fixup at %s sym # %jd in %s --> was=%p new=%p",
	    obj->path,
	    (intmax_t)reloff, defobj->strtab + def->st_name, 
	    (void *)got[obj->local_gotno + reloff - obj->gotsym],
	    (void *)target);
        got[obj->local_gotno + reloff - obj->gotsym] = target;
	return (Elf_Addr)target;
}

int
reloc_non_plt(Obj_Entry *obj, Obj_Entry *obj_rtld, RtldLockState *lockstate)
{
	const Elf_Rel *rel;
	const Elf_Rel *rellim;
	Elf_Addr *got = obj->pltgot;
	const Elf_Sym *sym, *def;
	const Obj_Entry *defobj;
	Elf_Word i;
#ifdef SUPPORT_OLD_BROKEN_LD
	int broken;
#endif

	/* The relocation for the dynamic loader has already been done. */
	if (obj == obj_rtld)
		return (0);

#ifdef SUPPORT_OLD_BROKEN_LD
	broken = 0;
	sym = obj->symtab;
	for (i = 1; i < 12; i++)
		if (sym[i].st_info == ELF_ST_INFO(STB_LOCAL, STT_NOTYPE))
			broken = 1;
	dbg("%s: broken=%d", obj->path, broken);
#endif

	i = (got[1] & GOT1_MASK) ? 2 : 1;

	/* Relocate the local GOT entries */
	got += i;
	dbg("got:%p for %d entries adding %p",
	    got, obj->local_gotno, obj->relocbase);
	for (; i < obj->local_gotno; i++) {
		*got += (Elf_Addr)obj->relocbase;
		got++;
	}
	sym = obj->symtab + obj->gotsym;

	dbg("got:%p for %d entries",
	    got, obj->symtabno);
	/* Now do the global GOT entries */
	for (i = obj->gotsym; i < obj->symtabno; i++) {
		dbg(" doing got %d sym %p (%s, %lx)", i - obj->gotsym, sym,
		    sym->st_name + obj->strtab, (u_long) *got);

#ifdef SUPPORT_OLD_BROKEN_LD
		if (ELF_ST_TYPE(sym->st_info) == STT_FUNC &&
		    broken && sym->st_shndx == SHN_UNDEF) {
			/*
			 * XXX DANGER WILL ROBINSON!
			 * You might think this is stupid, as it intentionally
			 * defeats lazy binding -- and you'd be right.
			 * Unfortunately, for lazy binding to work right, we
			 * need to a way to force the GOT slots used for
			 * function pointers to be resolved immediately.  This
			 * is supposed to be done automatically by the linker,
			 * by not outputting a PLT slot and setting st_value
			 * to 0 if there are non-PLT references, but older
			 * versions of GNU ld do not do this.
			 */
			def = find_symdef(i, obj, &defobj, false, NULL,
			    lockstate);
			if (def == NULL)
				return -1;
			*got = def->st_value + (Elf_Addr)defobj->relocbase;
		} else
#endif
		if (ELF_ST_TYPE(sym->st_info) == STT_FUNC &&
		    sym->st_value != 0 && sym->st_shndx == SHN_UNDEF) {
			/*
			 * If there are non-PLT references to the function,
			 * st_value should be 0, forcing us to resolve the
			 * address immediately.
			 *
			 * XXX DANGER WILL ROBINSON!
			 * The linker is not outputting PLT slots for calls to
			 * functions that are defined in the same shared
			 * library.  This is a bug, because it can screw up
			 * link ordering rules if the symbol is defined in
			 * more than one module.  For now, if there is a
			 * definition, we fail the test above and force a full
			 * symbol lookup.  This means that all intra-module
			 * calls are bound immediately.  - mycroft, 2003/09/24
			 */
			*got = sym->st_value + (Elf_Addr)obj->relocbase;
			if ((Elf_Addr)(*got) == (Elf_Addr)obj->relocbase) {
				dbg("Warning2, i:%d maps to relocbase address:%p",
				    i, obj->relocbase);
			}

		} else if (sym->st_info == ELF_ST_INFO(STB_GLOBAL, STT_SECTION)) {
			/* Symbols with index SHN_ABS are not relocated. */
			if (sym->st_shndx != SHN_ABS) {
				*got = sym->st_value +
				    (Elf_Addr)obj->relocbase;
				if ((Elf_Addr)(*got) == (Elf_Addr)obj->relocbase) {
					dbg("Warning3, i:%d maps to relocbase address:%p",
					    i, obj->relocbase);
				}
			}
		} else {
			/* TODO: add cache here */
			def = find_symdef(i, obj, &defobj, false, NULL,
			    lockstate);
			if (def == NULL) {
				dbg("Warning4, can't find symbole %d", i);
				return -1;
			}
			*got = def->st_value + (Elf_Addr)defobj->relocbase;
			if ((Elf_Addr)(*got) == (Elf_Addr)obj->relocbase) {
				dbg("Warning4, i:%d maps to relocbase address:%p",
				    i, obj->relocbase);
				dbg("via first obj symbol %s",
				    obj->strtab + obj->symtab[i].st_name);
				dbg("found in obj %p:%s",
				    defobj, defobj->path);
			}
		}

		dbg("  --> now %lx", (u_long) *got);
		++sym;
		++got;
	}

	got = obj->pltgot;
	rellim = (const Elf_Rel *)((caddr_t)obj->rel + obj->relsize);
	for (rel = obj->rel; rel < rellim; rel++) {
		Elf_Word	r_symndx, r_type;
		void		*where;

		where = obj->relocbase + rel->r_offset;
		r_symndx = ELF_R_SYM(rel->r_info);
		r_type = ELF_R_TYPE(rel->r_info);

		switch (r_type & 0xff) {
		case R_TYPE(NONE):
			break;

		case R_TYPE(REL32): {
			/* 32-bit PC-relative reference */
			const size_t rlen =
			    ELF_R_NXTTYPE_64_P(r_type)
				? sizeof(Elf_Sxword)
				: sizeof(Elf_Sword);
			Elf_Sxword old = load_ptr(where, rlen);
			Elf_Sxword val = old;

			def = obj->symtab + r_symndx;

			if (r_symndx >= obj->gotsym) {
				val += got[obj->local_gotno + r_symndx - obj->gotsym];
				dbg("REL32/G(%p) %p --> %p (%s) in %s",
				    where, (void *)old, (void *)val,
				    obj->strtab + def->st_name,
				    obj->path);
			} else {
				/*
				 * XXX: ABI DIFFERENCE!
				 *
				 * Old NetBSD binutils would generate shared
				 * libs with section-relative relocations being
				 * already adjusted for the start address of
				 * the section.
				 *
				 * New binutils, OTOH, generate shared libs
				 * with the same relocations being based at
				 * zero, so we need to add in the start address
				 * of the section.
				 *
				 * --rkb, Oct 6, 2001
				 */

				if (def->st_info ==
				    ELF_ST_INFO(STB_LOCAL, STT_SECTION)
#ifdef SUPPORT_OLD_BROKEN_LD
				    && !broken
#endif
				    )
					val += (Elf_Addr)def->st_value;

				val += (Elf_Addr)obj->relocbase;

				dbg("REL32/L(%p) %p -> %p (%s) in %s",
				    where, (void *)old, (void *)val,
				    obj->strtab + def->st_name, obj->path);
			}
			store_ptr(where, val, rlen);
			break;
		}

#ifdef __mips_n64
		case R_TYPE(TLS_DTPMOD64):
#else
		case R_TYPE(TLS_DTPMOD32): 
#endif
		{

			const size_t rlen = sizeof(Elf_Addr);
			Elf_Addr old = load_ptr(where, rlen);
			Elf_Addr val = old;

        		def = find_symdef(r_symndx, obj, &defobj, false, NULL,
	    			lockstate);
			if (def == NULL)
				return -1;

			val += (Elf_Addr)defobj->tlsindex;

			store_ptr(where, val, rlen);
			dbg("DTPMOD %s in %s %p --> %p in %s",
			    obj->strtab + obj->symtab[r_symndx].st_name,
			    obj->path, (void *)old, (void*)val, defobj->path);
			break;
		}

#ifdef __mips_n64
		case R_TYPE(TLS_DTPREL64):
#else
		case R_TYPE(TLS_DTPREL32):
#endif
		{
			const size_t rlen = sizeof(Elf_Addr);
			Elf_Addr old = load_ptr(where, rlen);
			Elf_Addr val = old;

        		def = find_symdef(r_symndx, obj, &defobj, false, NULL,
	    			lockstate);
			if (def == NULL)
				return -1;

			if (!defobj->tls_done && allocate_tls_offset(obj))
				return -1;

			val += (Elf_Addr)def->st_value - TLS_DTP_OFFSET;
			store_ptr(where, val, rlen);

			dbg("DTPREL %s in %s %p --> %p in %s",
			    obj->strtab + obj->symtab[r_symndx].st_name,
			    obj->path, (void*)old, (void *)val, defobj->path);
			break;
		}

#ifdef __mips_n64
		case R_TYPE(TLS_TPREL64):
#else
		case R_TYPE(TLS_TPREL32):
#endif
		{
			const size_t rlen = sizeof(Elf_Addr);
			Elf_Addr old = load_ptr(where, rlen);
			Elf_Addr val = old;

        		def = find_symdef(r_symndx, obj, &defobj, false, NULL,
	    			lockstate);

			if (def == NULL)
				return -1;

			if (!defobj->tls_done && allocate_tls_offset(obj))
				return -1;

			val += (Elf_Addr)(def->st_value + defobj->tlsoffset
			    - TLS_TP_OFFSET - TLS_TCB_SIZE);
			store_ptr(where, val, rlen);

			dbg("TPREL %s in %s %p --> %p in %s",
			    obj->strtab + obj->symtab[r_symndx].st_name,
			    obj->path, (void*)old, (void *)val, defobj->path);
			break;
		}



		default:
			dbg("sym = %lu, type = %lu, offset = %p, "
			    "contents = %p, symbol = %s",
			    (u_long)r_symndx, (u_long)ELF_R_TYPE(rel->r_info),
			    (void *)rel->r_offset,
			    (void *)load_ptr(where, sizeof(Elf_Sword)),
			    obj->strtab + obj->symtab[r_symndx].st_name);
			_rtld_error("%s: Unsupported relocation type %ld "
			    "in non-PLT relocations",
			    obj->path, (u_long) ELF_R_TYPE(rel->r_info));
			return -1;
		}
	}

	return 0;
}

/*
 *  Process the PLT relocations.
 */
int
reloc_plt(Obj_Entry *obj)
{
#if 0
	const Elf_Rel *rellim;
	const Elf_Rel *rel;
		
	dbg("reloc_plt obj:%p pltrel:%p sz:%s", obj, obj->pltrel, (int)obj->pltrelsize);
	dbg("gottable %p num syms:%s", obj->pltgot, obj->symtabno );
	dbg("*****************************************************");
	rellim = (const Elf_Rel *)((char *)obj->pltrel +
	    obj->pltrelsize);
	for (rel = obj->pltrel;  rel < rellim;  rel++) {
		Elf_Addr *where;
		where = (Elf_Addr *)(obj->relocbase + rel->r_offset);
		*where += (Elf_Addr )obj->relocbase;
	}

#endif
	/* PLT fixups were done above in the GOT relocation. */
	return (0);
}

/*
 * LD_BIND_NOW was set - force relocation for all jump slots
 */
int
reloc_jmpslots(Obj_Entry *obj, RtldLockState *lockstate)
{
	/* Do nothing */
	obj->jmpslots_done = true;
	
	return (0);
}

int
reloc_iresolve(Obj_Entry *obj, struct Struct_RtldLockState *lockstate)
{

	/* XXX not implemented */
	return (0);
}

int
reloc_gnu_ifunc(Obj_Entry *obj, struct Struct_RtldLockState *lockstate)
{

	/* XXX not implemented */
	return (0);
}

Elf_Addr
reloc_jmpslot(Elf_Addr *where, Elf_Addr target, const Obj_Entry *defobj,
    		const Obj_Entry *obj, const Elf_Rel *rel)
{

	/* Do nothing */

	return target;
}

void
allocate_initial_tls(Obj_Entry *objs)
{
	char *tls;
	
	/*
	 * Fix the size of the static TLS block by using the maximum
	 * offset allocated so far and adding a bit for dynamic modules to
	 * use.
	 */
	tls_static_space = tls_last_offset + tls_last_size + RTLD_STATIC_TLS_EXTRA;

	tls = (char *) allocate_tls(objs, NULL, TLS_TCB_SIZE, 8);

	sysarch(MIPS_SET_TLS, tls);
}

void *
__tls_get_addr(tls_index* ti)
{
	Elf_Addr** tls;
	char *p;

	sysarch(MIPS_GET_TLS, &tls);

	p = tls_get_addr_common(tls, ti->ti_module, ti->ti_offset + TLS_DTP_OFFSET);

	return (p);
}
