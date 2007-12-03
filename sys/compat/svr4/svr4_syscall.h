/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD: src/sys/compat/svr4/syscalls.master,v 1.17.2.3 2007/12/03 21:53:23 jhb Exp 
 */

#define	SVR4_SYS_exit	1
#define	SVR4_SYS_fork	2
#define	SVR4_SYS_read	3
#define	SVR4_SYS_write	4
#define	SVR4_SYS_svr4_sys_open	5
#define	SVR4_SYS_close	6
#define	SVR4_SYS_svr4_sys_wait	7
#define	SVR4_SYS_svr4_sys_creat	8
#define	SVR4_SYS_link	9
#define	SVR4_SYS_unlink	10
#define	SVR4_SYS_svr4_sys_execv	11
#define	SVR4_SYS_chdir	12
#define	SVR4_SYS_svr4_sys_time	13
#define	SVR4_SYS_svr4_sys_mknod	14
#define	SVR4_SYS_chmod	15
#define	SVR4_SYS_chown	16
#define	SVR4_SYS_svr4_sys_break	17
#define	SVR4_SYS_svr4_sys_stat	18
#define	SVR4_SYS_lseek	19
#define	SVR4_SYS_getpid	20
#define	SVR4_SYS_setuid	23
#define	SVR4_SYS_getuid	24
#define	SVR4_SYS_svr4_sys_alarm	27
#define	SVR4_SYS_svr4_sys_fstat	28
#define	SVR4_SYS_svr4_sys_pause	29
#define	SVR4_SYS_svr4_sys_utime	30
#define	SVR4_SYS_svr4_sys_access	33
#define	SVR4_SYS_svr4_sys_nice	34
#define	SVR4_SYS_sync	36
#define	SVR4_SYS_svr4_sys_kill	37
#define	SVR4_SYS_svr4_sys_pgrpsys	39
#define	SVR4_SYS_dup	41
#define	SVR4_SYS_pipe	42
#define	SVR4_SYS_svr4_sys_times	43
#define	SVR4_SYS_setgid	46
#define	SVR4_SYS_getgid	47
#define	SVR4_SYS_svr4_sys_signal	48
#define	SVR4_SYS_svr4_sys_msgsys	49
#define	SVR4_SYS_svr4_sys_sysarch	50
#define	SVR4_SYS_svr4_sys_shmsys	52
#define	SVR4_SYS_svr4_sys_semsys	53
#define	SVR4_SYS_svr4_sys_ioctl	54
#define	SVR4_SYS_svr4_sys_utssys	57
#define	SVR4_SYS_fsync	58
#define	SVR4_SYS_svr4_sys_execve	59
#define	SVR4_SYS_umask	60
#define	SVR4_SYS_chroot	61
#define	SVR4_SYS_svr4_sys_fcntl	62
#define	SVR4_SYS_svr4_sys_ulimit	63
#define	SVR4_SYS_rmdir	79
#define	SVR4_SYS_mkdir	80
#define	SVR4_SYS_svr4_sys_getdents	81
#define	SVR4_SYS_svr4_sys_getmsg	85
#define	SVR4_SYS_svr4_sys_putmsg	86
#define	SVR4_SYS_svr4_sys_poll	87
#define	SVR4_SYS_svr4_sys_lstat	88
#define	SVR4_SYS_symlink	89
#define	SVR4_SYS_readlink	90
#define	SVR4_SYS_getgroups	91
#define	SVR4_SYS_setgroups	92
#define	SVR4_SYS_fchmod	93
#define	SVR4_SYS_fchown	94
#define	SVR4_SYS_svr4_sys_sigprocmask	95
#define	SVR4_SYS_svr4_sys_sigsuspend	96
#define	SVR4_SYS_svr4_sys_sigaltstack	97
#define	SVR4_SYS_svr4_sys_sigaction	98
#define	SVR4_SYS_svr4_sys_sigpending	99
#define	SVR4_SYS_svr4_sys_context	100
#define	SVR4_SYS_svr4_sys_statvfs	103
#define	SVR4_SYS_svr4_sys_fstatvfs	104
#define	SVR4_SYS_svr4_sys_waitsys	107
#define	SVR4_SYS_svr4_sys_hrtsys	109
#define	SVR4_SYS_svr4_sys_pathconf	113
#define	SVR4_SYS_svr4_sys_mmap	115
#define	SVR4_SYS_mprotect	116
#define	SVR4_SYS_munmap	117
#define	SVR4_SYS_svr4_sys_fpathconf	118
#define	SVR4_SYS_vfork	119
#define	SVR4_SYS_fchdir	120
#define	SVR4_SYS_readv	121
#define	SVR4_SYS_writev	122
#define	SVR4_SYS_svr4_sys_xstat	123
#define	SVR4_SYS_svr4_sys_lxstat	124
#define	SVR4_SYS_svr4_sys_fxstat	125
#define	SVR4_SYS_svr4_sys_xmknod	126
#define	SVR4_SYS_svr4_sys_setrlimit	128
#define	SVR4_SYS_svr4_sys_getrlimit	129
#define	SVR4_SYS_lchown	130
#define	SVR4_SYS_svr4_sys_memcntl	131
#define	SVR4_SYS_rename	134
#define	SVR4_SYS_svr4_sys_uname	135
#define	SVR4_SYS_setegid	136
#define	SVR4_SYS_svr4_sys_sysconfig	137
#define	SVR4_SYS_adjtime	138
#define	SVR4_SYS_svr4_sys_systeminfo	139
#define	SVR4_SYS_seteuid	141
#define	SVR4_SYS_svr4_sys_fchroot	153
#define	SVR4_SYS_svr4_sys_utimes	154
#define	SVR4_SYS_svr4_sys_vhangup	155
#define	SVR4_SYS_svr4_sys_gettimeofday	156
#define	SVR4_SYS_getitimer	157
#define	SVR4_SYS_setitimer	158
#define	SVR4_SYS_svr4_sys_llseek	175
#define	SVR4_SYS_svr4_sys_acl	185
#define	SVR4_SYS_svr4_sys_auditsys	186
#define	SVR4_SYS_nanosleep	199
#define	SVR4_SYS_svr4_sys_facl	200
#define	SVR4_SYS_setreuid	202
#define	SVR4_SYS_setregid	203
#define	SVR4_SYS_svr4_sys_resolvepath	209
#define	SVR4_SYS_svr4_sys_getdents64	213
#define	SVR4_SYS_svr4_sys_mmap64	214
#define	SVR4_SYS_svr4_sys_stat64	215
#define	SVR4_SYS_svr4_sys_lstat64	216
#define	SVR4_SYS_svr4_sys_fstat64	217
#define	SVR4_SYS_svr4_sys_statvfs64	218
#define	SVR4_SYS_svr4_sys_fstatvfs64	219
#define	SVR4_SYS_svr4_sys_setrlimit64	220
#define	SVR4_SYS_svr4_sys_getrlimit64	221
#define	SVR4_SYS_svr4_sys_creat64	224
#define	SVR4_SYS_svr4_sys_open64	225
#define	SVR4_SYS_svr4_sys_socket	230
#define	SVR4_SYS_socketpair	231
#define	SVR4_SYS_bind	232
#define	SVR4_SYS_listen	233
#define	SVR4_SYS_accept	234
#define	SVR4_SYS_connect	235
#define	SVR4_SYS_shutdown	236
#define	SVR4_SYS_svr4_sys_recv	237
#define	SVR4_SYS_recvfrom	238
#define	SVR4_SYS_recvmsg	239
#define	SVR4_SYS_svr4_sys_send	240
#define	SVR4_SYS_sendmsg	241
#define	SVR4_SYS_svr4_sys_sendto	242
#define	SVR4_SYS_getpeername	243
#define	SVR4_SYS_getsockname	244
#define	SVR4_SYS_getsockopt	245
#define	SVR4_SYS_setsockopt	246
#define	SVR4_SYS_MAXSYSCALL	250
