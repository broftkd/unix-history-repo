#include "../api/api.h"

#include "apilib.h"

int
    api_sup_errno = 0,			/* Supervisor error number */
    api_sup_fcn_id = 0,			/* Supervisor function id (0x12) */
    api_fcn_errno = 0,			/* Function error number */
    api_fcn_fcn_id = 0;			/* Function ID (0x6b, etc.) */

static int
    gate_sessmgr = 0,
    gate_keyboard = 0,
    gate_copy = 0,
    gate_oiam = 0;

/*
 * Issue an API request, with reg structures supplied by the caller.
 *
 * Only certain routines need this (supervisor services come to mind).
 */

int
api_issue_regs(ah, al, bh, bl, cx, dx, parms, regs, sregs)
int		ah, al, bh, bl, cx, dx;
char 		*parms;
union REGS 	*regs;
struct SREGS 	*sregs;
{
    char far *ourseg = parms;

    regs->h.ah = ah;
    regs->h.al = al;
    regs->h.bh = bh;
    regs->h.bl = bl;
    regs->x.cx = cx;
    regs->x.dx = dx;
    sregs->es = (int) FP_SEG(ourseg);
    regs->x.di = (int) FP_OFF(ourseg);

    int86x(API_INTERRUPT_NUMBER, regs, regs, sregs);
    if (regs->h.cl != 0) {
	api_sup_errno = regs->h.cl;
	return -1;
    } else {
	return 0;
    }
}


/*
 * Issue an API request without requiring caller to supply
 * registers.  Most routines use this.
 */

int
api_issue(ah, al, bh, bl, cx, dx, parms)
int
    ah,
    al,
    bh,
    bl,
    cx,
    dx;
char *parms;
{
    union REGS regs;
    struct SREGS sregs;

    return api_issue_regs(ah, al, bh, bl, cx, dx, parms, &regs, &sregs);
}

/*
 * Supervisor Services
 */

int
api_name_resolve(name)
char *name;
{
    NameResolveParms parms;
    int i;
    union REGS regs;
    struct SREGS sregs;

    for (i = 0; i < sizeof parms.gate_name; i++) {
	if (*name) {
	    parms.gate_name[i] = *name++;
	} else {
	    parms.gate_name[i] = ' ';
	}
    }

    if (api_issue_regs(NAME_RESOLUTION, 0, 0, 0, 0, 0, &parms, &regs, &sregs)
		    == -1) {
	return -1;
    } else {
	return regs.x.dx;
    }
}

/*
 * Session Information Services
 */

api_query_sesion_id(parms)
QuerySessionIdParms *parms;
{
    if (api_issue(0x09, QUERY_SESSION_ID, 0x80, 0x20, 0,
					gate_sessmgr, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}


api_query_session_parms(parms)
QuerySessionParametersParms *parms;
{
    if (api_issue(0x09, QUERY_SESSION_PARMS, 0x80, 0x20, 0,
					gate_sessmgr, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

api_query_session_cursor(parms)
QuerySessionCursorParms *parms;
{
    if (api_issue(0x09, QUERY_SESSION_CURSOR, 0x80, 0x20, 0,
					gate_sessmgr, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

/*
 * Keyboard Services
 */

api_connect_to_keyboard(parms)
ConnectToKeyboardParms *parms;
{
    if (api_issue(0x09, CONNECT_TO_KEYBOARD, 0x80, 0x20, 0,
					gate_keyboard, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}


api_disconnect_from_keyboard(parms)
DisconnectFromKeyboardParms *parms;
{
    if (api_issue(0x09, DISCONNECT_FROM_KEYBOARD, 0x80, 0x20, 0,
					gate_keyboard, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}


api_write_keystroke(parms)
WriteKeystrokeParms *parms;
{
    if (api_issue(0x09, WRITE_KEYSTROKE, 0x80, 0x20, 0,
					gate_keyboard, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}


api_disable_input(parms)
DisableInputParms *parms;
{
    if (api_issue(0x09, DISABLE_INPUT, 0x80, 0x20, 0,
					gate_keyboard, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

api_enable_input(parms)
EnableInputParms *parms;
{
    if (api_issue(0x09, ENABLE_INPUT, 0x80, 0x20, 0,
					gate_keyboard, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

/*
 * Copy Services
 */

api_copy_string(parms)
CopyStringParms *parms;
{
    if (api_issue(0x09, COPY_STRING, 0x80, 0x20, 0,
					    gate_copy, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

/*
 * Operator Information Area Services
 */

api_read_oia_group(parms)
ReadOiaGroupParms *parms;
{
    if (api_issue(0x09, READ_OIA_GROUP, 0x80, 0x20, 0,
					    gate_oiam, (char *)parms) == -1) {
	api_fcn_errno = 0;
	api_fcn_fcn_id = 0;
	return -1;
    } else if (parms->rc == 0) {
	return 0;
    } else {
	api_fcn_errno = parms->rc;
	api_fcn_fcn_id = parms->function_id;
	return -1;
    }
}

/*
 * The initialization routine.  Be sure to call this first.
 */

api_init()
{
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x35;
    regs.h.al = API_INTERRUPT_NUMBER;
    intdosx(&regs, &regs, &sregs);

    if ((regs.x.bx == 0) && (sregs.es == 0)) {
	return 0;		/* Interrupt not being handled */
    }

    gate_sessmgr = api_name_resolve("SESSMGR");
    gate_keyboard = api_name_resolve("KEYBOARD");
    gate_copy = api_name_resolve("COPY");
    gate_oiam = api_name_resolve("OIAM");

    if ((gate_sessmgr == gate_keyboard) ||
	(gate_sessmgr == gate_copy) ||
	(gate_sessmgr == gate_oiam) ||
	(gate_keyboard == gate_copy) ||
	(gate_keyboard == gate_oiam) ||
	(gate_copy == gate_oiam)) {
	    return 0;		/* Interrupt doesn't seem correct */
    }
    return 1;
}
