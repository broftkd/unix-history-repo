# include <pwd.h>
# include "sendmail.h"

/*
**  CONF.C -- Sendmail Configuration Tables.
**
**	Defines the configuration of this installation.
**
**	Compilation Flags:
**		V6 -- running on a version 6 system.  This determines
**			whether to define certain routines between
**			the two systems.  If you are running a funny
**			system, e.g., V6 with long tty names, this
**			should be checked carefully.
**
**	Configuration Variables:
**		HdrInfo -- a table describing well-known header fields.
**			Each entry has the field name and some flags,
**			which are described in sendmail.h.
**
**	Notes:
**		I have tried to put almost all the reasonable
**		configuration information into the configuration
**		file read at runtime.  My intent is that anything
**		here is a function of the version of UNIX you
**		are running, or is really static -- for example
**		the headers are a superset of widely used
**		protocols.  If you find yourself playing with
**		this file too much, you may be making a mistake!
*/




SCCSID(@(#)conf.c	3.60		%G%);
/*
**  Header info table
**	Final (null) entry contains the flags used for any other field.
**
**	Not all of these are actually handled specially by sendmail
**	at this time.  They are included as placeholders, to let
**	you know that "someday" I intend to have sendmail do
**	something with them.
*/

struct hdrinfo	HdrInfo[] =
{
		/* date information */
	"posted-date",		0,			0,
	"date",			H_CHECK,		M_NEEDDATE,
	"resent-date",		0,			0,
	"received-date",	H_CHECK,		M_LOCAL,
		/* originator fields, most to least significant  */
	"resent-sender",	H_FROM,			0,
	"resent-from",		H_FROM,			0,
	"sender",		H_FROM,			0,
	"from",			H_FROM|H_CHECK,		M_NEEDFROM,
	"full-name",		H_ACHECK,		M_FULLNAME,
	"received-from",	H_CHECK,		M_LOCAL,
	"return-receipt-to",	H_FROM,			0,
	"errors-to",		H_FROM,			0,
		/* destination fields */
	"to",			H_RCPT,			0,
	"resent-to",		H_RCPT,			0,
	"cc",			H_RCPT,			0,
	"resent-cc",		H_RCPT,			0,
	"bcc",			H_RCPT|H_ACHECK,	0,
	"resent-bcc",		H_RCPT|H_ACHECK,	0,
		/* message identification and control */
	"message-id",		0,			0,
	"resent-message-id",	0,			0,
	"precedence",		0,			0,
	"message",		H_EOH,			0,
	"text",			H_EOH,			0,
		/* trace fields */
	"received",		H_TRACE|H_FORCE,	0,
	"via",			H_TRACE|H_FORCE,	0,
	"mail-from",		H_TRACE|H_FORCE,	0,

	NULL,			0,			0,
};


/*
**  ARPANET error message numbers.
*/

char	Arpa_Info[] =		"050";	/* arbitrary info */
char	Arpa_TSyserr[] =	"451";	/* some (transient) system error */
char	Arpa_PSyserr[] =	"554";	/* some (permanent) system error */
char	Arpa_Usrerr[] =		"554";	/* some (fatal) user error */





/*
**  Location of system files/databases/etc.
*/

char	*ConfFile =	"/usr/lib/sendmail.cf";	/* runtime configuration */
char	*XcriptFile =	"/tmp/mailxXXXXXX";	/* template for transcript */

# ifdef V6
/*
**  TTYNAME -- return name of terminal.
**
**	Parameters:
**		fd -- file descriptor to check.
**
**	Returns:
**		pointer to full path of tty.
**		NULL if no tty.
**
**	Side Effects:
**		none.
*/

char *
ttyname(fd)
	int fd;
{
	register char tn;
	static char pathn[] = "/dev/ttyx";

	/* compute the pathname of the controlling tty */
	if ((tn = ttyn(fd)) == NULL)
	{
		errno = 0;
		return (NULL);
	}
	pathn[8] = tn;
	return (pathn);
}
/*
**  FDOPEN -- Open a stdio file given an open file descriptor.
**
**	This is included here because it is standard in v7, but we
**	need it in v6.
**
**	Algorithm:
**		Open /dev/null to create a descriptor.
**		Close that descriptor.
**		Copy the existing fd into the descriptor.
**
**	Parameters:
**		fd -- the open file descriptor.
**		type -- "r", "w", or whatever.
**
**	Returns:
**		The file descriptor it creates.
**
**	Side Effects:
**		none
**
**	Called By:
**		deliver
**
**	Notes:
**		The mode of fd must match "type".
*/

FILE *
fdopen(fd, type)
	int fd;
	char *type;
{
	register FILE *f;

	f = fopen("/dev/null", type);
	(void) close(fileno(f));
	fileno(f) = fd;
	return (f);
}
/*
**  INDEX -- Return pointer to character in string
**
**	For V7 compatibility.
**
**	Parameters:
**		s -- a string to scan.
**		c -- a character to look for.
**
**	Returns:
**		If c is in s, returns the address of the first
**			instance of c in s.
**		NULL if c is not in s.
**
**	Side Effects:
**		none.
*/

char *
index(s, c)
	register char *s;
	register char c;
{
	while (*s != '\0')
	{
		if (*s++ == c)
			return (--s);
	}
	return (NULL);
}
/*
**  UMASK -- fake the umask system call.
**
**	Since V6 always acts like the umask is zero, we will just
**	assume the same thing.
*/

/*ARGSUSED*/
umask(nmask)
{
	return (0);
}


/*
**  GETRUID -- get real user id.
*/

getruid()
{
	return (getuid() & 0377);
}


/*
**  GETRGID -- get real group id.
*/

getrgid()
{
	return (getgid() & 0377);
}


/*
**  GETEUID -- get effective user id.
*/

geteuid()
{
	return ((getuid() >> 8) & 0377);
}


/*
**  GETEGID -- get effective group id.
*/

getegid()
{
	return ((getgid() >> 8) & 0377);
}

# endif V6

# ifndef V6

/*
**  GETRUID -- get real user id (V7)
*/

getruid()
{
	if (Mode == MD_DAEMON)
		return (RealUid);
	else
		return (getuid());
}


/*
**  GETRGID -- get real group id (V7).
*/

getrgid()
{
	if (Mode == MD_DAEMON)
		return (RealGid);
	else
		return (getgid());
}

# endif V6
/*
**  TTYPATH -- Get the path of the user's tty
**
**	Returns the pathname of the user's tty.  Returns NULL if
**	the user is not logged in or if s/he has write permission
**	denied.
**
**	Parameters:
**		none
**
**	Returns:
**		pathname of the user's tty.
**		NULL if not logged in or write permission denied.
**
**	Side Effects:
**		none.
**
**	WARNING:
**		Return value is in a local buffer.
**
**	Called By:
**		savemail
*/

# include <sys/stat.h>

char *
ttypath()
{
	struct stat stbuf;
	register char *pathn;
	extern char *ttyname();
	extern char *getlogin();

	/* compute the pathname of the controlling tty */
	if ((pathn = ttyname(2)) == NULL && (pathn = ttyname(1)) == NULL && (pathn = ttyname(0)) == NULL)
	{
		errno = 0;
		return (NULL);
	}

	/* see if we have write permission */
	if (stat(pathn, &stbuf) < 0 || !bitset(02, stbuf.st_mode))
	{
		errno = 0;
		return (NULL);
	}

	/* see if the user is logged in */
	if (getlogin() == NULL)
		return (NULL);

	/* looks good */
	return (pathn);
}
/*
**  CHECKCOMPAT -- check for From and To person compatible.
**
**	This routine can be supplied on a per-installation basis
**	to determine whether a person is allowed to send a message.
**	This allows restriction of certain types of internet
**	forwarding or registration of users.
**
**	If the hosts are found to be incompatible, an error
**	message should be given using "usrerr" and FALSE should
**	be returned.
**
**	'NoReturn' can be set to suppress the return-to-sender
**	function; this should be done on huge messages.
**
**	Parameters:
**		to -- the person being sent to.
**
**	Returns:
**		TRUE -- ok to send.
**		FALSE -- not ok.
**
**	Side Effects:
**		none (unless you include the usrerr stuff)
*/

bool
checkcompat(to)
	register ADDRESS *to;
{
# ifdef ING70
	register STAB *s;
# endif ING70

	if (to->q_mailer != LocalMailer && CurEnv->e_msgsize > 100000)
	{
		usrerr("Message exceeds 100000 bytes");
		NoReturn++;
		return (FALSE);
	}
# ifdef ING70
	s = stab("arpa", ST_MAILER, ST_FIND);
	if (s != NULL && CurEnv->e_from.q_mailer != LocalMailer && to->q_mailer == s->s_mailer)
	{
		usrerr("No ARPA mail through this machine: see your system administration");
		return (FALSE);
	}
# endif ING70
	return (TRUE);
}
