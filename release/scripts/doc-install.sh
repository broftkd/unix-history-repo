#!/bin/sh
#
if [ "`id -u`" != "0" ]; then
	echo "Sorry, this must be done as root."
	exit 1
fi
echo "You are about to extract the doc distribution into / - are you SURE"
echo -n "you want to do this over your installed system (y/n)? "
read ans
if [ "$ans" = "y" ]; then
	cat doc.?? | tar --unlink -xpzf - -C /
fi
