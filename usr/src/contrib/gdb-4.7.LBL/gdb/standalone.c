/* Interface to bare machine for GDB running as kernel debugger.
   Copyright (C) 1986, 1989, 1991 Free Software Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined (SIGTSTP) && defined (SIGIO)
#include <sys/time.h>
#include <sys/resource.h>
#endif /* SIGTSTP and SIGIO defined (must be 4.2) */

#include "defs.h"
#include "signals.h"
#include "symtab.h"
#include "frame.h"
#include "inferior.h"
#include "wait.h"


/* Random system calls, mostly no-ops to prevent link problems  */

ioctl (desc, code, arg)
{}

int (* signal ()) ()
{}

kill ()
{}

getpid ()
{
  return 0;
}

sigsetmask ()
{}

chdir ()
{}

char *
getcwd (buf, len)
     char *buf;
     unsigned int len;
{
  buf[0] = '/';
  buf[1] = 0;
  return buf;
}

/* Used to check for existence of .gdbinit.  Say no.  */

access ()
{
  return -1;
}

exit ()
{
  error ("Fatal error; restarting.");
}

/* Reading "files".  The contents of some files are written into kdb's
   data area before it is run.  These files are used to contain the
   symbol table for kdb to load, and the source files (in case the
   kdb user wants to print them).  The symbols are stored in a file
   named "kdb-symbols" in a.out format (except that all the text and
   data have been stripped to save room).

   The files are stored in the following format:
   int     number of bytes of data for this file, including these four.
   char[]  name of the file, ending with a null.
   padding to multiple of 4 boundary.
   char[]  file contents.  The length can be deduced from what was
           specified before.  There is no terminating null here.

   If the int at the front is zero, it means there are no more files.

   Opening a file in kdb returns a nonzero value to indicate success,
   but the value does not matter.  Only one file can be open, and only
   for reading.  All the primitives for input from the file know
   which file is open and ignore what is specified for the descriptor
   or for the stdio stream.

   Input with fgetc can be done either on the file that is open
   or on stdin (which reads from the terminal through tty_input ()  */

/* Address of data for the files stored in format described above.  */
char *files_start;

/* The file stream currently open:  */

char *sourcebeg;		/* beginning of contents */
int sourcesize;			/* size of contents */
char *sourceptr;		/* current read pointer */
int sourceleft;			/* number of bytes to eof */

/* "descriptor" for the file now open.
   Incremented at each close.
   If specified descriptor does not match this,
   it means the program is trying to use a closed descriptor.
   We report an error for that.  */

int sourcedesc;

open (filename, modes)
     char *filename;
     int modes;
{
  register char *next;

  if (modes)
    {
      errno = EROFS;
      return -1;
    }

  if (sourceptr)
    {
      errno = EMFILE;
      return -1;
    }

  for (next - files_start; * (int *) next;
       next += * (int *) next)
    {
      if (!strcmp (next + 4, filename))
	{
	  sourcebeg = next + 4 + strlen (next + 4) + 1;
	  sourcebeg = (char *) (((int) sourcebeg + 3) & (-4));
	  sourceptr = sourcebeg;
	  sourcesize = next + * (int *) next - sourceptr;
	  sourceleft = sourcesize;
	  return sourcedesc;
	}
    }
  return 0;
}

close (desc)
     int desc;
{
  sourceptr = 0;
  sourcedesc++;
  /* Don't let sourcedesc get big enough to be confused with stdin.  */
  if (sourcedesc == 100)
    sourcedesc = 5;
}

FILE *
fopen (filename, modes)
     char *filename;
     char *modes;
{
  return (FILE *) open (filename, *modes == 'w');
}

FILE *
fdopen (desc)
     int desc;
{
  return (FILE *) desc;
}

fclose (desc)
     int desc;
{
  close (desc);
}

fstat (desc, statbuf)
     struct stat *statbuf;
{
  if (desc != sourcedesc)
    {
      errno = EBADF;
      return -1;
    }
  statbuf->st_size = sourcesize;
}

myread (desc, destptr, size, filename)
     int desc;
     char *destptr;
     int size;
     char *filename;
{
  int len = min (sourceleft, size);

  if (desc != sourcedesc)
    {
      errno = EBADF;
      return -1;
    }

  bcopy (sourceptr, destptr, len);
  sourceleft -= len;
  return len;
}

int
fread (bufp, numelts, eltsize, stream)
{
  register int elts = min (numelts, sourceleft / eltsize);
  register int len = elts * eltsize;

  if (stream != sourcedesc)
    {
      errno = EBADF;
      return -1;
    }

  bcopy (sourceptr, bufp, len);
  sourceleft -= len;
  return elts;
}

int
fgetc (desc)
     int desc;
{

  if (desc == (int) stdin)
    return tty_input ();

  if (desc != sourcedesc)
    {
      errno = EBADF;
      return -1;
    }

  if (sourceleft-- <= 0)
    return EOF;
  return *sourceptr++;
}

lseek (desc, pos)
     int desc;
     int pos;
{

  if (desc != sourcedesc)
    {
      errno = EBADF;
      return -1;
    }

  if (pos < 0 || pos > sourcesize)
    {
      errno = EINVAL;
      return -1;
    }

  sourceptr = sourcebeg + pos;
  sourceleft = sourcesize - pos;
}

/* Output in kdb can go only to the terminal, so the stream
   specified may be ignored.  */

printf (a1, a2, a3, a4, a5, a6, a7, a8, a9)
{
  char buffer[1024];
  sprintf (buffer, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  display_string (buffer);
}

fprintf (ign, a1, a2, a3, a4, a5, a6, a7, a8, a9)
{
  char buffer[1024];
  sprintf (buffer, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  display_string (buffer);
}

fwrite (buf, numelts, size, stream)
     register char *buf;
     int numelts, size;
{
  register int i = numelts * size;
  while (i-- > 0)
    fputc (*buf++, stream);
}

fputc (c, ign)
{
  char buf[2];
  buf[0] = c;
  buf[1] = 0;
  display_string (buf);
}

/* sprintf refers to this, but loading this from the
   library would cause fflush to be loaded from it too.
   In fact there should be no need to call this (I hope).  */

_flsbuf ()
{
  error ("_flsbuf was actually called.");
}

fflush (ign)
{
}

/* Entries into core and inflow, needed only to make things link ok.  */

exec_file_command ()
{}

core_file_command ()
{}

char *
get_exec_file (err)
     int err;
{
  /* Makes one printout look reasonable; value does not matter otherwise.  */
  return "run";
}

/* Nonzero if there is a core file.  */

have_core_file_p ()
{
  return 0;
}

kill_command ()
{
  inferior_pid = 0;
}

terminal_inferior ()
{}

terminal_ours ()
{}

terminal_init_inferior ()
{}

write_inferior_register ()
{}

read_inferior_register ()
{}

read_memory (memaddr, myaddr, len)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
{
  bcopy (memaddr, myaddr, len);
}

/* Always return 0 indicating success.  */

write_memory (memaddr, myaddr, len)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
{
  bcopy (myaddr, memaddr, len);
  return 0;
}

static REGISTER_TYPE saved_regs[NUM_REGS];

REGISTER_TYPE
read_register (regno)
     int regno;
{
  if (regno < 0 || regno >= NUM_REGS)
    error ("Register number %d out of range.", regno);
  return saved_regs[regno];
}

void
write_register (regno, value)
     int regno;
     REGISTER_TYPE value;
{
  if (regno < 0 || regno >= NUM_REGS)
    error ("Register number %d out of range.", regno);
  saved_regs[regno] = value;
}

/* System calls needed in relation to running the "inferior".  */

vfork ()
{
  /* Just appear to "succeed".  Say the inferior's pid is 1.  */
  return 1;
}

/* These are called by code that normally runs in the inferior
   that has just been forked.  That code never runs, when standalone,
   and these definitions are so it will link without errors.  */

ptrace ()
{}

setpgrp ()
{}

execle ()
{}

_exit ()
{}

/* Malloc calls these.  */

malloc_warning (str)
     char *str;
{
  printf ("\n%s.\n\n", str);
}

char *next_free;
char *memory_limit;

char *
sbrk (amount)
     int amount;
{
  if (next_free + amount > memory_limit)
    return (char *) -1;
  next_free += amount;
  return next_free - amount;
}

/* Various ways malloc might ask where end of memory is.  */

char *
ulimit ()
{
  return memory_limit;
}

int
vlimit ()
{
  return memory_limit - next_free;
}

getrlimit (addr)
     struct rlimit *addr;
{
  addr->rlim_cur = memory_limit - next_free;
}

/* Context switching to and from program being debugged.  */

/* GDB calls here to run the user program.
   The frame pointer for this function is saved in
   gdb_stack by save_frame_pointer; then we restore
   all of the user program's registers, including PC and PS.  */

static int fault_code;
static REGISTER_TYPE gdb_stack;

resume ()
{
  REGISTER_TYPE restore[NUM_REGS];

  PUSH_FRAME_PTR;
  save_frame_pointer ();

  bcopy (saved_regs, restore, sizeof restore);
  POP_REGISTERS;
  /* Control does not drop through here!  */
}

save_frame_pointer (val)
     CORE_ADDR val;
{
  gdb_stack = val;
}

/* Fault handlers call here, running in the user program stack.
   They must first push a fault code,
   old PC, old PS, and any other info about the fault.
   The exact format is machine-dependent and is known only
   in the definition of PUSH_REGISTERS.  */

fault ()
{
  /* Transfer all registers and fault code to the stack
     in canonical order: registers in order of GDB register number,
     followed by fault code.  */
  PUSH_REGISTERS;

  /* Transfer them to saved_regs and fault_code.  */
  save_registers ();

  restore_gdb ();
  /* Control does not reach here */
}

restore_gdb ()
{
  CORE_ADDR new_fp = gdb_stack;
  /* Switch to GDB's stack  */
  POP_FRAME_PTR;
  /* Return from the function `resume'.  */
}

/* Assuming register contents and fault code have been pushed on the stack as
   arguments to this function, copy them into the standard place
   for the program's registers while GDB is running.  */

save_registers (firstreg)
     int firstreg;
{
  bcopy (&firstreg, saved_regs, sizeof saved_regs);
  fault_code = (&firstreg)[NUM_REGS];
}

/* Store into the structure such as `wait' would return
   the information on why the program faulted,
   converted into a machine-independent signal number.  */

static int fault_table[] = FAULT_TABLE;

int
wait (w)
     WAITTYPE *w;
{
  WSETSTOP (*w, fault_table[fault_code / FAULT_CODE_UNITS]);
  return inferior_pid;
}

/* Allocate a big space in which files for kdb to read will be stored.
   Whatever is left is where malloc can allocate storage.

   Initialize it, so that there will be space in the executable file
   for it.  Then the files can be put into kdb by writing them into
   kdb's executable file.  */

/* The default size is as much space as we expect to be available
   for kdb to use!  */

#ifndef HEAP_SIZE
#define HEAP_SIZE 400000
#endif

char heap[HEAP_SIZE] = {0};

#ifndef STACK_SIZE
#define STACK_SIZE 100000
#endif

int kdb_stack_beg[STACK_SIZE / sizeof (int)];
int kdb_stack_end;

_initialize_standalone ()
{
  register char *next;

  /* Find start of data on files.  */

  files_start = heap;

  /* Find the end of the data on files.  */

  for (next - files_start; * (int *) next;
       next += * (int *) next)
    {}

  /* That is where free storage starts for sbrk to give out.  */
  next_free = next;

  memory_limit = heap + sizeof heap;
}

