/*
 * $Id: mount_xdr.c,v 5.2.1.1 90/10/21 22:30:21 jsp Exp $
 *
 * Copyright (c) 1989 Jan-Simon Pendry
 * Copyright (c) 1989 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)mount_xdr.c	5.2 (Berkeley) %G%
 */

#include "am.h"
#include "mount.h"


bool_t
xdr_fhandle(xdrs, objp)
	XDR *xdrs;
	fhandle objp;
{
	if (!xdr_opaque(xdrs, objp, FHSIZE)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_fhstatus(xdrs, objp)
	XDR *xdrs;
	fhstatus *objp;
{
	if (!xdr_u_int(xdrs, &objp->fhs_status)) {
		return (FALSE);
	}
	switch (objp->fhs_status) {
	case 0:
		if (!xdr_fhandle(xdrs, objp->fhstatus_u.fhs_fhandle)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_dirpath(xdrs, objp)
	XDR *xdrs;
	dirpath *objp;
{
	if (!xdr_string(xdrs, objp, MNTPATHLEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_name(xdrs, objp)
	XDR *xdrs;
	name *objp;
{
	if (!xdr_string(xdrs, objp, MNTNAMLEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_mountlist(xdrs, objp)
	XDR *xdrs;
	mountlist *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct mountbody), xdr_mountbody)) {
		return (FALSE);
	}
	return (TRUE);
}



bool_t
xdr_mountbody(xdrs, objp)
	XDR *xdrs;
	mountbody *objp;
{
	if (!xdr_name(xdrs, &objp->ml_hostname)) {
		return (FALSE);
	}
	if (!xdr_dirpath(xdrs, &objp->ml_directory)) {
		return (FALSE);
	}
	if (!xdr_mountlist(xdrs, &objp->ml_next)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_groups(xdrs, objp)
	XDR *xdrs;
	groups *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct groupnode), xdr_groupnode)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_groupnode(xdrs, objp)
	XDR *xdrs;
	groupnode *objp;
{
	if (!xdr_name(xdrs, &objp->gr_name)) {
		return (FALSE);
	}
	if (!xdr_groups(xdrs, &objp->gr_next)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_exports(xdrs, objp)
	XDR *xdrs;
	exports *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct exportnode), xdr_exportnode)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_exportnode(xdrs, objp)
	XDR *xdrs;
	exportnode *objp;
{
	if (!xdr_dirpath(xdrs, &objp->ex_dir)) {
		return (FALSE);
	}
	if (!xdr_groups(xdrs, &objp->ex_groups)) {
		return (FALSE);
	}
	if (!xdr_exports(xdrs, &objp->ex_next)) {
		return (FALSE);
	}
	return (TRUE);
}


