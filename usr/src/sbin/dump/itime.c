/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)itime.c	5.5 (Berkeley) %G%";
#endif /* not lint */

#include "dump.h"
#include <sys/file.h>
#include <errno.h>

struct	dumpdates **ddatev = 0;
int	nddates = 0;
int	ddates_in = 0;
struct	dumptime *dthead = 0;

void	readdumptimes();
int	getrecord();
int	makedumpdate();

static void dumprecout();

void
initdumptimes()
{
	FILE *df;

	if ((df = fopen(dumpdates, "r")) == NULL) {
		if (errno == ENOENT) {
			msg("WARNING: no file `%s'\n", dumpdates);
			return;
		}
		quit("cannot read %s: %s\n", dumpdates, strerror(errno));
		/* NOTREACHED */
	}
	(void) flock(fileno(df), LOCK_SH);
	readdumptimes(df);
	fclose(df);
}

void
readdumptimes(df)
	FILE *df;
{
	register int i;
	register struct	dumptime *dtwalk;

	for (;;) {
		dtwalk = (struct dumptime *)calloc(1, sizeof (struct dumptime));
		if (getrecord(df, &(dtwalk->dt_value)) < 0)
			break;
		nddates++;
		dtwalk->dt_next = dthead;
		dthead = dtwalk;
	}

	ddates_in = 1;
	/*
	 *	arrayify the list, leaving enough room for the additional
	 *	record that we may have to add to the ddate structure
	 */
	ddatev = (struct dumpdates **)
		calloc(nddates + 1, sizeof (struct dumpdates *));
	dtwalk = dthead;
	for (i = nddates - 1; i >= 0; i--, dtwalk = dtwalk->dt_next)
		ddatev[i] = &dtwalk->dt_value;
}

void
getdumptime()
{
	register struct dumpdates *ddp;
	register int i;
	char *fname;

	fname = disk;
#ifdef FDEBUG
	msg("Looking for name %s in dumpdates = %s for level = %c\n",
		fname, dumpdates, level);
#endif
	spcl.c_ddate = 0;
	lastlevel = '0';

	initdumptimes();
	/*
	 *	Go find the entry with the same name for a lower increment
	 *	and older date
	 */
	ITITERATE(i, ddp) {
		if (strncmp(fname, ddp->dd_name, sizeof (ddp->dd_name)) != 0)
			continue;
		if (ddp->dd_level >= level)
			continue;
		if (ddp->dd_ddate <= spcl.c_ddate)
			continue;
		spcl.c_ddate = ddp->dd_ddate;
		lastlevel = ddp->dd_level;
	}
}

void
putdumptime()
{
	FILE *df;
	register struct dumpdates *dtwalk;
	register int i;
	int fd;
	char *fname;

	if(uflag == 0)
		return;
	if ((df = fopen(dumpdates, "r+")) == NULL)
		quit("cannot rewrite %s: %s\n", dumpdates, strerror(errno));
	fd = fileno(df);
	(void) flock(fd, LOCK_EX);
	fname = disk;
	free(ddatev);
	ddatev = 0;
	nddates = 0;
	dthead = 0;
	ddates_in = 0;
	readdumptimes(df);
	if (fseek(df, 0L, 0) < 0)
		quit("fseek: %s\n", strerror(errno));
	spcl.c_ddate = 0;
	ITITERATE(i, dtwalk) {
		if (strncmp(fname, dtwalk->dd_name,
				sizeof (dtwalk->dd_name)) != 0)
			continue;
		if (dtwalk->dd_level != level)
			continue;
		goto found;
	}
	/*
	 *	construct the new upper bound;
	 *	Enough room has been allocated.
	 */
	dtwalk = ddatev[nddates] =
		(struct dumpdates *)calloc(1, sizeof(struct dumpdates));
	nddates += 1;
  found:
	(void) strncpy(dtwalk->dd_name, fname, sizeof (dtwalk->dd_name));
	dtwalk->dd_level = level;
	dtwalk->dd_ddate = spcl.c_date;

	ITITERATE(i, dtwalk) {
		dumprecout(df, dtwalk);
	}
	if (fflush(df))
		quit("%s: %s\n", dumpdates, strerror(errno));
	if (ftruncate(fd, ftell(df)))
		quit("ftruncate (%s): %s\n", dumpdates, strerror(errno));
	(void) fclose(df);
	msg("level %c dump on %s", level,
		spcl.c_date == 0 ? "the epoch\n" : ctime(&spcl.c_date));
}

static void
dumprecout(file, what)
	FILE *file;
	struct dumpdates *what;
{

	if (fprintf(file, DUMPOUTFMT,
		    what->dd_name,
		    what->dd_level,
		    ctime(&what->dd_ddate)) < 0)
		quit("%s: %s\n", dumpdates, strerror(errno));
}

int	recno;
int
getrecord(df, ddatep)
	FILE *df;
	struct dumpdates *ddatep;
{
	char buf[BUFSIZ];

	recno = 0;
	if ( (fgets(buf, BUFSIZ, df)) != buf)
		return(-1);
	recno++;
	if (makedumpdate(ddatep, buf) < 0)
		msg("Unknown intermediate format in %s, line %d\n",
			dumpdates, recno);

#ifdef FDEBUG
	msg("getrecord: %s %c %s", ddatep->dd_name, ddatep->dd_level,
	    ddatep->dd_ddate == 0 ? "the epoch\n" : ctime(&ddatep->dd_ddate));
#endif
	return(0);
}

time_t	unctime();

int
makedumpdate(ddp, buf)
	struct dumpdates *ddp;
	char *buf;
{
	char un_buf[128];

	sscanf(buf, DUMPINFMT, ddp->dd_name, &ddp->dd_level, un_buf);
	ddp->dd_ddate = unctime(un_buf);
	if (ddp->dd_ddate < 0)
		return(-1);
	return(0);
}
