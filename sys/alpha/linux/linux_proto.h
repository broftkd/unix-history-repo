/*
 * System call prototypes.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD
 */

#ifndef _LINUX_SYSPROTO_H_
#define	_LINUX_SYSPROTO_H_

#include <sys/signal.h>

#include <sys/acl.h>

struct proc;

#define	PAD_(t)	(sizeof(register_t) <= sizeof(t) ? \
		0 : sizeof(register_t) - sizeof(t))

struct	linux_fork_args {
	register_t dummy;
};
struct	osf1_wait4_args {
	int	pid;	char pid_[PAD_(int)];
	int *	status;	char status_[PAD_(int *)];
	int	options;	char options_[PAD_(int)];
	struct osf1_rusage *	rusage;	char rusage_[PAD_(struct osf1_rusage *)];
};
struct	linux_link_args {
	char *	path;	char path_[PAD_(char *)];
	char *	to;	char to_[PAD_(char *)];
};
struct	linux_unlink_args {
	char *	path;	char path_[PAD_(char *)];
};
struct	linux_chdir_args {
	char *	path;	char path_[PAD_(char *)];
};
struct	linux_mknod_args {
	char *	path;	char path_[PAD_(char *)];
	l_int	mode;	char mode_[PAD_(l_int)];
	l_dev_t	dev;	char dev_[PAD_(l_dev_t)];
};
struct	linux_chmod_args {
	char *	path;	char path_[PAD_(char *)];
	l_mode_t	mode;	char mode_[PAD_(l_mode_t)];
};
struct	linux_chown_args {
	char *	path;	char path_[PAD_(char *)];
	l_uid_t	uid;	char uid_[PAD_(l_uid_t)];
	l_gid_t	gid;	char gid_[PAD_(l_gid_t)];
};
struct	linux_brk_args {
	l_ulong	dsend;	char dsend_[PAD_(l_ulong)];
};
struct	linux_lseek_args {
	l_uint	fdes;	char fdes_[PAD_(l_uint)];
	l_off_t	off;	char off_[PAD_(l_off_t)];
	l_int	whence;	char whence_[PAD_(l_int)];
};
struct	linux_getpid_args {
	register_t dummy;
};
struct	linux_umount_args {
	char *	path;	char path_[PAD_(char *)];
	l_int	flags;	char flags_[PAD_(l_int)];
};
struct	linux_getuid_args {
	register_t dummy;
};
struct	linux_ptrace_args {
	register_t dummy;
};
struct	linux_access_args {
	char *	path;	char path_[PAD_(char *)];
	l_int	flags;	char flags_[PAD_(l_int)];
};
struct	linux_kill_args {
	l_int	pid;	char pid_[PAD_(l_int)];
	l_int	signum;	char signum_[PAD_(l_int)];
};
struct	linux_open_args {
	char *	path;	char path_[PAD_(char *)];
	l_int	flags;	char flags_[PAD_(l_int)];
	l_int	mode;	char mode_[PAD_(l_int)];
};
struct	linux_getgid_args {
	register_t dummy;
};
struct	osf1_sigprocmask_args {
	int	how;	char how_[PAD_(int)];
	u_long	mask;	char mask_[PAD_(u_long)];
};
struct	linux_sigpending_args {
	register_t dummy;
};
struct	linux_ioctl_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	l_uint	cmd;	char cmd_[PAD_(l_uint)];
	l_ulong	arg;	char arg_[PAD_(l_ulong)];
};
struct	linux_symlink_args {
	char *	path;	char path_[PAD_(char *)];
	char *	to;	char to_[PAD_(char *)];
};
struct	linux_readlink_args {
	char *	name;	char name_[PAD_(char *)];
	char *	buf;	char buf_[PAD_(char *)];
	l_int	count;	char count_[PAD_(l_int)];
};
struct	linux_execve_args {
	char *	path;	char path_[PAD_(char *)];
	char **	argp;	char argp_[PAD_(char **)];
	char **	envp;	char envp_[PAD_(char **)];
};
struct	linux_getpagesize_args {
	register_t dummy;
};
struct	linux_vfork_args {
	register_t dummy;
};
struct	linux_newstat_args {
	char *	path;	char path_[PAD_(char *)];
	struct l_newstat *	buf;	char buf_[PAD_(struct l_newstat *)];
};
struct	linux_newlstat_args {
	char *	path;	char path_[PAD_(char *)];
	struct l_newstat *	buf;	char buf_[PAD_(struct l_newstat *)];
};
struct	linux_mmap_args {
	l_ulong	addr;	char addr_[PAD_(l_ulong)];
	l_ulong	len;	char len_[PAD_(l_ulong)];
	l_ulong	prot;	char prot_[PAD_(l_ulong)];
	l_ulong	flags;	char flags_[PAD_(l_ulong)];
	l_ulong	fd;	char fd_[PAD_(l_ulong)];
	l_ulong	pos;	char pos_[PAD_(l_ulong)];
};
struct	linux_munmap_args {
	l_ulong	addr;	char addr_[PAD_(l_ulong)];
	l_size_t	len;	char len_[PAD_(l_size_t)];
};
struct	linux_mprotect_args {
	l_ulong	addr;	char addr_[PAD_(l_ulong)];
	l_size_t	len;	char len_[PAD_(l_size_t)];
	l_ulong	prot;	char prot_[PAD_(l_ulong)];
};
struct	linux_madvise_args {
	register_t dummy;
};
struct	linux_vhangup_args {
	register_t dummy;
};
struct	linux_setgroups_args {
	l_int	gidsetsize;	char gidsetsize_[PAD_(l_int)];
	l_gid_t *	grouplist;	char grouplist_[PAD_(l_gid_t *)];
};
struct	linux_getgroups_args {
	l_int	gidsetsize;	char gidsetsize_[PAD_(l_int)];
	l_gid_t *	grouplist;	char grouplist_[PAD_(l_gid_t *)];
};
struct	osf1_setitimer_args {
	u_int	which;	char which_[PAD_(u_int)];
	struct itimerval *	itv;	char itv_[PAD_(struct itimerval *)];
	struct itimerval *	oitv;	char oitv_[PAD_(struct itimerval *)];
};
struct	linux_gethostname_args {
	register_t dummy;
};
struct	linux_getdtablesize_args {
	register_t dummy;
};
struct	linux_newfstat_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	struct l_newstat *	buf;	char buf_[PAD_(struct l_newstat *)];
};
struct	linux_fcntl_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	l_uint	cmd;	char cmd_[PAD_(l_uint)];
	l_ulong	arg;	char arg_[PAD_(l_ulong)];
};
struct	osf1_select_args {
	u_int	nd;	char nd_[PAD_(u_int)];
	fd_set *	in;	char in_[PAD_(fd_set *)];
	fd_set *	ou;	char ou_[PAD_(fd_set *)];
	fd_set *	ex;	char ex_[PAD_(fd_set *)];
	struct timeval *	tv;	char tv_[PAD_(struct timeval *)];
};
struct	osf1_socket_args {
	int	domain;	char domain_[PAD_(int)];
	int	type;	char type_[PAD_(int)];
	int	protocol;	char protocol_[PAD_(int)];
};
struct	linux_connect_args {
	l_int	s;	char s_[PAD_(l_int)];
	struct l_sockaddr *	name;	char name_[PAD_(struct l_sockaddr *)];
	l_int	namelen;	char namelen_[PAD_(l_int)];
};
struct	osf1_sigreturn_args {
	struct osigcontext *	sigcntxp;	char sigcntxp_[PAD_(struct osigcontext *)];
};
struct	osf1_sigsuspend_args {
	unsigned long	ss;	char ss_[PAD_(unsigned long)];
};
struct	linux_recvmsg_args {
	register_t dummy;
};
struct	linux_sendmsg_args {
	register_t dummy;
};
struct	osf1_gettimeofday_args {
	struct timeval *	tp;	char tp_[PAD_(struct timeval *)];
	struct timezone *	tzp;	char tzp_[PAD_(struct timezone *)];
};
struct	osf1_getrusage_args {
	long	who;	char who_[PAD_(long)];
	void *	rusage;	char rusage_[PAD_(void *)];
};
struct	linux_rename_args {
	char *	from;	char from_[PAD_(char *)];
	char *	to;	char to_[PAD_(char *)];
};
struct	linux_truncate_args {
	char *	path;	char path_[PAD_(char *)];
	l_ulong	length;	char length_[PAD_(l_ulong)];
};
struct	osf1_sendto_args {
	int	s;	char s_[PAD_(int)];
	caddr_t	buf;	char buf_[PAD_(caddr_t)];
	size_t	len;	char len_[PAD_(size_t)];
	int	flags;	char flags_[PAD_(int)];
	struct sockaddr *	to;	char to_[PAD_(struct sockaddr *)];
	int	tolen;	char tolen_[PAD_(int)];
};
struct	linux_socketpair_args {
	register_t dummy;
};
struct	linux_mkdir_args {
	char *	path;	char path_[PAD_(char *)];
	l_int	mode;	char mode_[PAD_(l_int)];
};
struct	linux_rmdir_args {
	char *	path;	char path_[PAD_(char *)];
};
struct	linux_getrlimit_args {
	l_uint	resource;	char resource_[PAD_(l_uint)];
	struct l_rlimit *	rlim;	char rlim_[PAD_(struct l_rlimit *)];
};
struct	linux_setrlimit_args {
	l_uint	resource;	char resource_[PAD_(l_uint)];
	struct l_rlimit *	rlim;	char rlim_[PAD_(struct l_rlimit *)];
};
struct	linux_quotactl_args {
	register_t dummy;
};
struct	osf1_sigaction_args {
	int	sig;	char sig_[PAD_(int)];
	struct osf1_sigaction *	nsa;	char nsa_[PAD_(struct osf1_sigaction *)];
	struct osf1_sigaction *	osa;	char osa_[PAD_(struct osf1_sigaction *)];
};
struct	linux_msgctl_args {
	l_int	msqid;	char msqid_[PAD_(l_int)];
	l_int	cmd;	char cmd_[PAD_(l_int)];
	struct l_msqid_ds *	buf;	char buf_[PAD_(struct l_msqid_ds *)];
};
struct	linux_msgget_args {
	l_key_t	key;	char key_[PAD_(l_key_t)];
	l_int	msgflg;	char msgflg_[PAD_(l_int)];
};
struct	linux_msgrcv_args {
	l_int	msqid;	char msqid_[PAD_(l_int)];
	struct l_msgbuf *	msgp;	char msgp_[PAD_(struct l_msgbuf *)];
	l_size_t	msgsz;	char msgsz_[PAD_(l_size_t)];
	l_long	msgtyp;	char msgtyp_[PAD_(l_long)];
	l_int	msgflg;	char msgflg_[PAD_(l_int)];
};
struct	linux_msgsnd_args {
	l_int	msqid;	char msqid_[PAD_(l_int)];
	struct l_msgbuf *	msgp;	char msgp_[PAD_(struct l_msgbuf *)];
	l_size_t	msgsz;	char msgsz_[PAD_(l_size_t)];
	l_int	msgflg;	char msgflg_[PAD_(l_int)];
};
struct	linux_semctl_args {
	l_int	semid;	char semid_[PAD_(l_int)];
	l_int	semnum;	char semnum_[PAD_(l_int)];
	l_int	cmd;	char cmd_[PAD_(l_int)];
	union l_semun	arg;	char arg_[PAD_(union l_semun)];
};
struct	linux_semget_args {
	l_key_t	key;	char key_[PAD_(l_key_t)];
	l_int	nsems;	char nsems_[PAD_(l_int)];
	l_int	semflg;	char semflg_[PAD_(l_int)];
};
struct	linux_semop_args {
	l_int	semid;	char semid_[PAD_(l_int)];
	struct l_sembuf *	tsops;	char tsops_[PAD_(struct l_sembuf *)];
	l_uint	nsops;	char nsops_[PAD_(l_uint)];
};
struct	linux_lchown_args {
	char *	path;	char path_[PAD_(char *)];
	l_uid_t	uid;	char uid_[PAD_(l_uid_t)];
	l_gid_t	gid;	char gid_[PAD_(l_gid_t)];
};
struct	linux_shmat_args {
	l_int	shmid;	char shmid_[PAD_(l_int)];
	char *	shmaddr;	char shmaddr_[PAD_(char *)];
	l_int	shmflg;	char shmflg_[PAD_(l_int)];
};
struct	linux_shmctl_args {
	l_int	shmid;	char shmid_[PAD_(l_int)];
	l_int	cmd;	char cmd_[PAD_(l_int)];
	struct l_shmid_ds *	buf;	char buf_[PAD_(struct l_shmid_ds *)];
};
struct	linux_shmdt_args {
	char *	shmaddr;	char shmaddr_[PAD_(char *)];
};
struct	linux_shmget_args {
	l_key_t	key;	char key_[PAD_(l_key_t)];
	l_size_t	size;	char size_[PAD_(l_size_t)];
	l_int	shmflg;	char shmflg_[PAD_(l_int)];
};
struct	linux_msync_args {
	l_ulong	addr;	char addr_[PAD_(l_ulong)];
	l_size_t	len;	char len_[PAD_(l_size_t)];
	l_int	fl;	char fl_[PAD_(l_int)];
};
struct	linux_getsid_args {
	l_pid_t	pid;	char pid_[PAD_(l_pid_t)];
};
struct	linux_sigaltstack_args {
	register_t dummy;
};
struct	osf1_sysinfo_args {
	int	cmd;	char cmd_[PAD_(int)];
	char *	buf;	char buf_[PAD_(char *)];
	long	count;	char count_[PAD_(long)];
};
struct	linux_sysfs_args {
	l_int	option;	char option_[PAD_(l_int)];
	l_ulong	arg1;	char arg1_[PAD_(l_ulong)];
	l_ulong	arg2;	char arg2_[PAD_(l_ulong)];
};
struct	osf1_getsysinfo_args {
	u_long	op;	char op_[PAD_(u_long)];
	caddr_t	buffer;	char buffer_[PAD_(caddr_t)];
	u_long	nbytes;	char nbytes_[PAD_(u_long)];
	caddr_t	arg;	char arg_[PAD_(caddr_t)];
	u_long	flag;	char flag_[PAD_(u_long)];
};
struct	osf1_setsysinfo_args {
	u_long	op;	char op_[PAD_(u_long)];
	caddr_t	buffer;	char buffer_[PAD_(caddr_t)];
	u_long	nbytes;	char nbytes_[PAD_(u_long)];
	caddr_t	arg;	char arg_[PAD_(caddr_t)];
	u_long	flag;	char flag_[PAD_(u_long)];
};
struct	linux_bdflush_args {
	register_t dummy;
};
struct	linux_sethae_args {
	register_t dummy;
};
struct	linux_mount_args {
	char *	specialfile;	char specialfile_[PAD_(char *)];
	char *	dir;	char dir_[PAD_(char *)];
	char *	filesystemtype;	char filesystemtype_[PAD_(char *)];
	l_ulong	rwflag;	char rwflag_[PAD_(l_ulong)];
	void *	data;	char data_[PAD_(void *)];
};
struct	linux_old_adjtimex_args {
	register_t dummy;
};
struct	linux_swapoff_args {
	register_t dummy;
};
struct	linux_getdents_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	void *	dent;	char dent_[PAD_(void *)];
	l_uint	count;	char count_[PAD_(l_uint)];
};
struct	linux_create_module_args {
	register_t dummy;
};
struct	linux_init_module_args {
	register_t dummy;
};
struct	linux_delete_module_args {
	register_t dummy;
};
struct	linux_get_kernel_syms_args {
	register_t dummy;
};
struct	linux_syslog_args {
	l_int	type;	char type_[PAD_(l_int)];
	char *	buf;	char buf_[PAD_(char *)];
	l_int	len;	char len_[PAD_(l_int)];
};
struct	linux_reboot_args {
	l_int	magic1;	char magic1_[PAD_(l_int)];
	l_int	magic2;	char magic2_[PAD_(l_int)];
	l_uint	cmd;	char cmd_[PAD_(l_uint)];
	void *	arg;	char arg_[PAD_(void *)];
};
struct	linux_clone_args {
	l_int	flags;	char flags_[PAD_(l_int)];
	void *	stack;	char stack_[PAD_(void *)];
};
struct	linux_uselib_args {
	char *	library;	char library_[PAD_(char *)];
};
struct	linux_sysinfo_args {
	register_t dummy;
};
struct	linux_sysctl_args {
	struct l___sysctl_args *	args;	char args_[PAD_(struct l___sysctl_args *)];
};
struct	linux_oldumount_args {
	char *	path;	char path_[PAD_(char *)];
};
struct	linux_times_args {
	struct l_times_argv *	buf;	char buf_[PAD_(struct l_times_argv *)];
};
struct	linux_personality_args {
	l_ulong	per;	char per_[PAD_(l_ulong)];
};
struct	linux_setfsuid_args {
	l_uid_t	uid;	char uid_[PAD_(l_uid_t)];
};
struct	linux_setfsgid_args {
	l_gid_t	gid;	char gid_[PAD_(l_gid_t)];
};
struct	linux_ustat_args {
	l_dev_t	dev;	char dev_[PAD_(l_dev_t)];
	struct l_ustat *	ubuf;	char ubuf_[PAD_(struct l_ustat *)];
};
struct	linux_statfs_args {
	char *	path;	char path_[PAD_(char *)];
	struct l_statfs_buf *	buf;	char buf_[PAD_(struct l_statfs_buf *)];
};
struct	linux_fstatfs_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	struct l_statfs_buf *	buf;	char buf_[PAD_(struct l_statfs_buf *)];
};
struct	linux_sched_setscheduler_args {
	l_pid_t	pid;	char pid_[PAD_(l_pid_t)];
	l_int	policy;	char policy_[PAD_(l_int)];
	struct l_sched_param *	param;	char param_[PAD_(struct l_sched_param *)];
};
struct	linux_sched_getscheduler_args {
	l_pid_t	pid;	char pid_[PAD_(l_pid_t)];
};
struct	linux_sched_get_priority_max_args {
	l_int	policy;	char policy_[PAD_(l_int)];
};
struct	linux_sched_get_priority_min_args {
	l_int	policy;	char policy_[PAD_(l_int)];
};
struct	linux_newuname_args {
	struct l_newuname_t *	buf;	char buf_[PAD_(struct l_newuname_t *)];
};
struct	linux_mremap_args {
	l_ulong	addr;	char addr_[PAD_(l_ulong)];
	l_ulong	old_len;	char old_len_[PAD_(l_ulong)];
	l_ulong	new_len;	char new_len_[PAD_(l_ulong)];
	l_ulong	flags;	char flags_[PAD_(l_ulong)];
	l_ulong	new_addr;	char new_addr_[PAD_(l_ulong)];
};
struct	linux_nfsservctl_args {
	register_t dummy;
};
struct	linux_pciconfig_read_args {
	register_t dummy;
};
struct	linux_pciconfig_write_args {
	register_t dummy;
};
struct	linux_query_module_args {
	register_t dummy;
};
struct	linux_prctl_args {
	register_t dummy;
};
struct	linux_pread_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	char *	buf;	char buf_[PAD_(char *)];
	l_size_t	nbyte;	char nbyte_[PAD_(l_size_t)];
	l_loff_t	offset;	char offset_[PAD_(l_loff_t)];
};
struct	linux_pwrite_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	char *	buf;	char buf_[PAD_(char *)];
	l_size_t	nbyte;	char nbyte_[PAD_(l_size_t)];
	l_loff_t	offset;	char offset_[PAD_(l_loff_t)];
};
struct	linux_rt_sigreturn_args {
	register_t dummy;
};
struct	linux_rt_sigaction_args {
	l_int	sig;	char sig_[PAD_(l_int)];
	l_sigaction_t *	act;	char act_[PAD_(l_sigaction_t *)];
	l_sigaction_t *	oact;	char oact_[PAD_(l_sigaction_t *)];
	l_size_t	sigsetsize;	char sigsetsize_[PAD_(l_size_t)];
};
struct	linux_rt_sigprocmask_args {
	l_int	how;	char how_[PAD_(l_int)];
	l_sigset_t *	mask;	char mask_[PAD_(l_sigset_t *)];
	l_sigset_t *	omask;	char omask_[PAD_(l_sigset_t *)];
	l_size_t	sigsetsize;	char sigsetsize_[PAD_(l_size_t)];
};
struct	linux_rt_sigpending_args {
	register_t dummy;
};
struct	linux_rt_sigtimedwait_args {
	register_t dummy;
};
struct	linux_rt_sigqueueinfo_args {
	register_t dummy;
};
struct	linux_rt_sigsuspend_args {
	l_sigset_t *	newset;	char newset_[PAD_(l_sigset_t *)];
	l_size_t	sigsetsize;	char sigsetsize_[PAD_(l_size_t)];
};
struct	linux_select_args {
	l_int	nfds;	char nfds_[PAD_(l_int)];
	l_fd_set *	readfds;	char readfds_[PAD_(l_fd_set *)];
	l_fd_set *	writefds;	char writefds_[PAD_(l_fd_set *)];
	l_fd_set *	exceptfds;	char exceptfds_[PAD_(l_fd_set *)];
	struct l_timeval *	timeout;	char timeout_[PAD_(struct l_timeval *)];
};
struct	linux_getitimer_args {
	l_int	which;	char which_[PAD_(l_int)];
	struct l_itimerval *	itv;	char itv_[PAD_(struct l_itimerval *)];
};
struct	linux_setitimer_args {
	l_int	which;	char which_[PAD_(l_int)];
	struct l_itimerval *	itv;	char itv_[PAD_(struct l_itimerval *)];
	struct l_itimerval *	oitv;	char oitv_[PAD_(struct l_itimerval *)];
};
struct	linux_utimes_args {
	char *	fname;	char fname_[PAD_(char *)];
	struct l_timeval *	times;	char times_[PAD_(struct l_timeval *)];
};
struct	linux_wait4_args {
	l_pid_t	pid;	char pid_[PAD_(l_pid_t)];
	l_uint *	status;	char status_[PAD_(l_uint *)];
	l_int	options;	char options_[PAD_(l_int)];
	struct l_rusage *	rusage;	char rusage_[PAD_(struct l_rusage *)];
};
struct	linux_adjtimex_args {
	register_t dummy;
};
struct	linux_getcwd_args {
	char *	buf;	char buf_[PAD_(char *)];
	l_ulong	bufsize;	char bufsize_[PAD_(l_ulong)];
};
struct	linux_capget_args {
	register_t dummy;
};
struct	linux_capset_args {
	register_t dummy;
};
struct	linux_sendfile_args {
	register_t dummy;
};
struct	linux_pivot_root_args {
	char *	new_root;	char new_root_[PAD_(char *)];
	char *	put_old;	char put_old_[PAD_(char *)];
};
struct	linux_mincore_args {
	l_ulong	start;	char start_[PAD_(l_ulong)];
	l_size_t	len;	char len_[PAD_(l_size_t)];
	u_char *	vec;	char vec_[PAD_(u_char *)];
};
struct	linux_pciconfig_iobase_args {
	register_t dummy;
};
struct	linux_getdents64_args {
	l_uint	fd;	char fd_[PAD_(l_uint)];
	void *	dirent;	char dirent_[PAD_(void *)];
	l_uint	count;	char count_[PAD_(l_uint)];
};
int	linux_fork __P((struct proc *, struct linux_fork_args *));
int	osf1_wait4 __P((struct proc *, struct osf1_wait4_args *));
int	linux_link __P((struct proc *, struct linux_link_args *));
int	linux_unlink __P((struct proc *, struct linux_unlink_args *));
int	linux_chdir __P((struct proc *, struct linux_chdir_args *));
int	linux_mknod __P((struct proc *, struct linux_mknod_args *));
int	linux_chmod __P((struct proc *, struct linux_chmod_args *));
int	linux_chown __P((struct proc *, struct linux_chown_args *));
int	linux_brk __P((struct proc *, struct linux_brk_args *));
int	linux_lseek __P((struct proc *, struct linux_lseek_args *));
int	linux_getpid __P((struct proc *, struct linux_getpid_args *));
int	linux_umount __P((struct proc *, struct linux_umount_args *));
int	linux_getuid __P((struct proc *, struct linux_getuid_args *));
int	linux_ptrace __P((struct proc *, struct linux_ptrace_args *));
int	linux_access __P((struct proc *, struct linux_access_args *));
int	linux_kill __P((struct proc *, struct linux_kill_args *));
int	linux_open __P((struct proc *, struct linux_open_args *));
int	linux_getgid __P((struct proc *, struct linux_getgid_args *));
int	osf1_sigprocmask __P((struct proc *, struct osf1_sigprocmask_args *));
int	linux_sigpending __P((struct proc *, struct linux_sigpending_args *));
int	linux_ioctl __P((struct proc *, struct linux_ioctl_args *));
int	linux_symlink __P((struct proc *, struct linux_symlink_args *));
int	linux_readlink __P((struct proc *, struct linux_readlink_args *));
int	linux_execve __P((struct proc *, struct linux_execve_args *));
int	linux_getpagesize __P((struct proc *, struct linux_getpagesize_args *));
int	linux_vfork __P((struct proc *, struct linux_vfork_args *));
int	linux_newstat __P((struct proc *, struct linux_newstat_args *));
int	linux_newlstat __P((struct proc *, struct linux_newlstat_args *));
int	linux_mmap __P((struct proc *, struct linux_mmap_args *));
int	linux_munmap __P((struct proc *, struct linux_munmap_args *));
int	linux_mprotect __P((struct proc *, struct linux_mprotect_args *));
int	linux_madvise __P((struct proc *, struct linux_madvise_args *));
int	linux_vhangup __P((struct proc *, struct linux_vhangup_args *));
int	linux_setgroups __P((struct proc *, struct linux_setgroups_args *));
int	linux_getgroups __P((struct proc *, struct linux_getgroups_args *));
int	osf1_setitimer __P((struct proc *, struct osf1_setitimer_args *));
int	linux_gethostname __P((struct proc *, struct linux_gethostname_args *));
int	linux_getdtablesize __P((struct proc *, struct linux_getdtablesize_args *));
int	linux_newfstat __P((struct proc *, struct linux_newfstat_args *));
int	linux_fcntl __P((struct proc *, struct linux_fcntl_args *));
int	osf1_select __P((struct proc *, struct osf1_select_args *));
int	osf1_socket __P((struct proc *, struct osf1_socket_args *));
int	linux_connect __P((struct proc *, struct linux_connect_args *));
int	osf1_sigreturn __P((struct proc *, struct osf1_sigreturn_args *));
int	osf1_sigsuspend __P((struct proc *, struct osf1_sigsuspend_args *));
int	linux_recvmsg __P((struct proc *, struct linux_recvmsg_args *));
int	linux_sendmsg __P((struct proc *, struct linux_sendmsg_args *));
int	osf1_gettimeofday __P((struct proc *, struct osf1_gettimeofday_args *));
int	osf1_getrusage __P((struct proc *, struct osf1_getrusage_args *));
int	linux_rename __P((struct proc *, struct linux_rename_args *));
int	linux_truncate __P((struct proc *, struct linux_truncate_args *));
int	osf1_sendto __P((struct proc *, struct osf1_sendto_args *));
int	linux_socketpair __P((struct proc *, struct linux_socketpair_args *));
int	linux_mkdir __P((struct proc *, struct linux_mkdir_args *));
int	linux_rmdir __P((struct proc *, struct linux_rmdir_args *));
int	linux_getrlimit __P((struct proc *, struct linux_getrlimit_args *));
int	linux_setrlimit __P((struct proc *, struct linux_setrlimit_args *));
int	linux_quotactl __P((struct proc *, struct linux_quotactl_args *));
int	osf1_sigaction __P((struct proc *, struct osf1_sigaction_args *));
int	linux_msgctl __P((struct proc *, struct linux_msgctl_args *));
int	linux_msgget __P((struct proc *, struct linux_msgget_args *));
int	linux_msgrcv __P((struct proc *, struct linux_msgrcv_args *));
int	linux_msgsnd __P((struct proc *, struct linux_msgsnd_args *));
int	linux_semctl __P((struct proc *, struct linux_semctl_args *));
int	linux_semget __P((struct proc *, struct linux_semget_args *));
int	linux_semop __P((struct proc *, struct linux_semop_args *));
int	linux_lchown __P((struct proc *, struct linux_lchown_args *));
int	linux_shmat __P((struct proc *, struct linux_shmat_args *));
int	linux_shmctl __P((struct proc *, struct linux_shmctl_args *));
int	linux_shmdt __P((struct proc *, struct linux_shmdt_args *));
int	linux_shmget __P((struct proc *, struct linux_shmget_args *));
int	linux_msync __P((struct proc *, struct linux_msync_args *));
int	linux_getsid __P((struct proc *, struct linux_getsid_args *));
int	linux_sigaltstack __P((struct proc *, struct linux_sigaltstack_args *));
int	osf1_sysinfo __P((struct proc *, struct osf1_sysinfo_args *));
int	linux_sysfs __P((struct proc *, struct linux_sysfs_args *));
int	osf1_getsysinfo __P((struct proc *, struct osf1_getsysinfo_args *));
int	osf1_setsysinfo __P((struct proc *, struct osf1_setsysinfo_args *));
int	linux_bdflush __P((struct proc *, struct linux_bdflush_args *));
int	linux_sethae __P((struct proc *, struct linux_sethae_args *));
int	linux_mount __P((struct proc *, struct linux_mount_args *));
int	linux_old_adjtimex __P((struct proc *, struct linux_old_adjtimex_args *));
int	linux_swapoff __P((struct proc *, struct linux_swapoff_args *));
int	linux_getdents __P((struct proc *, struct linux_getdents_args *));
int	linux_create_module __P((struct proc *, struct linux_create_module_args *));
int	linux_init_module __P((struct proc *, struct linux_init_module_args *));
int	linux_delete_module __P((struct proc *, struct linux_delete_module_args *));
int	linux_get_kernel_syms __P((struct proc *, struct linux_get_kernel_syms_args *));
int	linux_syslog __P((struct proc *, struct linux_syslog_args *));
int	linux_reboot __P((struct proc *, struct linux_reboot_args *));
int	linux_clone __P((struct proc *, struct linux_clone_args *));
int	linux_uselib __P((struct proc *, struct linux_uselib_args *));
int	linux_sysinfo __P((struct proc *, struct linux_sysinfo_args *));
int	linux_sysctl __P((struct proc *, struct linux_sysctl_args *));
int	linux_oldumount __P((struct proc *, struct linux_oldumount_args *));
int	linux_times __P((struct proc *, struct linux_times_args *));
int	linux_personality __P((struct proc *, struct linux_personality_args *));
int	linux_setfsuid __P((struct proc *, struct linux_setfsuid_args *));
int	linux_setfsgid __P((struct proc *, struct linux_setfsgid_args *));
int	linux_ustat __P((struct proc *, struct linux_ustat_args *));
int	linux_statfs __P((struct proc *, struct linux_statfs_args *));
int	linux_fstatfs __P((struct proc *, struct linux_fstatfs_args *));
int	linux_sched_setscheduler __P((struct proc *, struct linux_sched_setscheduler_args *));
int	linux_sched_getscheduler __P((struct proc *, struct linux_sched_getscheduler_args *));
int	linux_sched_get_priority_max __P((struct proc *, struct linux_sched_get_priority_max_args *));
int	linux_sched_get_priority_min __P((struct proc *, struct linux_sched_get_priority_min_args *));
int	linux_newuname __P((struct proc *, struct linux_newuname_args *));
int	linux_mremap __P((struct proc *, struct linux_mremap_args *));
int	linux_nfsservctl __P((struct proc *, struct linux_nfsservctl_args *));
int	linux_pciconfig_read __P((struct proc *, struct linux_pciconfig_read_args *));
int	linux_pciconfig_write __P((struct proc *, struct linux_pciconfig_write_args *));
int	linux_query_module __P((struct proc *, struct linux_query_module_args *));
int	linux_prctl __P((struct proc *, struct linux_prctl_args *));
int	linux_pread __P((struct proc *, struct linux_pread_args *));
int	linux_pwrite __P((struct proc *, struct linux_pwrite_args *));
int	linux_rt_sigreturn __P((struct proc *, struct linux_rt_sigreturn_args *));
int	linux_rt_sigaction __P((struct proc *, struct linux_rt_sigaction_args *));
int	linux_rt_sigprocmask __P((struct proc *, struct linux_rt_sigprocmask_args *));
int	linux_rt_sigpending __P((struct proc *, struct linux_rt_sigpending_args *));
int	linux_rt_sigtimedwait __P((struct proc *, struct linux_rt_sigtimedwait_args *));
int	linux_rt_sigqueueinfo __P((struct proc *, struct linux_rt_sigqueueinfo_args *));
int	linux_rt_sigsuspend __P((struct proc *, struct linux_rt_sigsuspend_args *));
int	linux_select __P((struct proc *, struct linux_select_args *));
int	linux_getitimer __P((struct proc *, struct linux_getitimer_args *));
int	linux_setitimer __P((struct proc *, struct linux_setitimer_args *));
int	linux_utimes __P((struct proc *, struct linux_utimes_args *));
int	linux_wait4 __P((struct proc *, struct linux_wait4_args *));
int	linux_adjtimex __P((struct proc *, struct linux_adjtimex_args *));
int	linux_getcwd __P((struct proc *, struct linux_getcwd_args *));
int	linux_capget __P((struct proc *, struct linux_capget_args *));
int	linux_capset __P((struct proc *, struct linux_capset_args *));
int	linux_sendfile __P((struct proc *, struct linux_sendfile_args *));
int	linux_pivot_root __P((struct proc *, struct linux_pivot_root_args *));
int	linux_mincore __P((struct proc *, struct linux_mincore_args *));
int	linux_pciconfig_iobase __P((struct proc *, struct linux_pciconfig_iobase_args *));
int	linux_getdents64 __P((struct proc *, struct linux_getdents64_args *));

#ifdef COMPAT_43


#endif /* COMPAT_43 */

#undef PAD_

#endif /* !_LINUX_SYSPROTO_H_ */
