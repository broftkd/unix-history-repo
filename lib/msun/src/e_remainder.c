/* @(#)e_remainder.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#ifndef lint
static char rcsid[] = "$Id: e_remainder.c,v 1.4 1994/03/03 17:04:20 jtc Exp $";
#endif

/* __ieee754_remainder(x,p)
 * Return :                  
 * 	returns  x REM p  =  x - [x/p]*p as if in infinite 
 * 	precise arithmetic, where [x/p] is the (infinite bit) 
 *	integer nearest x/p (in half way case choose the even one).
 * Method : 
 *	Based on fmod() return x-[x/p]chopped*p exactlp.
 */

#include "math.h"
#include <machine/endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define n0	1
#define n1	0
#else
#define n0	0
#define n1	1
#endif

#ifdef __STDC__
static const double zero = 0.0;
#else
static double zero = 0.0;
#endif


#ifdef __STDC__
	double __ieee754_remainder(double x, double p)
#else
	double __ieee754_remainder(x,p)
	double x,p;
#endif
{
	int hx,hp;
	unsigned sx,lx,lp;
	double p_half;

	hx = *( n0 + (int*)&x);		/* high word of x */
	lx = *( n1 + (int*)&x);		/* low  word of x */
	hp = *( n0 + (int*)&p);		/* high word of p */
	lp = *( n1 + (int*)&p);		/* low  word of p */
	sx = hx&0x80000000;
	hp &= 0x7fffffff;
	hx &= 0x7fffffff;

    /* purge off exception values */
	if((hp|lp)==0) return (x*p)/(x*p); 	/* p = 0 */
	if((hx>=0x7ff00000)||			/* x not finite */
	  ((hp>=0x7ff00000)&&			/* p is NaN */
	  (((hp-0x7ff00000)|lp)!=0)))
	    return (x*p)/(x*p);


	if (hp<=0x7fdfffff) x = __ieee754_fmod(x,p+p);	/* now x < 2p */
	if (((hx-hp)|(lx-lp))==0) return zero*x;
	x  = fabs(x);
	p  = fabs(p);
	if (hp<0x00200000) {
	    if(x+x>p) {
		x-=p;
		if(x+x>=p) x -= p;
	    }
	} else {
	    p_half = 0.5*p;
	    if(x>p_half) {
		x-=p;
		if(x>=p_half) x -= p;
	    }
	}
	*(n0+(int*)&x) ^= sx;
	return x;
}
