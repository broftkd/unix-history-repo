#include <pwd.h>

#define	BUFSIZ	160

static int pwf = -1;
static char line[BUFSIZ+1];
static struct passwd passwd;

setpwent()
{
	if( pwf == -1 )
		pwf = open( "/etc/passwd", 0 );
	else
		lseek(pwf, 0l, 0);
}

endpwent()
{
	if( pwf != -1 ){
		close( pwf );
		pwf = -1;
	}
}

static char *
pwskip(p)
register char *p;
{
	while( *p && *p != ':' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}

struct passwd *
getpwent()
{
	register char *p;
	register int i, j;

	if (pwf == -1) {
		if( (pwf = open( "/etc/passwd", 0 )) == -1 )
			return(0);
	}
	i = read(pwf, line, BUFSIZ);
	for (j = 0; j < i; j++)
		if (line[j] == '\n')
			break;
	if (j >= i)
		return(0);
	line[++j] = 0;
	lseek(pwf, (long) (j - i), 1);
	p = line;
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
/* For Cory, e.g. only
	passwd.pw_uid =+ atoi(p) << 8;
 */
	passwd.pw_quota = 0;
	passwd.pw_comment = "";
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
 	while(*p && *p != '\n') p++;
	*p = '\0';
	return(&passwd);
}
