#!/bin/sh -
#
# Copyright (c) 1991, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.proprietary.sh%
#
#	@(#)diff3.sh	8.1 (Berkeley) %G%
#

e=
case $1 in
-*)
	e=$1
	shift;;
esac
if test $# = 3 -a -f $1 -a -f $2 -a -f $3
then
	:
else
	echo usage: diff3 file1 file2 file3 1>&2
	exit
fi
trap "rm -f /tmp/d3[ab]$$" 0 1 2 13 15
diff $1 $3 >/tmp/d3a$$
diff $2 $3 >/tmp/d3b$$
/usr/libexec/diff3 $e /tmp/d3[ab]$$ $1 $2 $3
