/*-
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#ifndef lint
static char sccsid[] = "@(#)refer4.c	4.2 (Berkeley) 4/18/91";
#endif /* not lint */

#include "refer..c"
#define punctuat(c) (c=='.' || c=='?' || c=='!' || c==',' || c==';' || c==':')

static gate = 0;
static char buff[BUFSIZ];

output(s)
char *s;
{
	if (gate)
		fputs(buff,ftemp);
	else
		gate = 1;
	strcpy(buff, s);
	if (strlen(buff) > BUFSIZ)
		err("one buff too big (%d)!", BUFSIZ);
}

append(s)
char *s;
{
	char *p;
	int lch;

	trimnl(buff);
	for (p = buff; *p; p++)
		;
	lch = *--p;
	if (postpunct && punctuat(lch))
		*p = NULL;
	else /* pre-punctuation */
		switch (lch) {
		case '.': 
		case '?':
		case '!':
		case ',':
		case ';':
		case ':':
			*p++ = lch;
			*p = NULL;
		}
	strcat(buff, s);
	if (postpunct)
		switch(lch) {
		case '.': 
		case '?':
		case '!':
		case ',':
		case ';':
		case ':':
			for(p = buff; *p; p++)
				;
			if (*--p == '\n')
				*p = NULL;
			*p++ = lch;
			*p++ = '\n';
			*p = NULL;
		}
	if (strlen(buff) > BUFSIZ)
		err("output buff too long (%d)", BUFSIZ);
}

flout()
{
	if (gate)
		fputs(buff,ftemp);
	gate = 0;
}

char *
trimnl(ln)
char *ln;
{
	register char *p = ln;

	while (*p)
		p++;
	p--;
	if (*p == '\n')
		*p = 0;
	return(ln);
}
