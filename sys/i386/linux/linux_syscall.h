/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD: src/sys/i386/linux/syscalls.master,v 1.16.2.7 2000/01/13 17:26:19 marcel Exp 
 */

#define	LINUX_SYS_linux_setup	0
#define	LINUX_SYS_exit	1
#define	LINUX_SYS_linux_fork	2
#define	LINUX_SYS_read	3
#define	LINUX_SYS_write	4
#define	LINUX_SYS_linux_open	5
#define	LINUX_SYS_close	6
#define	LINUX_SYS_linux_waitpid	7
#define	LINUX_SYS_linux_creat	8
#define	LINUX_SYS_linux_link	9
#define	LINUX_SYS_linux_unlink	10
#define	LINUX_SYS_linux_execve	11
#define	LINUX_SYS_linux_chdir	12
#define	LINUX_SYS_linux_time	13
#define	LINUX_SYS_linux_mknod	14
#define	LINUX_SYS_linux_chmod	15
#define	LINUX_SYS_linux_lchown	16
#define	LINUX_SYS_linux_break	17
#define	LINUX_SYS_linux_stat	18
#define	LINUX_SYS_linux_lseek	19
#define	LINUX_SYS_getpid	20
#define	LINUX_SYS_linux_mount	21
#define	LINUX_SYS_linux_umount	22
#define	LINUX_SYS_setuid	23
#define	LINUX_SYS_getuid	24
#define	LINUX_SYS_linux_stime	25
#define	LINUX_SYS_linux_ptrace	26
#define	LINUX_SYS_linux_alarm	27
#define	LINUX_SYS_linux_fstat	28
#define	LINUX_SYS_linux_pause	29
#define	LINUX_SYS_linux_utime	30
#define	LINUX_SYS_linux_stty	31
#define	LINUX_SYS_linux_gtty	32
#define	LINUX_SYS_linux_access	33
#define	LINUX_SYS_linux_nice	34
#define	LINUX_SYS_linux_ftime	35
#define	LINUX_SYS_sync	36
#define	LINUX_SYS_linux_kill	37
#define	LINUX_SYS_linux_rename	38
#define	LINUX_SYS_linux_mkdir	39
#define	LINUX_SYS_linux_rmdir	40
#define	LINUX_SYS_dup	41
#define	LINUX_SYS_linux_pipe	42
#define	LINUX_SYS_linux_times	43
#define	LINUX_SYS_linux_prof	44
#define	LINUX_SYS_linux_brk	45
#define	LINUX_SYS_setgid	46
#define	LINUX_SYS_getgid	47
#define	LINUX_SYS_linux_signal	48
#define	LINUX_SYS_geteuid	49
#define	LINUX_SYS_getegid	50
#define	LINUX_SYS_acct	51
#define	LINUX_SYS_linux_phys	52
#define	LINUX_SYS_linux_lock	53
#define	LINUX_SYS_linux_ioctl	54
#define	LINUX_SYS_linux_fcntl	55
#define	LINUX_SYS_linux_mpx	56
#define	LINUX_SYS_setpgid	57
#define	LINUX_SYS_linux_ulimit	58
#define	LINUX_SYS_linux_olduname	59
#define	LINUX_SYS_umask	60
#define	LINUX_SYS_chroot	61
#define	LINUX_SYS_linux_ustat	62
#define	LINUX_SYS_dup2	63
#define	LINUX_SYS_getppid	64
#define	LINUX_SYS_getpgrp	65
#define	LINUX_SYS_setsid	66
#define	LINUX_SYS_linux_sigaction	67
#define	LINUX_SYS_linux_siggetmask	68
#define	LINUX_SYS_linux_sigsetmask	69
#define	LINUX_SYS_setreuid	70
#define	LINUX_SYS_setregid	71
#define	LINUX_SYS_linux_sigsuspend	72
#define	LINUX_SYS_linux_sigpending	73
#define	LINUX_SYS_osethostname	74
#define	LINUX_SYS_linux_setrlimit	75
#define	LINUX_SYS_linux_getrlimit	76
#define	LINUX_SYS_getrusage	77
#define	LINUX_SYS_gettimeofday	78
#define	LINUX_SYS_settimeofday	79
#define	LINUX_SYS_linux_getgroups	80
#define	LINUX_SYS_linux_setgroups	81
#define	LINUX_SYS_linux_select	82
#define	LINUX_SYS_linux_symlink	83
#define	LINUX_SYS_ostat	84
#define	LINUX_SYS_linux_readlink	85
#define	LINUX_SYS_linux_uselib	86
#define	LINUX_SYS_swapon	87
#define	LINUX_SYS_reboot	88
#define	LINUX_SYS_linux_readdir	89
#define	LINUX_SYS_linux_mmap	90
#define	LINUX_SYS_munmap	91
#define	LINUX_SYS_linux_truncate	92
#define	LINUX_SYS_oftruncate	93
#define	LINUX_SYS_fchmod	94
#define	LINUX_SYS_fchown	95
#define	LINUX_SYS_getpriority	96
#define	LINUX_SYS_setpriority	97
#define	LINUX_SYS_profil	98
#define	LINUX_SYS_linux_statfs	99
#define	LINUX_SYS_linux_fstatfs	100
#define	LINUX_SYS_linux_ioperm	101
#define	LINUX_SYS_linux_socketcall	102
#define	LINUX_SYS_linux_ksyslog	103
#define	LINUX_SYS_linux_setitimer	104
#define	LINUX_SYS_linux_getitimer	105
#define	LINUX_SYS_linux_newstat	106
#define	LINUX_SYS_linux_newlstat	107
#define	LINUX_SYS_linux_newfstat	108
#define	LINUX_SYS_linux_uname	109
#define	LINUX_SYS_linux_iopl	110
#define	LINUX_SYS_linux_vhangup	111
#define	LINUX_SYS_linux_idle	112
#define	LINUX_SYS_linux_vm86	113
#define	LINUX_SYS_linux_wait4	114
#define	LINUX_SYS_linux_swapoff	115
#define	LINUX_SYS_linux_sysinfo	116
#define	LINUX_SYS_linux_ipc	117
#define	LINUX_SYS_fsync	118
#define	LINUX_SYS_linux_sigreturn	119
#define	LINUX_SYS_linux_clone	120
#define	LINUX_SYS_setdomainname	121
#define	LINUX_SYS_linux_newuname	122
#define	LINUX_SYS_linux_modify_ldt	123
#define	LINUX_SYS_linux_adjtimex	124
#define	LINUX_SYS_mprotect	125
#define	LINUX_SYS_linux_sigprocmask	126
#define	LINUX_SYS_linux_create_module	127
#define	LINUX_SYS_linux_init_module	128
#define	LINUX_SYS_linux_delete_module	129
#define	LINUX_SYS_linux_get_kernel_syms	130
#define	LINUX_SYS_linux_quotactl	131
#define	LINUX_SYS_linux_getpgid	132
#define	LINUX_SYS_fchdir	133
#define	LINUX_SYS_linux_bdflush	134
#define	LINUX_SYS_linux_personality	136
#define	LINUX_SYS_linux_llseek	140
#define	LINUX_SYS_linux_getdents	141
#define	LINUX_SYS_linux_newselect	142
#define	LINUX_SYS_flock	143
#define	LINUX_SYS_linux_msync	144
#define	LINUX_SYS_readv	145
#define	LINUX_SYS_writev	146
#define	LINUX_SYS_linux_fdatasync	148
#define	LINUX_SYS_mlock	150
#define	LINUX_SYS_munlock	151
#define	LINUX_SYS_mlockall	152
#define	LINUX_SYS_munlockall	153
#define	LINUX_SYS_sched_setparam	154
#define	LINUX_SYS_sched_getparam	155
#define	LINUX_SYS_linux_sched_setscheduler	156
#define	LINUX_SYS_linux_sched_getscheduler	157
#define	LINUX_SYS_sched_yield	158
#define	LINUX_SYS_sched_get_priority_max	159
#define	LINUX_SYS_sched_get_priority_min	160
#define	LINUX_SYS_sched_rr_get_interval	161
#define	LINUX_SYS_nanosleep	162
#define	LINUX_SYS_linux_mremap	163
#define	LINUX_SYS_poll	168
#define	LINUX_SYS_linux_rt_sigaction	174
#define	LINUX_SYS_linux_rt_sigprocmask	175
#define	LINUX_SYS_linux_rt_sigsuspend	179
#define	LINUX_SYS_linux_chown	182
#define	LINUX_SYS_linux_getcwd	183
#define	LINUX_SYS_linux_sigaltstack	186
#define	LINUX_SYS_linux_vfork	190
#define	LINUX_SYS_MAXSYSCALL	191
