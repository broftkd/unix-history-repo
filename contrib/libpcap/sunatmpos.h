/*
 * Copyright (c) 1997 Yen Yen Lim and North Dakota State University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yen Yen Lim and
        North Dakota State University
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @(#) $Header: /tcpdump/master/libpcap/sunatmpos.h,v 1.1 2002/07/11 09:06:47 guy Exp $ (LBL)
 */

/* SunATM header for ATM packet */
#define SUNATM_DIR_POS		0
#define SUNATM_VPI_POS		1
#define SUNATM_VCI_POS		2
#define SUNATM_PKT_BEGIN_POS	4	/* Start of ATM packet */

/* Protocol type values in the bottom for bits of the byte at SUNATM_DIR_POS. */
#define PT_LANE		0x01	/* LANE */
#define PT_LLC		0x02	/* LLC encapsulation */
#define PT_ILMI		0x05	/* ILMI */
#define PT_QSAAL	0x06	/* Q.SAAL */
