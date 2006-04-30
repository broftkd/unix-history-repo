/* Target macros for GCC for SPARC running System V.4
   Copyright (C) 2003
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Provide a set of pre-definitions and pre-assertions appropriate for
   the SPARC running svr4.  __svr4__ is our extension.  */

/* Target OS builtins.  */			\
#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	builtin_define_std ("sparc");		\
	builtin_define_std ("unix");		\
	builtin_define ("__svr4__");		\
	builtin_assert ("system=unix");		\
	builtin_assert ("system=svr4");		\
    }						\
  while (0)
