# @(#)mdate.s	4.1 (Berkeley) 12/21/80
# C library-- mdate

	.set	mdate,30
.globl	_mdate
.globl  cerror

_mdate:
	.word	0x0000
	chmk	$mdate
	bcc 	noerror
	jmp 	cerror
noerror:
	clrl	r0
	ret
