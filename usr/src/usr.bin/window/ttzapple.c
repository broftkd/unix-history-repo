/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)ttzapple.c	3.12 (Berkeley) %G%";
#endif /* not lint */

#include "ww.h"
#include "tt.h"
#include "char.h"

/*
zz|zapple|perfect apple:\
	:am:pt:co#80:li#24:le=^H:nd=^F:up=^K:do=^J:\
	:ho=\E0:ll=\E1:cm=\E=%+ %+ :ch=\E<%+ :cv=\E>%+ :\
	:cl=\E4:ce=\E2:cd=\E3:rp=\E@%.%+ :\
	:so=\E+:se=\E-:\
	:dc=\Ec:DC=\EC%+ :ic=\Ei:IC=\EI%+ :\
	:al=\Ea:AL=\EA%+ :dl=\Ed:DL=\ED%+ :\
	:sf=\Ef:SF=\EF%+ :sr=\Er:SR=\ER%+ :cs=\E?%+ %+ :\
	:is=\E-\ET :
*/

#define NCOL		80
#define NROW		24
#define TOKEN_MAX	32

extern short gen_frame[];

	/* for error correction */
int zz_ecc;

zz_setmodes(new)
{
	if (new & WWM_REV) {
		if ((tt.tt_modes & WWM_REV) == 0)
			ttesc('+');
	} else
		if (tt.tt_modes & WWM_REV)
			ttesc('-');
	tt.tt_modes = new;
}

zz_insline(n)
{
	if (n == 1)
		ttesc('a');
	else {
		ttesc('A');
		ttputc(n + ' ');
	}
}

zz_delline(n)
{
	if (n == 1)
		ttesc('d');
	else {
		ttesc('D');
		ttputc(n + ' ');
	}
}

zz_putc(c)
	char c;
{
	if (tt.tt_nmodes != tt.tt_modes)
		zz_setmodes(tt.tt_nmodes);
	ttputc(c);
	if (++tt.tt_col == NCOL)
		tt.tt_col = 0, tt.tt_row++;
}

zz_write(p, n)
	register char *p;
	register n;
{
	if (tt.tt_nmodes != tt.tt_modes)
		zz_setmodes(tt.tt_nmodes);
	ttwrite(p, n);
	tt.tt_col += n;
	if (tt.tt_col == NCOL)
		tt.tt_col = 0, tt.tt_row++;
}

zz_move(row, col)
	register row, col;
{
	register x;

	if (tt.tt_row == row) {
same_row:
		if ((x = col - tt.tt_col) == 0)
			return;
		if (col == 0) {
			ttctrl('m');
			goto out;
		}
		switch (x) {
		case 2:
			ttctrl('f');
		case 1:
			ttctrl('f');
			goto out;
		case -2:
			ttctrl('h');
		case -1:
			ttctrl('h');
			goto out;
		}
		if ((col & 7) == 0 && x > 0 && x <= 16) {
			ttctrl('i');
			if (x > 8)
				ttctrl('i');
			goto out;
		}
		ttesc('<');
		ttputc(col + ' ');
		goto out;
	}
	if (tt.tt_col == col) {
		switch (row - tt.tt_row) {
		case 2:
			ttctrl('j');
		case 1:
			ttctrl('j');
			goto out;
		case -2:
			ttctrl('k');
		case -1:
			ttctrl('k');
			goto out;
		}
		if (col == 0) {
			if (row == 0)
				goto home;
			if (row == NROW - 1)
				goto ll;
		}
		ttesc('>');
		ttputc(row + ' ');
		goto out;
	}
	if (col == 0) {
		if (row == 0) {
home:
			ttesc('0');
			goto out;
		}
		if (row == tt.tt_row + 1) {
			/*
			 * Do newline first to match the sequence
			 * for scroll down and return
			 */
			ttctrl('j');
			ttctrl('m');
			goto out;
		}
		if (row == NROW - 1) {
ll:
			ttesc('1');
			goto out;
		}
	}
	/* favor local motion for better compression */
	if (row == tt.tt_row + 1) {
		ttctrl('j');
		goto same_row;
	}
	if (row == tt.tt_row - 1) {
		ttctrl('k');
		goto same_row;
	}
	ttesc('=');
	ttputc(' ' + row);
	ttputc(' ' + col);
out:
	tt.tt_col = col;
	tt.tt_row = row;
}

zz_start()
{
	zz_setmodes(0);
	zz_setscroll(0, NROW - 1);
	zz_clear();
	ttesc('T');
	ttputc(TOKEN_MAX + ' ');
	ttesc('U');
	ttputc('!');
	zz_ecc = 1;
}

zz_end()
{
	ttesc('T');
	ttputc(' ');
	ttesc('U');
	ttputc(' ');
	zz_ecc = 0;
}

zz_clreol()
{
	ttesc('2');
}

zz_clreos()
{
	ttesc('3');
}

zz_clear()
{
	ttesc('4');
	tt.tt_col = tt.tt_row = 0;
}

zz_insspace(n)
{
	if (n == 1)
		ttesc('i');
	else {
		ttesc('I');
		ttputc(n + ' ');
	}
}

zz_delchar(n)
{
	if (n == 1)
		ttesc('c');
	else {
		ttesc('C');
		ttputc(n + ' ');
	}
}

zz_scroll_down(n)
{
	if (n == 1)
		if (tt.tt_row == NROW - 1)
			ttctrl('j');
		else
			ttesc('f');
	else {
		ttesc('F');
		ttputc(n + ' ');
	}
}

zz_scroll_up(n)
{
	if (n == 1)
		ttesc('r');
	else {
		ttesc('R');
		ttputc(n + ' ');
	}
}

zz_setscroll(top, bot)
{
	ttesc('?');
	ttputc(top + ' ');
	ttputc(bot + ' ');
	tt.tt_scroll_top = top;
	tt.tt_scroll_bot = bot;
}

int zz_debug = 0;

zz_set_token(t, s, n)
	char *s;
{
	if (tt.tt_nmodes != tt.tt_modes)
		zz_setmodes(tt.tt_nmodes);
	if (zz_debug) {
		char buf[100];
		zz_setmodes(WWM_REV);
		(void) sprintf(buf, "%02x=", t);
		ttputs(buf);
		tt.tt_col += 3;
	}
	ttputc(0x80);
	ttputc(t + 1);
	s[n - 1] |= 0x80;
	ttwrite(s, n);
	s[n - 1] &= ~0x80;
}

/*ARGSUSED*/
zz_put_token(t, s, n)
	char *s;
{
	if (tt.tt_nmodes != tt.tt_modes)
		zz_setmodes(tt.tt_nmodes);
	if (zz_debug) {
		char buf[100];
		zz_setmodes(WWM_REV);
		(void) sprintf(buf, "%02x>", t);
		ttputs(buf);
		tt.tt_col += 3;
	}
	ttputc(t + 0x81);
}

zz_rint(p, n)
	char *p;
{
	static lastc;
	register i;
	register char *q;

	if (!zz_ecc)
		return n;
	for (i = n, q = p; --i >= 0;) {
		register c = (unsigned char) *p++;
		if (zz_ecc == 1) {
			if (c)
				*q++ = c;
			else {
				zz_ecc = 2;
				lastc = -1;
			}
		} else {
			if (lastc < 0) {
				lastc = c;
			} else if (lastc == c) {
				*q++ = lastc;
				lastc = -1;
			} else {
				wwnreadec++;
				lastc = c;
			}
		}
	}
	return q - (p - n);
}

tt_zapple()
{
	tt.tt_insspace = zz_insspace;
	tt.tt_delchar = zz_delchar;
	tt.tt_insline = zz_insline;
	tt.tt_delline = zz_delline;
	tt.tt_clreol = zz_clreol;
	tt.tt_clreos = zz_clreos;
	tt.tt_scroll_down = zz_scroll_down;
	tt.tt_scroll_up = zz_scroll_up;
	tt.tt_setscroll = zz_setscroll;
	tt.tt_availmodes = WWM_REV;
	tt.tt_wrap = 1;
	tt.tt_retain = 0;
	tt.tt_ncol = NCOL;
	tt.tt_nrow = NROW;
	tt.tt_start = zz_start;
	tt.tt_end = zz_end;
	tt.tt_write = zz_write;
	tt.tt_putc = zz_putc;
	tt.tt_move = zz_move;
	tt.tt_clear = zz_clear;
	tt.tt_setmodes = zz_setmodes;
	tt.tt_frame = gen_frame;
	tt.tt_padc = TT_PADC_NONE;
	tt.tt_ntoken = 127;
	tt.tt_set_token = zz_set_token;
	tt.tt_put_token = zz_put_token;
	tt.tt_token_min = 1;
	tt.tt_token_max = TOKEN_MAX;
	tt.tt_set_token_cost = 2;
	tt.tt_put_token_cost = 1;
	tt.tt_rint = zz_rint;
	return 0;
}
