/* ar.c - Archive modify and extract.
   Copyright 1991, 92, 93, 94, 95, 96, 97, 98, 99, 2000
   Free Software Foundation, Inc.

This file is part of GNU Binutils.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
   Bugs: should use getopt the way tar does (complete w/optional -) and
   should have long options too. GNU ar used to check file against filesystem
   in quick_update and replace operations (would check mtime). Doesn't warn
   when name truncated. No way to specify pos_end. Error messages should be
   more consistant.
*/
#include "bfd.h"
#include "libiberty.h"
#include "progress.h"
#include "bucomm.h"
#include "aout/ar.h"
#include "libbfd.h"
#include "arsup.h"
#include <sys/stat.h>

#ifdef __GO32___
#define EXT_NAME_LEN 3		/* bufflen of addition to name if it's MS-DOS */
#else
#define EXT_NAME_LEN 6		/* ditto for *NIX */
#endif

/* We need to open files in binary modes on system where that makes a
   difference.  */
#ifndef O_BINARY
#define O_BINARY 0
#endif

#define BUFSIZE 8192

/* Kludge declaration from BFD!  This is ugly!  FIXME!  XXX */

struct ar_hdr *
  bfd_special_undocumented_glue PARAMS ((bfd * abfd, const char *filename));

/* Static declarations */

static void
mri_emul PARAMS ((void));

static const char *
normalize PARAMS ((const char *, bfd *));

static void
remove_output PARAMS ((void));

static void
map_over_members PARAMS ((bfd *, void (*)(bfd *), char **, int));

static void
print_contents PARAMS ((bfd * member));

static void
delete_members PARAMS ((bfd *, char **files_to_delete));

#if 0
static void
do_quick_append PARAMS ((const char *archive_filename,
			 char **files_to_append));
#endif

static void
move_members PARAMS ((bfd *, char **files_to_move));

static void
replace_members PARAMS ((bfd *, char **files_to_replace, boolean quick));

static void
print_descr PARAMS ((bfd * abfd));

static void
write_archive PARAMS ((bfd *));

static void
ranlib_only PARAMS ((const char *archname));

static void
ranlib_touch PARAMS ((const char *archname));

static void
usage PARAMS ((int));

/** Globals and flags */

int mri_mode;

/* This flag distinguishes between ar and ranlib:
   1 means this is 'ranlib'; 0 means this is 'ar'.
   -1 means if we should use argv[0] to decide.  */
extern int is_ranlib;

/* Nonzero means don't warn about creating the archive file if necessary.  */
int silent_create = 0;

/* Nonzero means describe each action performed.  */
int verbose = 0;

/* Nonzero means preserve dates of members when extracting them.  */
int preserve_dates = 0;

/* Nonzero means don't replace existing members whose dates are more recent
   than the corresponding files.  */
int newer_only = 0;

/* Controls the writing of an archive symbol table (in BSD: a __.SYMDEF
   member).  -1 means we've been explicitly asked to not write a symbol table;
   +1 means we've been explictly asked to write it;
   0 is the default.
   Traditionally, the default in BSD has been to not write the table.
   However, for POSIX.2 compliance the default is now to write a symbol table
   if any of the members are object files.  */
int write_armap = 0;

/* Nonzero means it's the name of an existing member; position new or moved
   files with respect to this one.  */
char *posname = NULL;

/* Sez how to use `posname': pos_before means position before that member.
   pos_after means position after that member. pos_end means always at end.
   pos_default means default appropriately. For the latter two, `posname'
   should also be zero.  */
enum pos
  {
    pos_default, pos_before, pos_after, pos_end
  } postype = pos_default;

static bfd **
get_pos_bfd PARAMS ((bfd **, enum pos, const char *));

/* For extract/delete only.  If COUNTED_NAME_MODE is true, we only
   extract the COUNTED_NAME_COUNTER instance of that name.  */
static boolean counted_name_mode = 0;
static int counted_name_counter = 0;

/* Whether to truncate names of files stored in the archive.  */
static boolean ar_truncate = false;

/* Whether to use a full file name match when searching an archive.
   This is convenient for archives created by the Microsoft lib
   program.  */
static boolean full_pathname = false;

int interactive = 0;

static void
mri_emul ()
{
  interactive = isatty (fileno (stdin));
  yyparse ();
}

/* If COUNT is 0, then FUNCTION is called once on each entry.  If nonzero,
   COUNT is the length of the FILES chain; FUNCTION is called on each entry
   whose name matches one in FILES.  */

static void
map_over_members (arch, function, files, count)
     bfd *arch;
     void (*function) PARAMS ((bfd *));
     char **files;
     int count;
{
  bfd *head;
  int match_count;

  if (count == 0)
    {
      for (head = arch->next; head; head = head->next)
	{
	  PROGRESS (1);
	  function (head);
	}
      return;
    }

  /* This may appear to be a baroque way of accomplishing what we want.
     However we have to iterate over the filenames in order to notice where
     a filename is requested but does not exist in the archive.  Ditto
     mapping over each file each time -- we want to hack multiple
     references.  */

  for (; count > 0; files++, count--)
    {
      boolean found = false;

      match_count = 0;
      for (head = arch->next; head; head = head->next)
	{
	  PROGRESS (1);
	  if (head->filename == NULL)
	    {
	      /* Some archive formats don't get the filenames filled in
		 until the elements are opened.  */
	      struct stat buf;
	      bfd_stat_arch_elt (head, &buf);
	    }
	  if ((head->filename != NULL) &&
	      (!strcmp (normalize (*files, arch), head->filename)))
	    {
	      ++match_count;
	      if (counted_name_mode
		  && match_count != counted_name_counter) 
		{
		  /* Counting, and didn't match on count; go on to the
                     next one.  */
		  continue;
		}

	      found = true;
	      function (head);
	    }
	}
      if (!found)
	/* xgettext:c-format */
	fprintf (stderr, _("no entry %s in archive\n"), *files);
    }
}

boolean operation_alters_arch = false;

static void
usage (help)
     int help;
{
  FILE *s;

  s = help ? stdout : stderr;
  
  if (! is_ranlib)
    {
      /* xgettext:c-format */
      fprintf (s, _("Usage: %s [-]{dmpqrstx}[abcfilNoPsSuvV] [member-name] [count] archive-file file...\n"),
	       program_name);
      /* xgettext:c-format */
      fprintf (s, _("       %s -M [<mri-script]\n"), program_name);
      fprintf (s, _(" commands:\n"));
      fprintf (s, _("  d            - delete file(s) from the archive\n"));
      fprintf (s, _("  m[ab]        - move file(s) in the archive\n"));
      fprintf (s, _("  p            - print file(s) found in the archive\n"));
      fprintf (s, _("  q[f]         - quick append file(s) to the archive\n"));
      fprintf (s, _("  r[ab][f][u]  - replace existing or insert new file(s) into the archive\n"));
      fprintf (s, _("  t            - display contents of archive\n"));
      fprintf (s, _("  x[o]         - extract file(s) from the archive\n"));
      fprintf (s, _(" command specific modifiers:\n"));
      fprintf (s, _("  [a]          - put file(s) after [member-name]\n"));
      fprintf (s, _("  [b]          - put file(s) before [member-name] (same as [i])\n"));
      fprintf (s, _("  [N]          - use instance [count] of name\n"));
      fprintf (s, _("  [f]          - truncate inserted file names\n"));
      fprintf (s, _("  [P]          - use full path names when matching\n"));
      fprintf (s, _("  [o]          - preserve original dates\n"));
      fprintf (s, _("  [u]          - only replace files that are newer than current archive contents\n"));
      fprintf (s, _(" generic modifiers:\n"));
      fprintf (s, _("  [c]          - do not warn if the library had to be created\n"));
      fprintf (s, _("  [s]          - create an archive index (cf. ranlib)\n"));
      fprintf (s, _("  [S]          - do not build a symbol table\n"));
      fprintf (s, _("  [v]          - be verbose\n"));
      fprintf (s, _("  [V]          - display the version number\n"));
    }
  else
    /* xgettext:c-format */
    fprintf (s, _("Usage: %s [-vV] archive\n"), program_name);

  list_supported_targets (program_name, stderr);

  if (help)
    fprintf (s, _("Report bugs to %s\n"), REPORT_BUGS_TO);

  xexit (help ? 0 : 1);
}

/* Normalize a file name specified on the command line into a file
   name which we will use in an archive.  */

static const char *
normalize (file, abfd)
     const char *file;
     bfd *abfd;
{
  const char *filename;

  if (full_pathname)
    return file;

  filename = strrchr (file, '/');
  if (filename != (char *) NULL)
    filename++;
  else
    filename = file;

  if (ar_truncate
      && abfd != NULL
      && strlen (filename) > abfd->xvec->ar_max_namelen)
    {
      char *s;

      /* Space leak.  */
      s = (char *) xmalloc (abfd->xvec->ar_max_namelen + 1);
      memcpy (s, filename, abfd->xvec->ar_max_namelen);
      s[abfd->xvec->ar_max_namelen] = '\0';
      filename = s;
    }

  return filename;
}

/* Remove any output file.  This is only called via xatexit.  */

static const char *output_filename = NULL;
static FILE *output_file = NULL;
static bfd *output_bfd = NULL;

static void
remove_output ()
{
  if (output_filename != NULL)
    {
      if (output_bfd != NULL && output_bfd->iostream != NULL)
	fclose ((FILE *) (output_bfd->iostream));
      if (output_file != NULL)
	fclose (output_file);
      unlink (output_filename);
    }
}

/* The option parsing should be in its own function.
   It will be when I have getopt working.  */

int
main (argc, argv)
     int argc;
     char **argv;
{
  char *arg_ptr;
  char c;
  enum
    {
      none = 0, delete, replace, print_table,
      print_files, extract, move, quick_append
    } operation = none;
  int arg_index;
  char **files;
  int file_count;
  char *inarch_filename;
  int show_version;

#if defined (HAVE_SETLOCALE) && defined (HAVE_LC_MESSAGES)
  setlocale (LC_MESSAGES, "");
#endif
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = argv[0];
  xmalloc_set_program_name (program_name);

  if (is_ranlib < 0)
    {
      char *temp;

      temp = strrchr (program_name, '/');
      if (temp == NULL)
	temp = program_name;
      else
	++temp;
      if (strlen (temp) >= 6
	  && strcmp (temp + strlen (temp) - 6, "ranlib") == 0)
	is_ranlib = 1;
      else
	is_ranlib = 0;
    }

  if (argc > 1 && argv[1][0] == '-')
    {
      if (strcmp (argv[1], "--help") == 0)
	usage (1);
      else if (strcmp (argv[1], "--version") == 0)
	{
	  if (is_ranlib)
	    print_version ("ranlib");
	  else
	    print_version ("ar");
	}
    }

  START_PROGRESS (program_name, 0);

  bfd_init ();
  set_default_bfd_target ();

  show_version = 0;

  xatexit (remove_output);

  if (is_ranlib)
    {
      boolean touch = false;

      if (argc < 2 || strcmp (argv[1], "--help") == 0)
	usage (0);
      if (strcmp (argv[1], "-V") == 0
	  || strcmp (argv[1], "-v") == 0
	  || strncmp (argv[1], "--v", 3) == 0)
	print_version ("ranlib");
      arg_index = 1;
      if (strcmp (argv[1], "-t") == 0)
	{
	  ++arg_index;
	  touch = true;
	}
      while (arg_index < argc)
	{
	  if (! touch)
	    ranlib_only (argv[arg_index]);
	  else
	    ranlib_touch (argv[arg_index]);
	  ++arg_index;
	}
      xexit (0);
    }

  if (argc == 2 && strcmp (argv[1], "-M") == 0)
    {
      mri_emul ();
      xexit (0);
    }

  if (argc < 2)
    usage (0);

  arg_ptr = argv[1];

  if (*arg_ptr == '-')
    ++arg_ptr;			/* compatibility */

  while ((c = *arg_ptr++) != '\0')
    {
      switch (c)
	{
	case 'd':
	case 'm':
	case 'p':
	case 'q':
	case 'r':
	case 't':
	case 'x':
	  if (operation != none)
	    fatal (_("two different operation options specified"));
	  switch (c)
	    {
	    case 'd':
	      operation = delete;
	      operation_alters_arch = true;
	      break;
	    case 'm':
	      operation = move;
	      operation_alters_arch = true;
	      break;
	    case 'p':
	      operation = print_files;
	      break;
	    case 'q':
	      operation = quick_append;
	      operation_alters_arch = true;
	      break;
	    case 'r':
	      operation = replace;
	      operation_alters_arch = true;
	      break;
	    case 't':
	      operation = print_table;
	      break;
	    case 'x':
	      operation = extract;
	      break;
	    }
	case 'l':
	  break;
	case 'c':
	  silent_create = 1;
	  break;
	case 'o':
	  preserve_dates = 1;
	  break;
	case 'V':
	  show_version = true;
	  break;
	case 's':
	  write_armap = 1;
	  break;
	case 'S':
	  write_armap = -1;
	  break;
	case 'u':
	  newer_only = 1;
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'a':
	  postype = pos_after;
	  break;
	case 'b':
	  postype = pos_before;
	  break;
	case 'i':
	  postype = pos_before;
	  break;
	case 'M':
	  mri_mode = 1;
	  break;
	case 'N':
	  counted_name_mode = true;
	  break;
	case 'f':
	  ar_truncate = true;
	  break;
	case 'P':
	  full_pathname = true;
	  break;
	default:
	  /* xgettext:c-format */
	  non_fatal (_("illegal option -- %c"), c);
	  usage (0);
	}
    }

  if (show_version)
    print_version ("ar");

  if (argc < 3)
    usage (0);

  if (mri_mode)
    {
      mri_emul ();
    }
  else
    {
      bfd *arch;

      /* We can't write an armap when using ar q, so just do ar r
         instead.  */
      if (operation == quick_append && write_armap)
	operation = replace;

      if ((operation == none || operation == print_table)
	  && write_armap == 1)
	{
	  ranlib_only (argv[2]);
	  xexit (0);
	}

      if (operation == none)
	fatal (_("no operation specified"));

      if (newer_only && operation != replace)
	fatal (_("`u' is only meaningful with the `r' option."));

      arg_index = 2;

      if (postype != pos_default)
	posname = argv[arg_index++];

      if (counted_name_mode) 
	{
          if (operation != extract && operation != delete) 
	     fatal (_("`N' is only meaningful with the `x' and `d' options."));
	  counted_name_counter = atoi (argv[arg_index++]);
          if (counted_name_counter <= 0)
	    fatal (_("Value for `N' must be positive."));
	}

      inarch_filename = argv[arg_index++];

      files = arg_index < argc ? argv + arg_index : NULL;
      file_count = argc - arg_index;

#if 0
      /* We don't use do_quick_append any more.  Too many systems
         expect ar to always rebuild the symbol table even when q is
         used.  */

      /* We can't do a quick append if we need to construct an
	 extended name table, because do_quick_append won't be able to
	 rebuild the name table.  Unfortunately, at this point we
	 don't actually know the maximum name length permitted by this
	 object file format.  So, we guess.  FIXME.  */
      if (operation == quick_append && ! ar_truncate)
	{
	  char **chk;

	  for (chk = files; chk != NULL && *chk != '\0'; chk++)
	    {
	      if (strlen (normalize (*chk, (bfd *) NULL)) > 14)
		{
		  operation = replace;
		  break;
		}
	    }
	}

      if (operation == quick_append)
	{
	  /* Note that quick appending to a non-existent archive creates it,
	     even if there are no files to append. */
	  do_quick_append (inarch_filename, files);
	  xexit (0);
	}
#endif

      arch = open_inarch (inarch_filename,
			  files == NULL ? (char *) NULL : files[0]);

      switch (operation)
	{
	case print_table:
	  map_over_members (arch, print_descr, files, file_count);
	  break;

	case print_files:
	  map_over_members (arch, print_contents, files, file_count);
	  break;

	case extract:
	  map_over_members (arch, extract_file, files, file_count);
	  break;

	case delete:
	  if (files != NULL)
	    delete_members (arch, files);
	  else
	    output_filename = NULL;
	  break;

	case move:
	  if (files != NULL)
	    move_members (arch, files);
	  else
	    output_filename = NULL;
	  break;

	case replace:
	case quick_append:
	  if (files != NULL || write_armap > 0)
	    replace_members (arch, files, operation == quick_append);
	  else
	    output_filename = NULL;
	  break;

	  /* Shouldn't happen! */
	default:
	  /* xgettext:c-format */
	  fatal (_("internal error -- this option not implemented"));
	}
    }

  END_PROGRESS (program_name);

  xexit (0);
  return 0;
}

bfd *
open_inarch (archive_filename, file)
     const char *archive_filename;
     const char *file;
{
  const char *target;
  bfd **last_one;
  bfd *next_one;
  struct stat sbuf;
  bfd *arch;
  char **matching;

  bfd_set_error (bfd_error_no_error);

  target = NULL;

  if (stat (archive_filename, &sbuf) != 0)
    {
#ifndef __GO32__

/* KLUDGE ALERT! Temporary fix until I figger why
 * stat() is wrong ... think it's buried in GO32's IDT
 * - Jax
 */
      if (errno != ENOENT)
	bfd_fatal (archive_filename);
#endif

      if (!operation_alters_arch)
	{
	  fprintf (stderr, "%s: ", program_name);
	  perror (archive_filename);
	  maybequit ();
	  return NULL;
	}

      /* Try to figure out the target to use for the archive from the
         first object on the list.  */
      if (file != NULL)
	{
	  bfd *obj;

	  obj = bfd_openr (file, NULL);
	  if (obj != NULL)
	    {
	      if (bfd_check_format (obj, bfd_object))
		target = bfd_get_target (obj);
	      (void) bfd_close (obj);
	    }
	}

      /* Create an empty archive.  */
      arch = bfd_openw (archive_filename, target);
      if (arch == NULL
	  || ! bfd_set_format (arch, bfd_archive)
	  || ! bfd_close (arch))
	bfd_fatal (archive_filename);

      /* If we die creating a new archive, don't leave it around.  */
      output_filename = archive_filename;
    }

  arch = bfd_openr (archive_filename, target);
  if (arch == NULL)
    {
    bloser:
      bfd_fatal (archive_filename);
    }

  if (! bfd_check_format_matches (arch, bfd_archive, &matching))
    {
      bfd_nonfatal (archive_filename);
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	{
	  list_matching_formats (matching);
	  free (matching);
	}
      xexit (1);
    }

  last_one = &(arch->next);
  /* Read all the contents right away, regardless.  */
  for (next_one = bfd_openr_next_archived_file (arch, NULL);
       next_one;
       next_one = bfd_openr_next_archived_file (arch, next_one))
    {
      PROGRESS (1);
      *last_one = next_one;
      last_one = &next_one->next;
    }
  *last_one = (bfd *) NULL;
  if (bfd_get_error () != bfd_error_no_more_archived_files)
    goto bloser;
  return arch;
}

static void
print_contents (abfd)
     bfd *abfd;
{
  int ncopied = 0;
  char *cbuf = xmalloc (BUFSIZE);
  struct stat buf;
  long size;
  if (bfd_stat_arch_elt (abfd, &buf) != 0)
    /* xgettext:c-format */
    fatal (_("internal stat error on %s"), bfd_get_filename (abfd));

  if (verbose)
    printf ("\n<%s>\n\n", bfd_get_filename (abfd));

  bfd_seek (abfd, 0, SEEK_SET);

  size = buf.st_size;
  while (ncopied < size)
    {

      int nread;
      int tocopy = size - ncopied;
      if (tocopy > BUFSIZE)
	tocopy = BUFSIZE;

      nread = bfd_read (cbuf, 1, tocopy, abfd);	/* oops -- broke
							   abstraction!  */
      if (nread != tocopy)
	/* xgettext:c-format */
	fatal (_("%s is not a valid archive"),
	       bfd_get_filename (bfd_my_archive (abfd)));
      fwrite (cbuf, 1, nread, stdout);
      ncopied += tocopy;
    }
  free (cbuf);
}

/* Extract a member of the archive into its own file.

   We defer opening the new file until after we have read a BUFSIZ chunk of the
   old one, since we know we have just read the archive header for the old
   one.  Since most members are shorter than BUFSIZ, this means we will read
   the old header, read the old data, write a new inode for the new file, and
   write the new data, and be done. This 'optimization' is what comes from
   sitting next to a bare disk and hearing it every time it seeks.  -- Gnu
   Gilmore  */

void
extract_file (abfd)
     bfd *abfd;
{
  FILE *ostream;
  char *cbuf = xmalloc (BUFSIZE);
  int nread, tocopy;
  long ncopied = 0;
  long size;
  struct stat buf;
  
  if (bfd_stat_arch_elt (abfd, &buf) != 0)
    /* xgettext:c-format */
    fatal (_("internal stat error on %s"), bfd_get_filename (abfd));
  size = buf.st_size;

  if (size < 0)
    /* xgettext:c-format */
    fatal (_("stat returns negative size for %s"), bfd_get_filename (abfd));
  
  if (verbose)
    printf ("x - %s\n", bfd_get_filename (abfd));

  bfd_seek (abfd, 0, SEEK_SET);

  ostream = NULL;
  if (size == 0)
    {
      /* Seems like an abstraction violation, eh?  Well it's OK! */
      output_filename = bfd_get_filename (abfd);

      ostream = fopen (bfd_get_filename (abfd), FOPEN_WB);
      if (ostream == NULL)
	{
	  perror (bfd_get_filename (abfd));
	  xexit (1);
	}

      output_file = ostream;
    }
  else
    while (ncopied < size)
      {
	tocopy = size - ncopied;
	if (tocopy > BUFSIZE)
	  tocopy = BUFSIZE;

	nread = bfd_read (cbuf, 1, tocopy, abfd);
	if (nread != tocopy)
	  /* xgettext:c-format */
	  fatal (_("%s is not a valid archive"),
		 bfd_get_filename (bfd_my_archive (abfd)));

	/* See comment above; this saves disk arm motion */
	if (ostream == NULL)
	  {
	    /* Seems like an abstraction violation, eh?  Well it's OK! */
	    output_filename = bfd_get_filename (abfd);

	    ostream = fopen (bfd_get_filename (abfd), FOPEN_WB);
	    if (ostream == NULL)
	      {
		perror (bfd_get_filename (abfd));
		xexit (1);
	      }

	    output_file = ostream;
	  }
	fwrite (cbuf, 1, nread, ostream);
	ncopied += tocopy;
      }

  if (ostream != NULL)
    fclose (ostream);

  output_file = NULL;
  output_filename = NULL;

  chmod (bfd_get_filename (abfd), buf.st_mode);

  if (preserve_dates)
    set_times (bfd_get_filename (abfd), &buf);

  free (cbuf);
}

#if 0

/* We don't use this anymore.  Too many systems expect ar to rebuild
   the symbol table even when q is used.  */

/* Just do it quickly; don't worry about dups, armap, or anything like that */

static void
do_quick_append (archive_filename, files_to_append)
     const char *archive_filename;
     char **files_to_append;
{
  FILE *ofile, *ifile;
  char *buf = xmalloc (BUFSIZE);
  long tocopy, thistime;
  bfd *temp;
  struct stat sbuf;
  boolean newfile = false;
  bfd_set_error (bfd_error_no_error);

  if (stat (archive_filename, &sbuf) != 0)
    {

#ifndef __GO32__

/* KLUDGE ALERT! Temporary fix until I figger why
 * stat() is wrong ... think it's buried in GO32's IDT
 * - Jax
 */

      if (errno != ENOENT)
	bfd_fatal (archive_filename);
#endif

      newfile = true;
    }

  ofile = fopen (archive_filename, FOPEN_AUB);
  if (ofile == NULL)
    {
      perror (program_name);
      xexit (1);
    }

  temp = bfd_openr (archive_filename, NULL);
  if (temp == NULL)
    {
      bfd_fatal (archive_filename);
    }
  if (newfile == false)
    {
      if (bfd_check_format (temp, bfd_archive) != true)
	/* xgettext:c-format */
	fatal (_("%s is not an archive"), archive_filename);
    }
  else
    {
      fwrite (ARMAG, 1, SARMAG, ofile);
      if (!silent_create)
	/* xgettext:c-format */
	non_fatal (_("creating %s"), archive_filename);
    }

  if (ar_truncate)
    temp->flags |= BFD_TRADITIONAL_FORMAT;

  /* assume it's an achive, go straight to the end, sans $200 */
  fseek (ofile, 0, 2);

  for (; files_to_append && *files_to_append; ++files_to_append)
    {
      struct ar_hdr *hdr = bfd_special_undocumented_glue (temp, *files_to_append);
      if (hdr == NULL)
	{
	  bfd_fatal (*files_to_append);
	}

      BFD_SEND (temp, _bfd_truncate_arname, (temp, *files_to_append, (char *) hdr));

      ifile = fopen (*files_to_append, FOPEN_RB);
      if (ifile == NULL)
	{
	  bfd_nonfatal (*files_to_append);
	}

      if (stat (*files_to_append, &sbuf) != 0)
	{
	  bfd_nonfatal (*files_to_append);
	}

      tocopy = sbuf.st_size;

      /* XXX should do error-checking! */
      fwrite (hdr, 1, sizeof (struct ar_hdr), ofile);

      while (tocopy > 0)
	{
	  thistime = tocopy;
	  if (thistime > BUFSIZE)
	    thistime = BUFSIZE;
	  fread (buf, 1, thistime, ifile);
	  fwrite (buf, 1, thistime, ofile);
	  tocopy -= thistime;
	}
      fclose (ifile);
      if ((sbuf.st_size % 2) == 1)
	putc ('\012', ofile);
    }
  fclose (ofile);
  bfd_close (temp);
  free (buf);
}

#endif /* 0 */

static void
write_archive (iarch)
     bfd *iarch;
{
  bfd *obfd;
  char *old_name, *new_name;
  bfd *contents_head = iarch->next;

  old_name = xmalloc (strlen (bfd_get_filename (iarch)) + 1);
  strcpy (old_name, bfd_get_filename (iarch));
  new_name = make_tempname (old_name);

  output_filename = new_name;

  obfd = bfd_openw (new_name, bfd_get_target (iarch));

  if (obfd == NULL)
    bfd_fatal (old_name);

  output_bfd = obfd;

  bfd_set_format (obfd, bfd_archive);

  /* Request writing the archive symbol table unless we've
     been explicitly requested not to.  */
  obfd->has_armap = write_armap >= 0;

  if (ar_truncate)
    {
      /* This should really use bfd_set_file_flags, but that rejects
         archives.  */
      obfd->flags |= BFD_TRADITIONAL_FORMAT;
    }

  if (bfd_set_archive_head (obfd, contents_head) != true)
    bfd_fatal (old_name);

  if (!bfd_close (obfd))
    bfd_fatal (old_name);

  output_bfd = NULL;
  output_filename = NULL;

  /* We don't care if this fails; we might be creating the archive.  */
  bfd_close (iarch);

  if (smart_rename (new_name, old_name, 0) != 0)
    xexit (1);
}

/* Return a pointer to the pointer to the entry which should be rplacd'd
   into when altering.  DEFAULT_POS should be how to interpret pos_default,
   and should be a pos value.  */

static bfd **
get_pos_bfd (contents, default_pos, default_posname)
     bfd **contents;
     enum pos default_pos;
     const char *default_posname;
{
  bfd **after_bfd = contents;
  enum pos realpos;
  const char *realposname;

  if (postype == pos_default)
    {
      realpos = default_pos;
      realposname = default_posname;
    }
  else
    {
      realpos = postype;
      realposname = posname;
    }

  if (realpos == pos_end)
    {
      while (*after_bfd)
	after_bfd = &((*after_bfd)->next);
    }
  else
    {
      for (; *after_bfd; after_bfd = &(*after_bfd)->next)
	if (strcmp ((*after_bfd)->filename, realposname) == 0)
	  {
	    if (realpos == pos_after)
	      after_bfd = &(*after_bfd)->next;
	    break;
	  }
    }
  return after_bfd;
}

static void
delete_members (arch, files_to_delete)
     bfd *arch;
     char **files_to_delete;
{
  bfd **current_ptr_ptr;
  boolean found;
  boolean something_changed = false;
  int match_count;

  for (; *files_to_delete != NULL; ++files_to_delete)
    {
      /* In a.out systems, the armap is optional.  It's also called
	 __.SYMDEF.  So if the user asked to delete it, we should remember
	 that fact. This isn't quite right for COFF systems (where
	 __.SYMDEF might be regular member), but it's very unlikely
	 to be a problem.  FIXME */

      if (!strcmp (*files_to_delete, "__.SYMDEF"))
	{
	  arch->has_armap = false;
	  write_armap = -1;
	  continue;
	}

      found = false;
      match_count = 0;
      current_ptr_ptr = &(arch->next);
      while (*current_ptr_ptr)
	{
	  if (strcmp (normalize (*files_to_delete, arch),
		      (*current_ptr_ptr)->filename) == 0)
	    {
	      ++match_count;
	      if (counted_name_mode
		  && match_count != counted_name_counter) 
		{
		  /* Counting, and didn't match on count; go on to the
                     next one.  */
		}
	      else
		{
		  found = true;
		  something_changed = true;
		  if (verbose)
		    printf ("d - %s\n",
			    *files_to_delete);
		  *current_ptr_ptr = ((*current_ptr_ptr)->next);
		  goto next_file;
		}
	    }

	  current_ptr_ptr = &((*current_ptr_ptr)->next);
	}

      if (verbose && found == false)
	{
	  /* xgettext:c-format */
	  printf (_("No member named `%s'\n"), *files_to_delete);
	}
    next_file:
      ;
    }

  if (something_changed == true)
    write_archive (arch);
  else
    output_filename = NULL;
}


/* Reposition existing members within an archive */

static void
move_members (arch, files_to_move)
     bfd *arch;
     char **files_to_move;
{
  bfd **after_bfd;		/* New entries go after this one */
  bfd **current_ptr_ptr;	/* cdr pointer into contents */

  for (; *files_to_move; ++files_to_move)
    {
      current_ptr_ptr = &(arch->next);
      while (*current_ptr_ptr)
	{
	  bfd *current_ptr = *current_ptr_ptr;
	  if (strcmp (normalize (*files_to_move, arch),
		      current_ptr->filename) == 0)
	    {
	      /* Move this file to the end of the list - first cut from
		 where it is.  */
	      bfd *link;
	      *current_ptr_ptr = current_ptr->next;

	      /* Now glue to end */
	      after_bfd = get_pos_bfd (&arch->next, pos_end, NULL);
	      link = *after_bfd;
	      *after_bfd = current_ptr;
	      current_ptr->next = link;

	      if (verbose)
		printf ("m - %s\n", *files_to_move);

	      goto next_file;
	    }

	  current_ptr_ptr = &((*current_ptr_ptr)->next);
	}
      /* xgettext:c-format */
      fatal (_("no entry %s in archive %s!"), *files_to_move, arch->filename);

    next_file:;
    }

  write_archive (arch);
}

/* Ought to default to replacing in place, but this is existing practice!  */

static void
replace_members (arch, files_to_move, quick)
     bfd *arch;
     char **files_to_move;
     boolean quick;
{
  boolean changed = false;
  bfd **after_bfd;		/* New entries go after this one */
  bfd *current;
  bfd **current_ptr;
  bfd *temp;

  while (files_to_move && *files_to_move)
    {
      if (! quick)
	{
	  current_ptr = &arch->next;
	  while (*current_ptr)
	    {
	      current = *current_ptr;

	      /* For compatibility with existing ar programs, we
		 permit the same file to be added multiple times.  */
	      if (strcmp (normalize (*files_to_move, arch),
			  normalize (current->filename, arch)) == 0
		  && current->arelt_data != NULL)
		{
		  if (newer_only)
		    {
		      struct stat fsbuf, asbuf;

		      if (stat (*files_to_move, &fsbuf) != 0)
			{
			  if (errno != ENOENT)
			    bfd_fatal (*files_to_move);
			  goto next_file;
			}
		      if (bfd_stat_arch_elt (current, &asbuf) != 0)
			/* xgettext:c-format */
			fatal (_("internal stat error on %s"), current->filename);

		      if (fsbuf.st_mtime <= asbuf.st_mtime)
			goto next_file;
		    }

		  after_bfd = get_pos_bfd (&arch->next, pos_after,
					   current->filename);
		  temp = *after_bfd;

		  *after_bfd = bfd_openr (*files_to_move, NULL);
		  if (*after_bfd == (bfd *) NULL)
		    {
		      bfd_fatal (*files_to_move);
		    }
		  (*after_bfd)->next = temp;

		  /* snip out this entry from the chain */
		  *current_ptr = (*current_ptr)->next;

		  if (verbose)
		    {
		      printf ("r - %s\n", *files_to_move);
		    }

		  changed = true;

		  goto next_file;
		}
	      current_ptr = &(current->next);
	    }
	}

      /* Add to the end of the archive.  */

      after_bfd = get_pos_bfd (&arch->next, pos_end, NULL);
      temp = *after_bfd;
      *after_bfd = bfd_openr (*files_to_move, NULL);
      if (*after_bfd == (bfd *) NULL)
	{
	  bfd_fatal (*files_to_move);
	}
      if (verbose)
	{
	  printf ("a - %s\n", *files_to_move);
	}

      (*after_bfd)->next = temp;

      changed = true;

    next_file:;

      files_to_move++;
    }

  if (changed)
    write_archive (arch);
  else
    output_filename = NULL;
}

static void
ranlib_only (archname)
     const char *archname;
{
  bfd *arch;

  write_armap = 1;
  arch = open_inarch (archname, (char *) NULL);
  if (arch == NULL)
    xexit (1);
  write_archive (arch);
}

/* Update the timestamp of the symbol map of an archive.  */

static void
ranlib_touch (archname)
     const char *archname;
{
#ifdef __GO32__
  /* I don't think updating works on go32.  */
  ranlib_only (archname);
#else
  int f;
  bfd *arch;
  char **matching;

  f = open (archname, O_RDWR | O_BINARY, 0);
  if (f < 0)
    {
      bfd_set_error (bfd_error_system_call);
      bfd_fatal (archname);
    }

  arch = bfd_fdopenr (archname, (const char *) NULL, f);
  if (arch == NULL)
    bfd_fatal (archname);
  if (! bfd_check_format_matches (arch, bfd_archive, &matching))
    {
      bfd_nonfatal (archname);
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	{
	  list_matching_formats (matching);
	  free (matching);
	}
      xexit (1);
    }

  if (! bfd_has_map (arch))
    /* xgettext:c-format */
    fatal (_("%s: no archive map to update"), archname);

  bfd_update_armap_timestamp (arch);

  if (! bfd_close (arch))
    bfd_fatal (archname);
#endif
}

/* Things which are interesting to map over all or some of the files: */

static void
print_descr (abfd)
     bfd *abfd;
{
  print_arelt_descr (stdout, abfd, verbose);
}
