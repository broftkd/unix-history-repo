/*
 *      CONF.H
 *      UTREE system dependent configurable definitions.
 *      3.03-um klin, Sat Feb 15 19:23:58 1992
 *
 *      Copyright (c) 1991/92 by Peter Klingebiel & UNIX Magazin Muenchen.
 *      For copying and distribution information see the file COPYRIGHT.
 *
 *      Version:        Consensys V.4 UNIX
 */
#if     defined(_MAIN_) && !defined(lint)
static char sccsid_conf[] = "@(#) utree 3.03-um (klin) Feb 15 1992 conf.h";
#endif  /* _MAIN_ && !lint */

/*
 *      This file contains definitions you can change for your needs.
 */

/*
 *      SOME UTREE DEPENDENT DEFINITIONS.
 */

/* BSD: define HASVSPRINTF if your system supports vsprintf(3)          */
/*#define HASVSPRINTF             /* Not needed for SYSV!                 */

/* SYSV: define HASFIONREAD if your system supports the FIONREAD        */
/*       ioctl(2) system call to check how many chars are typed ahead   */
#define HASFIONREAD             /* Not needed for BSD!                  */
#include <sys/filio.h>          /* Needed for definition of FIONREAD    */

/* SYSV: define HASVFORK if your system supports vfork(2)               */
#define HASVFORK                /* Not needed for BSD!                  */

/* SYSV: define NODIRENT if your system not supports the directory type */
/*       struct dirent with opendir(3), closedir(3) and readdir(3)      */
/*#define NODIRENT                /* Needed only for older SYSVs          */

/* ALL: define NOWINCHG to ignore sreen resizing on window systems. The */
/*      handling of screen resizing is intended as a little bit support */
/*      for window systems like X, but it may not run on your system.   */
/*      ATTENTION: Resizing may be confusing for utree anyway!          */
/*#define NOWINCHG                /* No screen resizing allowed           */

/* ALL: define STRUCTCOPY if your compiler doesn't support assignment   */
/*      of struct variables, i.e. t[o] = f[rom].                        */
/*#define STRUCTCOPY(f, t) memcpy(t, f, sizeof(f))        /* For SYSV     */
/*#define STRUCTCOPY(f, t) bcopy(f, t, sizeof(f))         /* For BSD      */

/* ALL: define UTCLOCK for a clock updated in echo line every second    */
#define UTCLOCK                 /* Show/update clock                    */

/* ALL: directory for utree startup file and help pages                 */
#ifndef UTLIB
# define UTLIB "/usr/local/lib"
#endif  /* !UTLIB */
