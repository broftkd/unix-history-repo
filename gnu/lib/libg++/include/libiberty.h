/* Function declarations for libiberty.
   Written by Cygnus Support, 1994.

   The libiberty library provides a number of functions which are
   missing on some operating systems.  We do not declare those here,
   to avoid conflicts with the system header files on operating
   systems that do support those functions.  In this file we only
   declare those functions which are specific to libiberty.  */

#ifndef LIBIBERTY_H
#define LIBIBERTY_H

#include "ansidecl.h"

/* Build an argument vector from a string.  Allocates memory using
   malloc.  Use freeargv to free the vector.  */

extern char **buildargv PARAMS ((char *));

/* Free a vector returned by buildargv.  */

extern void freeargv PARAMS ((char **));

/* Return the last component of a path name.  */

extern char *basename PARAMS ((char *));

/* Concatenate an arbitrary number of strings, up to (char *) NULL.
   Allocates memory using xmalloc.  */

extern char *concat PARAMS ((const char *, ...));

/* Check whether two file descriptors refer to the same file.  */

extern int fdmatch PARAMS ((int fd1, int fd2));

/* Get the amount of time the process has run, in microseconds.  */

extern long get_run_time PARAMS ((void));

/* Allocate memory filled with spaces.  Allocates using malloc.  */

extern const char *spaces PARAMS ((int count));

/* Return the maximum error number for which strerror will return a
   string.  */

extern int errno_max PARAMS ((void));

/* Return the name of an errno value (e.g., strerrno (EINVAL) returns
   "EINVAL").  */

extern const char *strerrno PARAMS ((int));

/* Given the name of an errno value, return the value.  */

extern int strtoerrno PARAMS ((const char *));

/* Return the maximum signal number for which strsignal will return a
   string.  */

extern int signo_max PARAMS ((void));

/* Return the name of a signal number (e.g., strsigno (SIGHUP) returns
   "SIGHUP").  */

extern const char *strsigno PARAMS ((int));

/* Given the name of a signal, return its number.  */

extern int strtosigno PARAMS ((const char *));

/* Register a function to be run by xexit.  Returns 0 on success.  */

extern int xatexit PARAMS ((void (*fn) (void)));

/* Exit, calling all the functions registered with xatexit.  */

#ifndef __GNUC__
extern void xexit PARAMS ((int status));
#else
typedef void libiberty_voidfn PARAMS ((int status));
__volatile__ libiberty_voidfn xexit;
#endif

/* Set the program name used by xmalloc.  */

extern void xmalloc_set_program_name PARAMS ((const char *));

/* Allocate memory without fail.  If malloc fails, this will print a
   message to stderr (using the name set by xmalloc_set_program_name,
   if any) and then call xexit.

   FIXME: We do not declare the parameter type (size_t) in order to
   avoid conflicts with other declarations of xmalloc that exist in
   programs which use libiberty.  */

extern PTR xmalloc ();

/* Reallocate memory without fail.  This works like xmalloc.

   FIXME: We do not declare the parameter types for the same reason as
   xmalloc.  */

extern PTR xrealloc ();

#endif /* ! defined (LIBIBERTY_H) */
