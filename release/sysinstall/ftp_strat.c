/*
 * The new sysinstall program.
 *
 * This is probably the last attempt in the `sysinstall' line, the next
 * generation being slated to essentially a complete rewrite.
 *
 * $Id: ftp_strat.c,v 1.6.2.2 1995/06/01 21:03:57 jkh Exp $
 *
 * Copyright (c) 1995
 *	Jordan Hubbard.  All rights reserved.
 * Copyright (c) 1995
 * 	Gary J Palmer. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    verbatim and that no modifications are made prior to this
 *    point in the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Jordan Hubbard
 *	for the FreeBSD Project.
 * 4. The name of Jordan Hubbard or the FreeBSD project may not be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JORDAN HUBBARD ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JORDAN HUBBARD OR HIS PETS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, LIFE OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "sysinstall.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netdb.h>
#include "ftp.h"

Boolean ftpInitted;
static FTP_t ftp;

#define FTP_USER	"_ftpUser"
#define FTP_PASS	"_ftpPass"

int
mediaSetFtpUserPass(char *str)
{
    char *user;
    char *pass;

    dialog_clear();
    user = msgGetInput(getenv(FTP_USER), "Please enter the username you wish to login as");
    pass = msgGetInput(getenv(FTP_PASS), "Please enter the password for this user.\nWARNING: This password will echo on the screen!");
    dialog_clear();
    if (user)
	variable_set2(FTP_USER, user);
    if (pass)
	variable_set2(FTP_PASS, pass);
    return 0;
}

Boolean
mediaInitFTP(Device *dev)
{
    int i;
    char *cp, *hostname, *dir;
    char *login_name, password[80], url[BUFSIZ];
    Device *netDevice = (Device *)dev->private;

    if (ftpInitted)
	return TRUE;

    if (netDevice->init) {
	if (isDebug())
	    msgDebug("InitFTP: Calling network init routine\n");
	if (!(*netDevice->init)(netDevice))
	    return FALSE;
    }

    if ((ftp = FtpInit()) == NULL) {
	msgConfirm("FTP initialisation failed!");
	return FALSE;
    }
    if (isDebug())
	msgDebug("Initialized FTP library.\n");

    cp = getenv("ftp");
    if (!cp)
	return FALSE;
    if (isDebug())
	msgDebug("Attempting to open connection for: %s\n", cp);
    hostname = getenv(VAR_HOSTNAME);
    if (strncmp("ftp://", cp, 6) != NULL) {
	msgConfirm("Invalid URL: %s\n(A URL must start with `ftp://' here)", cp);
	return FALSE;
    }
    strncpy(url, cp, BUFSIZ);
    if (isDebug())
	msgDebug("Using URL `%s'\n", url);
    hostname = url + 6;
    if ((dir = index(hostname, '/')) != NULL)
	*(dir++) = '\0';
    if (isDebug()) {
	msgDebug("hostname = `%s'\n", hostname);
	msgDebug("dir = `%s'\n", dir ? dir : "/");
    }
    msgNotify("Looking up host %s..", hostname);
    if ((gethostbyname(hostname) == NULL) && (inet_addr(hostname) == INADDR_NONE)) {
	msgConfirm("Cannot resolve hostname `%s'!  Are you sure your name server\nand/or gateway values are set properly?", hostname);
	return FALSE;
    }
    if (!getenv(FTP_USER)) {
	snprintf(password, BUFSIZ, "installer@%s", hostname);
	login_name = "anonymous";
    }
    else {
	login_name = getenv(FTP_USER);
	strcpy(password, getenv(FTP_PASS) ? getenv(FTP_PASS) : login_name);
    }
    msgNotify("Logging in as %s..", login_name);
    if ((i = FtpOpen(ftp, hostname, login_name, password)) != 0) {
	msgConfirm("Couldn't open FTP connection to %s: %s (%u)\n", hostname, strerror(i), i);
	return FALSE;
    }

    if (getenv("ftpPassive"))
	FtpPassive(ftp, 1);
    FtpBinary(ftp, 1);
    if (dir && *dir != '\0') {
	msgNotify("CD to distribution in ~ftp/%s", dir);
	FtpChdir(ftp, dir);
    }
    if (isDebug())
	msgDebug("leaving mediaInitFTP!\n");
    ftpInitted = TRUE;
    return TRUE;
}

int
mediaGetFTP(char *file)
{
    return(FtpGet(ftp, file));
}

Boolean
mediaCloseFTP(Device *dev, int fd)
{
    FtpEOF(ftp);
    return (TRUE);
}

void
mediaShutdownFTP(Device *dev)
{
    Device *netdev = (Device *)dev->private;

    if (!ftpInitted)
	return;

    if (ftp != NULL) {
	FtpClose(ftp);
	ftp = NULL;
    }
    if (netdev->shutdown)
	(*netdev->shutdown)(netdev);
    ftpInitted = FALSE;
}
