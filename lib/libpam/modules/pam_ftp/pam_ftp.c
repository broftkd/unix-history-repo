/*-
 * Copyright (c) 2001 Mark R V Murray
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#define PLEASE_ENTER_PASSWORD "Password required for %s."
#define GUEST_LOGIN_PROMPT "Guest login ok, send your e-mail address as password."

/* the following is a password that "can't be correct" */
#define BLOCK_PASSWORD "\177BAD PASSWPRD\177"

#include <security/_pam_aconf.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>

/* here, we make a definition for the externally accessible function in this
 * file (this definition is required for static a module but strongly
 * encouraged generally) it is used to instruct the modules include file to
 * define the function prototypes. */

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <pam_mod_misc.h>

#include <security/_pam_macros.h>

static int 
converse(pam_handle_t *pamh, int nargs, struct pam_message **message,
	struct pam_response **response)
{
	struct pam_conv *conv;
	int retval;

	retval = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
	if (retval == PAM_SUCCESS)
		retval = conv->conv(nargs, (const struct pam_message **)message,
			response, conv->appdata_ptr);
	return retval;
}

static const char *anonusers[] = {"ftp", "anonymous", NULL};

/* Check if name is in list or default list.
 * Place user's name in *user
 * Return 1 if listed 0 otherwise
 */
static int 
lookup(const char *name, char *list, const char **user)
{
	int anon, i;
	char *item, *context, *locallist;

	anon = 0;
	*user = name;		/* this is the default */
	if (list) {
		locallist = list;
		while ((item = strtok_r(locallist, ",", &context))) {
			locallist = NULL;
			if (strcmp(name, item) == 0) {
				*user = item;
				anon = 1;
				break;
			}
		}
	}
	else {
		for (i = 0; anonusers[i] != NULL; i++) {
			if (strcmp(anonusers[i], name) == 0) {
				*user = anonusers[0];
				anon = 1;
				break;
			}
		}
	}
	return anon;
}

/* --- authentication management functions (only) --- */

/* Check if the user name is 'ftp' or 'anonymous'.
 * If this is the case, set the PAM_RUSER to the entered email address
 * and succeed, otherwise fail.
 */
PAM_EXTERN int 
pam_sm_authenticate(pam_handle_t * pamh, int flags, int argc, const char **argv)
{
	struct pam_message msg[1], *mesg[1];
	struct pam_response *resp;
	int retval, anon, options, i;
	const char *user, *token;
	char *users, *context, *prompt;

	users = prompt = NULL;

	options = 0;
	for (i = 0;  i < argc;  i++)
		pam_std_option(&options, argv[i]);

	retval = pam_get_user(pamh, &user, NULL);
	if (retval != PAM_SUCCESS || user == NULL)
		return PAM_USER_UNKNOWN;

	anon = 0;
	if (!(options & PAM_OPT_NO_ANON))
		anon = lookup(user, users, &user);

	if (anon) {
		retval = pam_set_item(pamh, PAM_USER, (const void *)user);
		if (retval != PAM_SUCCESS || user == NULL)
			return PAM_USER_UNKNOWN;
	}

	/* Require an email address for user's password. */
	if (!anon) {
		prompt = malloc(strlen(PLEASE_ENTER_PASSWORD) + strlen(user));
		if (prompt == NULL)
			return PAM_BUF_ERR;
		else {
			sprintf(prompt, PLEASE_ENTER_PASSWORD, user);
			msg[0].msg = prompt;
		}
	}
	else
		msg[0].msg = GUEST_LOGIN_PROMPT;
	msg[0].msg_style = PAM_PROMPT_ECHO_OFF;
	mesg[0] = &msg[0];

	resp = NULL;
	retval = converse(pamh, 1, mesg, &resp);
	if (prompt) {
		_pam_overwrite(prompt);
		_pam_drop(prompt);
	}

	if (retval != PAM_SUCCESS) {
		if (resp != NULL)
			_pam_drop_reply(resp, 1);
		return retval == PAM_CONV_AGAIN
			? PAM_INCOMPLETE : PAM_AUTHINFO_UNAVAIL;
	}

	if (anon) {
		if (!(options & PAM_OPT_IGNORE)) {
			token = strtok_r(resp->resp, "@", &context);
			pam_set_item(pamh, PAM_RUSER, token);

			if ((token) && (retval == PAM_SUCCESS)) {
				token = strtok_r(NULL, "@", &context);
				pam_set_item(pamh, PAM_RHOST, token);
			}
		}
		retval = PAM_SUCCESS;
	}
	else {
		pam_set_item(pamh, PAM_AUTHTOK, resp->resp);
		retval = PAM_AUTH_ERR;
	}

	if (resp)
		_pam_drop_reply(resp, i);

	return retval;
}

PAM_EXTERN int 
pam_sm_setcred(pam_handle_t * pamh, int flags, int argc, const char **argv)
{
	return PAM_IGNORE;
}

/* end of module definition */

PAM_MODULE_ENTRY("pam_ftp");
