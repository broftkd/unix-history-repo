#ifndef lint
static char sccsid[] = "@(#)open.c	4.1 (Berkeley) 6/27/83";
#endif

int xnow;
int ynow;
float boty 0.;
float botx 0.;
float oboty 0.;
float obotx 0.;
float scalex 1.;
float scaley 1.;
int vti -1;

openvt ()
{
		vti = open("/dev/vt0",1);
		return;
}
openpl()
{
	vti = open("/dev/vt0",1);
	return;
}
