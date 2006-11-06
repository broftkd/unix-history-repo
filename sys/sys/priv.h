/*-
 * Copyright (c) 2006 nCircle Network Security, Inc.
 * All rights reserved.
 *
 * This software was developed by Robert N. M. Watson for the TrustedBSD
 * Project under contract to nCircle Network Security, Inc.
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR, NCIRCLE NETWORK SECURITY,
 * INC., OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/*
 * Privilege checking interface for BSD kernel.
 */
#ifndef _SYS_PRIV_H_
#define	_SYS_PRIV_H_

/*
 * Privilege list.  In no particular order.
 *
 * Think carefully before adding or reusing one of these privileges -- are
 * there existing instances referring to the same privilege?  Third party
 * vendors may request the assignment of privileges to be used in loadable
 * modules.  Particular numeric privilege assignments are part of the
 * loadable kernel module ABI, and should not be changed across minor
 * releases.
 *
 * When adding a new privilege, remember to determine if it's appropriate for
 * use in jail, and update the privilege switch in kern_jail.c as necessary.
 */

/*
 * Track beginning of privilege list.
 */
#define	_PRIV_LOWEST	0

/*
 * PRIV_ROOT is a catch-all for as yet unnamed privileges.  No new
 * references to this privilege should be added.
 */
#define	PRIV_ROOT	1	/* Catch-all during development. */

/*
 * The remaining privileges typically correspond to one or a small
 * number of specific privilege checks, and have (relatively) precise
 * meanings.  They are loosely sorted into a set of base system
 * privileges, such as the ability to reboot, and then loosely by
 * subsystem, indicated by a subsystem name.
 */
#define	PRIV_ACCT		2	/* Manage process accounting. */
#define	PRIV_MAXFILES		3	/* Exceed system open files limit. */
#define	PRIV_MAXPROC		4	/* Exceed system processes limit. */
#define	PRIV_KTRACE		5	/* Set/clear KTRFAC_ROOT on ktrace. */
#define	PRIV_SETDUMPER		6	/* Configure dump device. */
#define	PRIV_NFSD		7	/* Can become NFS daemon. */
#define	PRIV_REBOOT		8	/* Can reboot system. */
#define	PRIV_SWAPON		9	/* Can swapon(). */
#define	PRIV_SWAPOFF		10	/* Can swapoff(). */
#define	PRIV_MSGBUF		11	/* Can read kernel message buffer. */
#define	PRIV_WITNESS		12	/* Can configure WITNESS. */
#define	PRIV_IO			13	/* Can perform low-level I/O. */
#define	PRIV_KEYBOARD		14	/* Reprogram keyboard. */
#define	PRIV_DRIVER		15	/* Low-level driver privilege. */
#define	PRIV_ADJTIME		16	/* Set time adjustment. */
#define	PRIV_NTP_ADJTIME	17	/* Set NTP time adjustment. */
#define	PRIV_CLOCK_SETTIME	18	/* Can call clock_settime. */
#define	PRIV_SETTIMEOFDAY	19	/* Can call settimeofday. */
#define	PRIV_SETHOSTID		20	/* Can call sethostid. */
#define	PRIV_SETDOMAINNAME	21	/* Can call setdomainname. */

/*
 * Audit subsystem privileges.
 */
#define	PRIV_AUDIT_CONTROL	40	/* Can configure audit. */
#define	PRIV_AUDIT_FAILSTOP	41	/* Can run during audit fail stop. */
#define	PRIV_AUDIT_GETAUDIT	42	/* Can get proc audit properties. */
#define	PRIV_AUDIT_SETAUDIT	43	/* Can set proc audit properties. */
#define	PRIV_AUDIT_SUBMIT	44	/* Can submit an audit record. */

/*
 * Credential management privileges.
 */
#define	PRIV_CRED_SETUID	50	/* setuid. */
#define	PRIV_CRED_SETEUID	51	/* seteuid to !ruid and !svuid. */
#define	PRIV_CRED_SETGID	52	/* setgid. */
#define	PRIV_CRED_SETEGID	53	/* setgid to !rgid and !svgid. */
#define	PRIV_CRED_SETGROUPS	54	/* Set process additional groups. */
#define	PRIV_CRED_SETREUID	55	/* setreuid. */
#define	PRIV_CRED_SETREGID	56	/* setregid. */
#define	PRIV_CRED_SETRESUID	57	/* setresuid. */
#define	PRIV_CRED_SETRESGID	58	/* setresgid. */
#define	PRIV_SEEOTHERGIDS	59	/* Exempt bsd.seeothergids. */
#define	PRIV_SEEOTHERUIDS	60	/* Exempt bsd.seeotheruids. */

/*
 * Debugging privileges.
 */
#define	PRIV_DEBUG_DIFFCRED	80	/* Exempt debugging other users. */
#define	PRIV_DEBUG_SUGID	81	/* Exempt debugging setuid proc. */
#define	PRIV_DEBUG_UNPRIV	82	/* Exempt unprivileged debug limit. */

/*
 * Dtrace privileges.
 */
#define	PRIV_DTRACE_KERNEL	90	/* Allow use of DTrace on the kernel. */
#define	PRIV_DTRACE_PROC	91	/* Allow attaching DTrace to process. */
#define	PRIV_DTRACE_USER	92	/* Process may submit DTrace events. */

/*
 * Firmware privilegs.
 */
#define	PRIV_FIRMWARE_LOAD	100	/* Can load firmware. */

/*
 * Jail privileges.
 */
#define	PRIV_JAIL_ATTACH	110	/* Attach to a jail. */

/*
 * Kernel environment priveleges.
 */
#define	PRIV_KENV_SET		120	/* Set kernel env. variables. */
#define	PRIV_KENV_UNSET		121	/* Unset kernel env. variables. */

/*
 * Loadable kernel module privileges.
 */
#define	PRIV_KLD_LOAD		130	/* Load a kernel module. */
#define	PRIV_KLD_UNLOAD		131	/* Unload a kernel module. */

/*
 * Privileges associated with the MAC Framework and specific MAC policy
 * modules.
 */
#define	PRIV_MAC_PARTITION	140	/* Privilege in mac_partition policy. */
#define	PRIV_MAC_PRIVS		141	/* Privilege in the mac_privs policy. */

/*
 * Process-related privileges.
 */
#define	PRIV_PROC_LIMIT		160	/* Exceed user process limit. */
#define	PRIV_PROC_SETLOGIN	161	/* Can call setlogin. */
#define	PRIV_PROC_SETRLIMIT	162	/* Can raise resources limits. */

/* System V IPC privileges.
 */
#define	PRIV_IPC_READ		170	/* Can override IPC read perm. */
#define	PRIV_IPC_WRITE		171	/* Can override IPC write perm. */
#define	PRIV_IPC_EXEC		172	/* Can override IPC exec perm. */
#define	PRIV_IPC_ADMIN		173	/* Can override IPC owner-only perm. */
#define	PRIV_IPC_MSGSIZE	174	/* Exempt IPC message queue limit. */

/*
 * POSIX message queue privileges.
 */
#define	PRIV_MQ_ADMIN		180	/* Can override msgq owner-only perm. */

/*
 * Performance monitoring counter privileges.
 */
#define	PRIV_PMC_MANAGE		190	/* Can administer PMC. */
#define	PRIV_PMC_SYSTEM		191	/* Can allocate a system-wide PMC. */

/*
 * Scheduling privileges.
 */
#define	PRIV_SCHED_DIFFCRED	200	/* Exempt scheduling other users. */
#define	PRIV_SCHED_SETPRIORITY	201	/* Can set lower nice value for proc. */
#define	PRIV_SCHED_RTPRIO	202	/* Can set real time scheduling. */
#define	PRIV_SCHED_SETPOLICY	203	/* Can set scheduler policy. */
#define	PRIV_SCHED_SET		204	/* Can set thread scheduler. */
#define	PRIV_SCHED_SETPARAM	205	/* Can set thread scheduler params. */

/*
 * POSIX semaphore privileges.
 */
#define	PRIV_SEM_WRITE		220	/* Can override sem write perm. */

/*
 * Signal privileges.
 */
#define	PRIV_SIGNAL_DIFFCRED	230	/* Exempt signalling other users. */
#define	PRIV_SIGNAL_SUGID	231	/* Non-conserv signal setuid proc. */

/*
 * Sysctl privileges.
 */
#define	PRIV_SYSCTL_DEBUG	240	/* Can invoke sysctl.debug. */
#define	PRIV_SYSCTL_WRITE	241	/* Can write sysctls. */
#define	PRIV_SYSCTL_WRITEJAIL	242	/* Can write sysctls, jail permitted. */

/*
 * TTY privileges.
 */
#define	PRIV_TTY_CONSOLE	250	/* Set console to tty. */
#define	PRIV_TTY_DRAINWAIT	251	/* Set tty drain wait time. */
#define	PRIV_TTY_DTRWAIT	252	/* Set DTR wait on tty. */
#define	PRIV_TTY_EXCLUSIVE	253	/* Override tty exclusive flag. */
#define	PRIV_TTY_PRISON		254	/* Can open pts across jails. */
#define	PRIV_TTY_STI		255	/* Simulate input on another tty. */
#define	PRIV_TTY_SETA		256	/* Set tty termios structure. */

/*
 * UFS-specific privileges.
 */
#define	PRIV_UFS_EXTATTRCTL	270	/* Can configure EAs on UFS1. */
#define	PRIV_UFS_GETQUOTA	271	/* getquota(). */
#define	PRIV_UFS_QUOTAOFF	272	/* quotaoff(). */
#define	PRIV_UFS_QUOTAON	273	/* quotaon(). */
#define	PRIV_UFS_SETQUOTA	274	/* setquota(). */
#define	PRIV_UFS_SETUSE		275	/* setuse(). */
#define	PRIV_UFS_EXCEEDQUOTA	276	/* Exempt from quota restrictions. */

/*
 * VFS privileges.
 */
#define	PRIV_VFS_READ		310	/* Override vnode DAC read perm. */
#define	PRIV_VFS_WRITE		311	/* Override vnode DAC write perm. */
#define	PRIV_VFS_ADMIN		312	/* Override vnode DAC admin perm. */
#define	PRIV_VFS_EXEC		313	/* Override vnode DAC exec perm. */
#define	PRIV_VFS_LOOKUP		314	/* Override vnode DAC lookup perm. */
#define	PRIV_VFS_BLOCKRESERVE	315	/* Can use free block reserve. */
#define	PRIV_VFS_CHFLAGS_DEV	316	/* Can chflags() a device node. */
#define	PRIV_VFS_CHOWN		317	/* Can set user; group to non-member. */
#define	PRIV_VFS_CHROOT		318	/* chroot(). */
#define	PRIV_VFS_CLEARSUGID	319	/* Don't clear sugid on change. */
#define	PRIV_VFS_EXTATTR_SYSTEM	320	/* Operate on system EA namespace. */
#define	PRIV_VFS_FCHROOT	321	/* fchroot(). */
#define	PRIV_VFS_FHOPEN		322	/* Can fhopen(). */
#define	PRIV_VFS_FHSTAT		323	/* Can fhstat(). */
#define	PRIV_VFS_FHSTATFS	324	/* Can fhstatfs(). */
#define	PRIV_VFS_GENERATION	325	/* stat() returns generation number. */
#define	PRIV_VFS_GETFH		326	/* Can retrieve file handles. */
#define	PRIV_VFS_LINK		327	/* bsd.hardlink_check_uid */
#define	PRIV_VFS_MKNOD_BAD	328	/* Can mknod() to mark bad inodes. */
#define	PRIV_VFS_MKNOD_DEV	329	/* Can mknod() to create dev nodes. */
#define	PRIV_VFS_MKNOD_WHT	330	/* Can mknod() to create whiteout. */
#define	PRIV_VFS_MOUNT		331	/* Can mount(). */
#define	PRIV_VFS_MOUNT_OWNER	332	/* Override owner on user mounts. */
#define	PRIV_VFS_MOUNT_EXPORTED	333	/* Can set MNT_EXPORTED on mount. */
#define	PRIV_VFS_MOUNT_PERM	334	/* Override dev node perms at mount. */
#define	PRIV_VFS_MOUNT_SUIDDIR	335	/* Can set MNT_SUIDDIR on mount. */
#define	PRIV_VFS_MOUNT_NONUSER	336	/* Can perform a non-user mount. */
#define	PRIV_VFS_SETGID		337	/* Can setgid if not in group. */
#define	PRIV_VFS_STICKYFILE	338	/* Can set sticky bit on file. */
#define	PRIV_VFS_SYSFLAGS	339	/* Can modify system flags. */
#define	PRIV_VFS_UNMOUNT	340	/* Can unmount(). */

/*
 * Virtual memory privileges.
 */
#define	PRIV_VM_MADV_PROTECT	360	/* Can set MADV_PROTECT. */
#define	PRIV_VM_MLOCK		361	/* Can mlock(), mlockall(). */
#define	PRIV_VM_MUNLOCK		362	/* Can munlock(), munlockall(). */

/*
 * Device file system privileges.
 */
#define	PRIV_DEVFS_RULE		370	/* Can manage devfs rules. */
#define	PRIV_DEVFS_SYMLINK	371	/* Can create symlinks in devfs. */

/*
 * Random number generator privileges.
 */
#define	PRIV_RANDOM_RESEED	380	/* Closing /dev/random reseeds. */

/*
 * Network stack privileges.
 */
#define	PRIV_NET_BRIDGE		390	/* Administer bridge. */
#define	PRIV_NET_GRE		391	/* Administer GRE. */
#define	PRIV_NET_PPP		392	/* Administer PPP. */
#define	PRIV_NET_SLIP		393	/* Administer SLIP. */
#define	PRIV_NET_BPF		394	/* Monitor BPF. */
#define	PRIV_NET_RAW		395	/* Open raw socket. */
#define	PRIV_NET_ROUTE		396	/* Administer routing. */
#define	PRIV_NET_TAP		397	/* Can open tap device. */
#define	PRIV_NET_SETIFMTU	398	/* Set interface MTU. */
#define	PRIV_NET_SETIFFLAGS	399	/* Set interface flags. */
#define	PRIV_NET_SETIFCAP	400	/* Set interface capabilities. */
#define	PRIV_NET_SETIFNAME	401	/* Set interface name. */
#define	PRIV_NET_SETIFMETRIC	402	/* Set interface metrics. */
#define	PRIV_NET_SETIFPHYS	403	/* Set interface physical layer prop. */
#define	PRIV_NET_SETIFMAC	404	/* Set interface MAC label. */
#define	PRIV_NET_ADDMULTI	405	/* Add multicast addr. to ifnet. */
#define	PRIV_NET_DELMULTI	406	/* Delete multicast addr. from ifnet. */
#define	PRIV_NET_HWIOCTL	507	/* Issue hardware ioctl on ifnet. */
#define	PRIV_NET_SETLLADDR	508
#define	PRIV_NET_ADDIFGROUP	509	/* Add new interface group. */
#define	PRIV_NET_DELIFGROUP	510	/* Delete interface group. */
#define	PRIV_NET_IFCREATE	511	/* Create cloned interface. */
#define	PRIV_NET_IFDESTROY	512	/* Destroy cloned interface. */
#define	PRIV_NET_ADDIFADDR	513	/* Add protocol addr to interface. */
#define	PRIV_NET_DELIFADDR	514	/* Delete protocol addr on interface. */

/*
 * 802.11-related privileges.
 */
#define	PRIV_NET80211_GETKEY	540	/* Query 802.11 keys. */
#define	PRIV_NET80211_MANAGE	541	/* Administer 802.11. */

/*
 * AppleTalk privileges.
 */
#define	PRIV_NETATALK_RESERVEDPORT	550	/* Bind low port number. */

/*
 * ATM privileges.
 */
#define	PRIV_NETATM_CFG		560
#define	PRIV_NETATM_ADD		561
#define	PRIV_NETATM_DEL		562
#define	PRIV_NETATM_SET		563

/*
 * Bluetooth privileges.
 */
#define	PRIV_NETBLUETOOTH_RAW	570	/* Open raw bluetooth socket. */

/*
 * Netgraph and netgraph module privileges.
 */
#define	PRIV_NETGRAPH_CONTROL	580	/* Open netgraph control socket. */
#define	PRIV_NETGRAPH_TTY	581	/* Configure tty for netgraph. */

/*
 * IPv4 and IPv6 privileges.
 */
#define	PRIV_NETINET_RESERVEDPORT	590	/* Bind low port number. */
#define	PRIV_NETINET_IPFW	591	/* Administer IPFW firewall. */
#define	PRIV_NETINET_DIVERT	592	/* Open IP divert socket. */
#define	PRIV_NETINET_PF		593	/* Administer pf firewall. */
#define	PRIV_NETINET_DUMMYNET	594	/* Administer DUMMYNET. */
#define	PRIV_NETINET_CARP	595	/* Administer CARP. */
#define	PRIV_NETINET_MROUTE	596	/* Administer multicast routing. */
#define	PRIV_NETINET_RAW	597	/* Open netinet raw socket. */
#define	PRIV_NETINET_GETCRED	598	/* Query netinet pcb credentials. */
#define	PRIV_NETINET_ADDRCTRL6	599	/* Administer IPv6 address scopes. */
#define	PRIV_NETINET_ND6	600	/* Administer IPv6 neighbor disc. */
#define	PRIV_NETINET_SCOPE6	601	/* Administer IPv6 address scopes. */
#define	PRIV_NETINET_ALIFETIME6	602	/* Administer IPv6 address lifetimes. */
#define	PRIV_NETINET_IPSEC	603	/* Administer IPSEC. */

/*
 * IPX/SPX privileges.
 */
#define	PRIV_NETIPX_RESERVEDPORT	620	/* Bind low port number. */
#define	PRIV_NETIPX_RAW		621	/* Open netipx raw socket. */

/*
 * NCP privileges.
 */
#define	PRIV_NETNCP		630	/* Use another user's connection. */

/*
 * SMB privileges.
 */
#define	PRIV_NETSMB		640	/* Use another user's connection. */

/*
 * VM86 privileges.
 */
#define	PRIV_VM86_INTCALL	650/* Allow invoking vm86 int handlers. */

/*
 * Set of reserved privilege values, which will be allocated to code as
 * needed, in order to avoid renumbering later privileges due to insertion.
 */
#define	_PRIV_RESERVED0		660
#define	_PRIV_RESERVED1		661
#define	_PRIV_RESERVED2		662
#define	_PRIV_RESERVED3		663
#define	_PRIV_RESERVED4		664
#define	_PRIV_RESERVED5		665
#define	_PRIV_RESERVED6		666
#define	_PRIV_RESERVED7		667
#define	_PRIV_RESERVED8		668
#define	_PRIV_RESERVED9		669
#define	_PRIV_RESERVED10	670
#define	_PRIV_RESERVED11	671
#define	_PRIV_RESERVED12	672
#define	_PRIV_RESERVED13	673
#define	_PRIV_RESERVED14	674
#define	_PRIV_RESERVED15	675

/*
 * Define a set of valid privilege numbers that can be used by loadable
 * modules that don't yet have privilege reservations.  Ideally, these should
 * not be used, since their meaning is opaque to any policies that are aware
 * of specific privileges, such as jail, and as such may be arbitrarily
 * denied.
 */
#define	PRIV_MODULE0		700
#define	PRIV_MODULE1		701
#define	PRIV_MODULE2		702
#define	PRIV_MODULE3		703
#define	PRIV_MODULE4		704
#define	PRIV_MODULE5		705
#define	PRIV_MODULE6		706
#define	PRIV_MODULE7		707
#define	PRIV_MODULE8		708
#define	PRIV_MODULE9		709
#define	PRIV_MODULE10		710
#define	PRIV_MODULE11		711
#define	PRIV_MODULE12		712
#define	PRIV_MODULE13		713
#define	PRIV_MODULE14		714
#define	PRIV_MODULE15		715

/*
 * Track end of privilege list.
 */
#define	_PRIV_HIGHEST		716

/*
 * Validate that a named privilege is known by the privilege system.  Invalid
 * privileges presented to the privilege system by a priv_check interface
 * will result in a panic.  This is only approximate due to sparse allocation
 * of the privilege space.
 */
#define	PRIV_VALID(x)	((x) > _PRIV_LOWEST && (x) < _PRIV_HIGHEST)

#ifdef _KERNEL
/*
 * Privilege check interfaces, modeled after historic suser() interfacs, but
 * with the addition of a specific privilege name.  The existing SUSER_* flag
 * name space is used here.  The jail flag will likely be something that can
 * be removed at some point as jail itself will be able to decide if the priv
 * is appropriate, rather than the caller.
 */
struct thread;
struct ucred;
int	priv_check(struct thread *td, int priv);
int	priv_check_cred(struct ucred *cred, int priv, int flags);
#endif

#endif /* !_SYS_PRIV_H_ */
