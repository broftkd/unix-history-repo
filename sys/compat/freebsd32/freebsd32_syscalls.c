/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD: stable/9/sys/compat/freebsd32/syscalls.master 229500 2012-01-04 16:29:45Z jhb 
 */

const char *freebsd32_syscallnames[] = {
#if !defined(PAD64_REQUIRED) && defined(__powerpc__)
#define PAD64_REQUIRED
#endif
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"open",			/* 5 = open */
	"close",			/* 6 = close */
	"freebsd32_wait4",			/* 7 = freebsd32_wait4 */
	"obs_old",			/* 8 = obsolete old creat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"obs_execv",			/* 11 = obsolete execv */
	"chdir",			/* 12 = chdir */
	"fchdir",			/* 13 = fchdir */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"break",			/* 17 = break */
	"compat4.freebsd32_getfsstat",		/* 18 = freebsd4 freebsd32_getfsstat */
	"compat.freebsd32_lseek",		/* 19 = old freebsd32_lseek */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"unmount",			/* 22 = unmount */
	"setuid",			/* 23 = setuid */
	"getuid",			/* 24 = getuid */
	"geteuid",			/* 25 = geteuid */
	"ptrace",			/* 26 = ptrace */
	"freebsd32_recvmsg",			/* 27 = freebsd32_recvmsg */
	"freebsd32_sendmsg",			/* 28 = freebsd32_sendmsg */
	"freebsd32_recvfrom",			/* 29 = freebsd32_recvfrom */
	"accept",			/* 30 = accept */
	"getpeername",			/* 31 = getpeername */
	"getsockname",			/* 32 = getsockname */
	"access",			/* 33 = access */
	"chflags",			/* 34 = chflags */
	"fchflags",			/* 35 = fchflags */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"compat.freebsd32_stat",		/* 38 = old freebsd32_stat */
	"getppid",			/* 39 = getppid */
	"compat.freebsd32_lstat",		/* 40 = old freebsd32_lstat */
	"dup",			/* 41 = dup */
	"pipe",			/* 42 = pipe */
	"getegid",			/* 43 = getegid */
	"profil",			/* 44 = profil */
	"ktrace",			/* 45 = ktrace */
	"compat.freebsd32_sigaction",		/* 46 = old freebsd32_sigaction */
	"getgid",			/* 47 = getgid */
	"compat.freebsd32_sigprocmask",		/* 48 = old freebsd32_sigprocmask */
	"getlogin",			/* 49 = getlogin */
	"setlogin",			/* 50 = setlogin */
	"acct",			/* 51 = acct */
	"compat.freebsd32_sigpending",		/* 52 = old freebsd32_sigpending */
	"freebsd32_sigaltstack",			/* 53 = freebsd32_sigaltstack */
	"freebsd32_ioctl",			/* 54 = freebsd32_ioctl */
	"reboot",			/* 55 = reboot */
	"revoke",			/* 56 = revoke */
	"symlink",			/* 57 = symlink */
	"readlink",			/* 58 = readlink */
	"freebsd32_execve",			/* 59 = freebsd32_execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"compat.freebsd32_fstat",		/* 62 = old freebsd32_fstat */
	"obs_ogetkerninfo",			/* 63 = obsolete ogetkerninfo */
	"compat.freebsd32_getpagesize",		/* 64 = old freebsd32_getpagesize */
	"msync",			/* 65 = msync */
	"vfork",			/* 66 = vfork */
	"obs_vread",			/* 67 = obsolete vread */
	"obs_vwrite",			/* 68 = obsolete vwrite */
	"sbrk",			/* 69 = sbrk */
	"sstk",			/* 70 = sstk */
	"obs_ommap",			/* 71 = obsolete ommap */
	"vadvise",			/* 72 = vadvise */
	"munmap",			/* 73 = munmap */
	"mprotect",			/* 74 = mprotect */
	"madvise",			/* 75 = madvise */
	"obs_vhangup",			/* 76 = obsolete vhangup */
	"obs_vlimit",			/* 77 = obsolete vlimit */
	"mincore",			/* 78 = mincore */
	"getgroups",			/* 79 = getgroups */
	"setgroups",			/* 80 = setgroups */
	"getpgrp",			/* 81 = getpgrp */
	"setpgid",			/* 82 = setpgid */
	"freebsd32_setitimer",			/* 83 = freebsd32_setitimer */
	"obs_owait",			/* 84 = obsolete owait */
	"swapon",			/* 85 = swapon */
	"freebsd32_getitimer",			/* 86 = freebsd32_getitimer */
	"obs_ogethostname",			/* 87 = obsolete ogethostname */
	"obs_osethostname",			/* 88 = obsolete osethostname */
	"getdtablesize",			/* 89 = getdtablesize */
	"dup2",			/* 90 = dup2 */
	"#91",			/* 91 = getdopt */
	"fcntl",			/* 92 = fcntl */
	"freebsd32_select",			/* 93 = freebsd32_select */
	"#94",			/* 94 = setdopt */
	"fsync",			/* 95 = fsync */
	"setpriority",			/* 96 = setpriority */
	"socket",			/* 97 = socket */
	"connect",			/* 98 = connect */
	"obs_oaccept",			/* 99 = obsolete oaccept */
	"getpriority",			/* 100 = getpriority */
	"obs_osend",			/* 101 = obsolete osend */
	"obs_orecv",			/* 102 = obsolete orecv */
	"compat.freebsd32_sigreturn",		/* 103 = old freebsd32_sigreturn */
	"bind",			/* 104 = bind */
	"setsockopt",			/* 105 = setsockopt */
	"listen",			/* 106 = listen */
	"obs_vtimes",			/* 107 = obsolete vtimes */
	"compat.freebsd32_sigvec",		/* 108 = old freebsd32_sigvec */
	"compat.freebsd32_sigblock",		/* 109 = old freebsd32_sigblock */
	"compat.freebsd32_sigsetmask",		/* 110 = old freebsd32_sigsetmask */
	"compat.freebsd32_sigsuspend",		/* 111 = old freebsd32_sigsuspend */
	"compat.freebsd32_sigstack",		/* 112 = old freebsd32_sigstack */
	"obs_orecvmsg",			/* 113 = obsolete orecvmsg */
	"obs_osendmsg",			/* 114 = obsolete osendmsg */
	"obs_vtrace",			/* 115 = obsolete vtrace */
	"freebsd32_gettimeofday",			/* 116 = freebsd32_gettimeofday */
	"freebsd32_getrusage",			/* 117 = freebsd32_getrusage */
	"getsockopt",			/* 118 = getsockopt */
	"#119",			/* 119 = resuba */
	"freebsd32_readv",			/* 120 = freebsd32_readv */
	"freebsd32_writev",			/* 121 = freebsd32_writev */
	"freebsd32_settimeofday",			/* 122 = freebsd32_settimeofday */
	"fchown",			/* 123 = fchown */
	"fchmod",			/* 124 = fchmod */
	"obs_orecvfrom",			/* 125 = obsolete orecvfrom */
	"setreuid",			/* 126 = setreuid */
	"setregid",			/* 127 = setregid */
	"rename",			/* 128 = rename */
	"obs_otruncate",			/* 129 = obsolete otruncate */
	"obs_ftruncate",			/* 130 = obsolete ftruncate */
	"flock",			/* 131 = flock */
	"mkfifo",			/* 132 = mkfifo */
	"sendto",			/* 133 = sendto */
	"shutdown",			/* 134 = shutdown */
	"socketpair",			/* 135 = socketpair */
	"mkdir",			/* 136 = mkdir */
	"rmdir",			/* 137 = rmdir */
	"freebsd32_utimes",			/* 138 = freebsd32_utimes */
	"obs_4.2",			/* 139 = obsolete 4.2 sigreturn */
	"freebsd32_adjtime",			/* 140 = freebsd32_adjtime */
	"obs_ogetpeername",			/* 141 = obsolete ogetpeername */
	"obs_ogethostid",			/* 142 = obsolete ogethostid */
	"obs_sethostid",			/* 143 = obsolete sethostid */
	"obs_getrlimit",			/* 144 = obsolete getrlimit */
	"obs_setrlimit",			/* 145 = obsolete setrlimit */
	"obs_killpg",			/* 146 = obsolete killpg */
	"setsid",			/* 147 = setsid */
	"quotactl",			/* 148 = quotactl */
	"obs_oquota",			/* 149 = obsolete oquota */
	"obs_ogetsockname",			/* 150 = obsolete ogetsockname */
	"#151",			/* 151 = sem_lock */
	"#152",			/* 152 = sem_wakeup */
	"#153",			/* 153 = asyncdaemon */
	"#154",			/* 154 = nlm_syscall */
	"#155",			/* 155 = nfssvc */
	"compat.freebsd32_getdirentries",		/* 156 = old freebsd32_getdirentries */
	"compat4.freebsd32_statfs",		/* 157 = freebsd4 freebsd32_statfs */
	"compat4.freebsd32_fstatfs",		/* 158 = freebsd4 freebsd32_fstatfs */
	"#159",			/* 159 = nosys */
	"#160",			/* 160 = lgetfh */
	"getfh",			/* 161 = getfh */
	"obs_getdomainname",			/* 162 = obsolete getdomainname */
	"obs_setdomainname",			/* 163 = obsolete setdomainname */
	"obs_uname",			/* 164 = obsolete uname */
	"freebsd32_sysarch",			/* 165 = freebsd32_sysarch */
	"rtprio",			/* 166 = rtprio */
	"#167",			/* 167 = nosys */
	"#168",			/* 168 = nosys */
	"freebsd32_semsys",			/* 169 = freebsd32_semsys */
	"freebsd32_msgsys",			/* 170 = freebsd32_msgsys */
	"freebsd32_shmsys",			/* 171 = freebsd32_shmsys */
	"#172",			/* 172 = nosys */
	"compat6.freebsd32_pread",		/* 173 = freebsd6 freebsd32_pread */
	"compat6.freebsd32_pwrite",		/* 174 = freebsd6 freebsd32_pwrite */
	"#175",			/* 175 = nosys */
	"ntp_adjtime",			/* 176 = ntp_adjtime */
	"#177",			/* 177 = sfork */
	"#178",			/* 178 = getdescriptor */
	"#179",			/* 179 = setdescriptor */
	"#180",			/* 180 = nosys */
	"setgid",			/* 181 = setgid */
	"setegid",			/* 182 = setegid */
	"seteuid",			/* 183 = seteuid */
	"#184",			/* 184 = lfs_bmapv */
	"#185",			/* 185 = lfs_markv */
	"#186",			/* 186 = lfs_segclean */
	"#187",			/* 187 = lfs_segwait */
	"freebsd32_stat",			/* 188 = freebsd32_stat */
	"freebsd32_fstat",			/* 189 = freebsd32_fstat */
	"freebsd32_lstat",			/* 190 = freebsd32_lstat */
	"pathconf",			/* 191 = pathconf */
	"fpathconf",			/* 192 = fpathconf */
	"#193",			/* 193 = nosys */
	"getrlimit",			/* 194 = getrlimit */
	"setrlimit",			/* 195 = setrlimit */
	"freebsd32_getdirentries",			/* 196 = freebsd32_getdirentries */
	"compat6.freebsd32_mmap",		/* 197 = freebsd6 freebsd32_mmap */
	"__syscall",			/* 198 = __syscall */
	"compat6.freebsd32_lseek",		/* 199 = freebsd6 freebsd32_lseek */
	"compat6.freebsd32_truncate",		/* 200 = freebsd6 freebsd32_truncate */
	"compat6.freebsd32_ftruncate",		/* 201 = freebsd6 freebsd32_ftruncate */
	"freebsd32_sysctl",			/* 202 = freebsd32_sysctl */
	"mlock",			/* 203 = mlock */
	"munlock",			/* 204 = munlock */
	"undelete",			/* 205 = undelete */
	"freebsd32_futimes",			/* 206 = freebsd32_futimes */
	"getpgid",			/* 207 = getpgid */
	"#208",			/* 208 = newreboot */
	"poll",			/* 209 = poll */
	"lkmnosys",			/* 210 = lkmnosys */
	"lkmnosys",			/* 211 = lkmnosys */
	"lkmnosys",			/* 212 = lkmnosys */
	"lkmnosys",			/* 213 = lkmnosys */
	"lkmnosys",			/* 214 = lkmnosys */
	"lkmnosys",			/* 215 = lkmnosys */
	"lkmnosys",			/* 216 = lkmnosys */
	"lkmnosys",			/* 217 = lkmnosys */
	"lkmnosys",			/* 218 = lkmnosys */
	"lkmnosys",			/* 219 = lkmnosys */
	"compat7.freebsd32_semctl",		/* 220 = freebsd7 freebsd32_semctl */
	"semget",			/* 221 = semget */
	"semop",			/* 222 = semop */
	"#223",			/* 223 = semconfig */
	"compat7.freebsd32_msgctl",		/* 224 = freebsd7 freebsd32_msgctl */
	"msgget",			/* 225 = msgget */
	"freebsd32_msgsnd",			/* 226 = freebsd32_msgsnd */
	"freebsd32_msgrcv",			/* 227 = freebsd32_msgrcv */
	"shmat",			/* 228 = shmat */
	"compat7.freebsd32_shmctl",		/* 229 = freebsd7 freebsd32_shmctl */
	"shmdt",			/* 230 = shmdt */
	"shmget",			/* 231 = shmget */
	"freebsd32_clock_gettime",			/* 232 = freebsd32_clock_gettime */
	"freebsd32_clock_settime",			/* 233 = freebsd32_clock_settime */
	"freebsd32_clock_getres",			/* 234 = freebsd32_clock_getres */
	"#235",			/* 235 = timer_create */
	"#236",			/* 236 = timer_delete */
	"#237",			/* 237 = timer_settime */
	"#238",			/* 238 = timer_gettime */
	"#239",			/* 239 = timer_getoverrun */
	"freebsd32_nanosleep",			/* 240 = freebsd32_nanosleep */
	"#241",			/* 241 = nosys */
	"#242",			/* 242 = nosys */
	"#243",			/* 243 = nosys */
	"#244",			/* 244 = nosys */
	"#245",			/* 245 = nosys */
	"#246",			/* 246 = nosys */
	"#247",			/* 247 = nosys */
	"#248",			/* 248 = ntp_gettime */
	"#249",			/* 249 = nosys */
	"minherit",			/* 250 = minherit */
	"rfork",			/* 251 = rfork */
	"openbsd_poll",			/* 252 = openbsd_poll */
	"issetugid",			/* 253 = issetugid */
	"lchown",			/* 254 = lchown */
	"freebsd32_aio_read",			/* 255 = freebsd32_aio_read */
	"freebsd32_aio_write",			/* 256 = freebsd32_aio_write */
	"freebsd32_lio_listio",			/* 257 = freebsd32_lio_listio */
	"#258",			/* 258 = nosys */
	"#259",			/* 259 = nosys */
	"#260",			/* 260 = nosys */
	"#261",			/* 261 = nosys */
	"#262",			/* 262 = nosys */
	"#263",			/* 263 = nosys */
	"#264",			/* 264 = nosys */
	"#265",			/* 265 = nosys */
	"#266",			/* 266 = nosys */
	"#267",			/* 267 = nosys */
	"#268",			/* 268 = nosys */
	"#269",			/* 269 = nosys */
	"#270",			/* 270 = nosys */
	"#271",			/* 271 = nosys */
	"getdents",			/* 272 = getdents */
	"#273",			/* 273 = nosys */
	"lchmod",			/* 274 = lchmod */
	"netbsd_lchown",			/* 275 = netbsd_lchown */
	"freebsd32_lutimes",			/* 276 = freebsd32_lutimes */
	"netbsd_msync",			/* 277 = netbsd_msync */
	"nstat",			/* 278 = nstat */
	"nfstat",			/* 279 = nfstat */
	"nlstat",			/* 280 = nlstat */
	"#281",			/* 281 = nosys */
	"#282",			/* 282 = nosys */
	"#283",			/* 283 = nosys */
	"#284",			/* 284 = nosys */
	"#285",			/* 285 = nosys */
	"#286",			/* 286 = nosys */
	"#287",			/* 287 = nosys */
	"#288",			/* 288 = nosys */
	"freebsd32_preadv",			/* 289 = freebsd32_preadv */
	"freebsd32_pwritev",			/* 290 = freebsd32_pwritev */
	"#291",			/* 291 = nosys */
	"#292",			/* 292 = nosys */
	"#293",			/* 293 = nosys */
	"#294",			/* 294 = nosys */
	"#295",			/* 295 = nosys */
	"#296",			/* 296 = nosys */
	"compat4.freebsd32_fhstatfs",		/* 297 = freebsd4 freebsd32_fhstatfs */
	"fhopen",			/* 298 = fhopen */
	"fhstat",			/* 299 = fhstat */
	"modnext",			/* 300 = modnext */
	"freebsd32_modstat",			/* 301 = freebsd32_modstat */
	"modfnext",			/* 302 = modfnext */
	"modfind",			/* 303 = modfind */
	"kldload",			/* 304 = kldload */
	"kldunload",			/* 305 = kldunload */
	"kldfind",			/* 306 = kldfind */
	"kldnext",			/* 307 = kldnext */
	"freebsd32_kldstat",			/* 308 = freebsd32_kldstat */
	"kldfirstmod",			/* 309 = kldfirstmod */
	"getsid",			/* 310 = getsid */
	"setresuid",			/* 311 = setresuid */
	"setresgid",			/* 312 = setresgid */
	"obs_signanosleep",			/* 313 = obsolete signanosleep */
	"freebsd32_aio_return",			/* 314 = freebsd32_aio_return */
	"freebsd32_aio_suspend",			/* 315 = freebsd32_aio_suspend */
	"freebsd32_aio_cancel",			/* 316 = freebsd32_aio_cancel */
	"freebsd32_aio_error",			/* 317 = freebsd32_aio_error */
	"freebsd32_oaio_read",			/* 318 = freebsd32_oaio_read */
	"freebsd32_oaio_write",			/* 319 = freebsd32_oaio_write */
	"freebsd32_olio_listio",			/* 320 = freebsd32_olio_listio */
	"yield",			/* 321 = yield */
	"obs_thr_sleep",			/* 322 = obsolete thr_sleep */
	"obs_thr_wakeup",			/* 323 = obsolete thr_wakeup */
	"mlockall",			/* 324 = mlockall */
	"munlockall",			/* 325 = munlockall */
	"__getcwd",			/* 326 = __getcwd */
	"sched_setparam",			/* 327 = sched_setparam */
	"sched_getparam",			/* 328 = sched_getparam */
	"sched_setscheduler",			/* 329 = sched_setscheduler */
	"sched_getscheduler",			/* 330 = sched_getscheduler */
	"sched_yield",			/* 331 = sched_yield */
	"sched_get_priority_max",			/* 332 = sched_get_priority_max */
	"sched_get_priority_min",			/* 333 = sched_get_priority_min */
	"sched_rr_get_interval",			/* 334 = sched_rr_get_interval */
	"utrace",			/* 335 = utrace */
	"compat4.freebsd32_sendfile",		/* 336 = freebsd4 freebsd32_sendfile */
	"kldsym",			/* 337 = kldsym */
	"freebsd32_jail",			/* 338 = freebsd32_jail */
	"#339",			/* 339 = pioctl */
	"sigprocmask",			/* 340 = sigprocmask */
	"sigsuspend",			/* 341 = sigsuspend */
	"compat4.freebsd32_sigaction",		/* 342 = freebsd4 freebsd32_sigaction */
	"sigpending",			/* 343 = sigpending */
	"compat4.freebsd32_sigreturn",		/* 344 = freebsd4 freebsd32_sigreturn */
	"freebsd32_sigtimedwait",			/* 345 = freebsd32_sigtimedwait */
	"freebsd32_sigwaitinfo",			/* 346 = freebsd32_sigwaitinfo */
	"__acl_get_file",			/* 347 = __acl_get_file */
	"__acl_set_file",			/* 348 = __acl_set_file */
	"__acl_get_fd",			/* 349 = __acl_get_fd */
	"__acl_set_fd",			/* 350 = __acl_set_fd */
	"__acl_delete_file",			/* 351 = __acl_delete_file */
	"__acl_delete_fd",			/* 352 = __acl_delete_fd */
	"__acl_aclcheck_file",			/* 353 = __acl_aclcheck_file */
	"__acl_aclcheck_fd",			/* 354 = __acl_aclcheck_fd */
	"extattrctl",			/* 355 = extattrctl */
	"extattr_set_file",			/* 356 = extattr_set_file */
	"extattr_get_file",			/* 357 = extattr_get_file */
	"extattr_delete_file",			/* 358 = extattr_delete_file */
	"freebsd32_aio_waitcomplete",			/* 359 = freebsd32_aio_waitcomplete */
	"getresuid",			/* 360 = getresuid */
	"getresgid",			/* 361 = getresgid */
	"kqueue",			/* 362 = kqueue */
	"freebsd32_kevent",			/* 363 = freebsd32_kevent */
	"#364",			/* 364 = __cap_get_proc */
	"#365",			/* 365 = __cap_set_proc */
	"#366",			/* 366 = __cap_get_fd */
	"#367",			/* 367 = __cap_get_file */
	"#368",			/* 368 = __cap_set_fd */
	"#369",			/* 369 = __cap_set_file */
	"#370",			/* 370 = nosys */
	"extattr_set_fd",			/* 371 = extattr_set_fd */
	"extattr_get_fd",			/* 372 = extattr_get_fd */
	"extattr_delete_fd",			/* 373 = extattr_delete_fd */
	"__setugid",			/* 374 = __setugid */
	"#375",			/* 375 = nfsclnt */
	"eaccess",			/* 376 = eaccess */
	"#377",			/* 377 = afs_syscall */
	"freebsd32_nmount",			/* 378 = freebsd32_nmount */
	"#379",			/* 379 = kse_exit */
	"#380",			/* 380 = kse_wakeup */
	"#381",			/* 381 = kse_create */
	"#382",			/* 382 = kse_thr_interrupt */
	"#383",			/* 383 = kse_release */
	"#384",			/* 384 = __mac_get_proc */
	"#385",			/* 385 = __mac_set_proc */
	"#386",			/* 386 = __mac_get_fd */
	"#387",			/* 387 = __mac_get_file */
	"#388",			/* 388 = __mac_set_fd */
	"#389",			/* 389 = __mac_set_file */
	"kenv",			/* 390 = kenv */
	"lchflags",			/* 391 = lchflags */
	"uuidgen",			/* 392 = uuidgen */
	"freebsd32_sendfile",			/* 393 = freebsd32_sendfile */
	"#394",			/* 394 = mac_syscall */
	"getfsstat",			/* 395 = getfsstat */
	"statfs",			/* 396 = statfs */
	"fstatfs",			/* 397 = fstatfs */
	"fhstatfs",			/* 398 = fhstatfs */
	"#399",			/* 399 = nosys */
	"ksem_close",			/* 400 = ksem_close */
	"ksem_post",			/* 401 = ksem_post */
	"ksem_wait",			/* 402 = ksem_wait */
	"ksem_trywait",			/* 403 = ksem_trywait */
	"freebsd32_ksem_init",			/* 404 = freebsd32_ksem_init */
	"freebsd32_ksem_open",			/* 405 = freebsd32_ksem_open */
	"ksem_unlink",			/* 406 = ksem_unlink */
	"ksem_getvalue",			/* 407 = ksem_getvalue */
	"ksem_destroy",			/* 408 = ksem_destroy */
	"#409",			/* 409 = __mac_get_pid */
	"#410",			/* 410 = __mac_get_link */
	"#411",			/* 411 = __mac_set_link */
	"extattr_set_link",			/* 412 = extattr_set_link */
	"extattr_get_link",			/* 413 = extattr_get_link */
	"extattr_delete_link",			/* 414 = extattr_delete_link */
	"#415",			/* 415 = __mac_execve */
	"freebsd32_sigaction",			/* 416 = freebsd32_sigaction */
	"freebsd32_sigreturn",			/* 417 = freebsd32_sigreturn */
	"#418",			/* 418 = __xstat */
	"#419",			/* 419 = __xfstat */
	"#420",			/* 420 = __xlstat */
	"freebsd32_getcontext",			/* 421 = freebsd32_getcontext */
	"freebsd32_setcontext",			/* 422 = freebsd32_setcontext */
	"freebsd32_swapcontext",			/* 423 = freebsd32_swapcontext */
	"#424",			/* 424 = swapoff */
	"__acl_get_link",			/* 425 = __acl_get_link */
	"__acl_set_link",			/* 426 = __acl_set_link */
	"__acl_delete_link",			/* 427 = __acl_delete_link */
	"__acl_aclcheck_link",			/* 428 = __acl_aclcheck_link */
	"sigwait",			/* 429 = sigwait */
	"#430",			/* 430 = thr_create; */
	"thr_exit",			/* 431 = thr_exit */
	"thr_self",			/* 432 = thr_self */
	"thr_kill",			/* 433 = thr_kill */
	"freebsd32_umtx_lock",			/* 434 = freebsd32_umtx_lock */
	"freebsd32_umtx_unlock",			/* 435 = freebsd32_umtx_unlock */
	"jail_attach",			/* 436 = jail_attach */
	"extattr_list_fd",			/* 437 = extattr_list_fd */
	"extattr_list_file",			/* 438 = extattr_list_file */
	"extattr_list_link",			/* 439 = extattr_list_link */
	"#440",			/* 440 = kse_switchin */
	"freebsd32_ksem_timedwait",			/* 441 = freebsd32_ksem_timedwait */
	"freebsd32_thr_suspend",			/* 442 = freebsd32_thr_suspend */
	"thr_wake",			/* 443 = thr_wake */
	"kldunloadf",			/* 444 = kldunloadf */
	"audit",			/* 445 = audit */
	"auditon",			/* 446 = auditon */
	"getauid",			/* 447 = getauid */
	"setauid",			/* 448 = setauid */
	"getaudit",			/* 449 = getaudit */
	"setaudit",			/* 450 = setaudit */
	"getaudit_addr",			/* 451 = getaudit_addr */
	"setaudit_addr",			/* 452 = setaudit_addr */
	"auditctl",			/* 453 = auditctl */
	"freebsd32_umtx_op",			/* 454 = freebsd32_umtx_op */
	"freebsd32_thr_new",			/* 455 = freebsd32_thr_new */
	"sigqueue",			/* 456 = sigqueue */
	"freebsd32_kmq_open",			/* 457 = freebsd32_kmq_open */
	"freebsd32_kmq_setattr",			/* 458 = freebsd32_kmq_setattr */
	"freebsd32_kmq_timedreceive",			/* 459 = freebsd32_kmq_timedreceive */
	"freebsd32_kmq_timedsend",			/* 460 = freebsd32_kmq_timedsend */
	"kmq_notify",			/* 461 = kmq_notify */
	"kmq_unlink",			/* 462 = kmq_unlink */
	"abort2",			/* 463 = abort2 */
	"thr_set_name",			/* 464 = thr_set_name */
	"freebsd32_aio_fsync",			/* 465 = freebsd32_aio_fsync */
	"rtprio_thread",			/* 466 = rtprio_thread */
	"#467",			/* 467 = nosys */
	"#468",			/* 468 = nosys */
	"#469",			/* 469 = __getpath_fromfd */
	"#470",			/* 470 = __getpath_fromaddr */
	"sctp_peeloff",			/* 471 = sctp_peeloff */
	"sctp_generic_sendmsg",			/* 472 = sctp_generic_sendmsg */
	"sctp_generic_sendmsg_iov",			/* 473 = sctp_generic_sendmsg_iov */
	"sctp_generic_recvmsg",			/* 474 = sctp_generic_recvmsg */
#ifdef PAD64_REQUIRED
	"freebsd32_pread",			/* 475 = freebsd32_pread */
	"freebsd32_pwrite",			/* 476 = freebsd32_pwrite */
	"freebsd32_mmap",			/* 477 = freebsd32_mmap */
	"freebsd32_lseek",			/* 478 = freebsd32_lseek */
	"freebsd32_truncate",			/* 479 = freebsd32_truncate */
	"freebsd32_ftruncate",			/* 480 = freebsd32_ftruncate */
#else
	"freebsd32_pread",			/* 475 = freebsd32_pread */
	"freebsd32_pwrite",			/* 476 = freebsd32_pwrite */
	"freebsd32_mmap",			/* 477 = freebsd32_mmap */
	"freebsd32_lseek",			/* 478 = freebsd32_lseek */
	"freebsd32_truncate",			/* 479 = freebsd32_truncate */
	"freebsd32_ftruncate",			/* 480 = freebsd32_ftruncate */
#endif
	"thr_kill2",			/* 481 = thr_kill2 */
	"shm_open",			/* 482 = shm_open */
	"shm_unlink",			/* 483 = shm_unlink */
	"cpuset",			/* 484 = cpuset */
#ifdef PAD64_REQUIRED
	"freebsd32_cpuset_setid",			/* 485 = freebsd32_cpuset_setid */
#else
	"freebsd32_cpuset_setid",			/* 485 = freebsd32_cpuset_setid */
#endif
	"freebsd32_cpuset_getid",			/* 486 = freebsd32_cpuset_getid */
	"freebsd32_cpuset_getaffinity",			/* 487 = freebsd32_cpuset_getaffinity */
	"freebsd32_cpuset_setaffinity",			/* 488 = freebsd32_cpuset_setaffinity */
	"faccessat",			/* 489 = faccessat */
	"fchmodat",			/* 490 = fchmodat */
	"fchownat",			/* 491 = fchownat */
	"freebsd32_fexecve",			/* 492 = freebsd32_fexecve */
	"freebsd32_fstatat",			/* 493 = freebsd32_fstatat */
	"freebsd32_futimesat",			/* 494 = freebsd32_futimesat */
	"linkat",			/* 495 = linkat */
	"mkdirat",			/* 496 = mkdirat */
	"mkfifoat",			/* 497 = mkfifoat */
	"mknodat",			/* 498 = mknodat */
	"openat",			/* 499 = openat */
	"readlinkat",			/* 500 = readlinkat */
	"renameat",			/* 501 = renameat */
	"symlinkat",			/* 502 = symlinkat */
	"unlinkat",			/* 503 = unlinkat */
	"posix_openpt",			/* 504 = posix_openpt */
	"#505",			/* 505 = gssd_syscall */
	"freebsd32_jail_get",			/* 506 = freebsd32_jail_get */
	"freebsd32_jail_set",			/* 507 = freebsd32_jail_set */
	"jail_remove",			/* 508 = jail_remove */
	"closefrom",			/* 509 = closefrom */
	"freebsd32_semctl",			/* 510 = freebsd32_semctl */
	"freebsd32_msgctl",			/* 511 = freebsd32_msgctl */
	"freebsd32_shmctl",			/* 512 = freebsd32_shmctl */
	"lpathconf",			/* 513 = lpathconf */
	"cap_new",			/* 514 = cap_new */
	"cap_getrights",			/* 515 = cap_getrights */
	"cap_enter",			/* 516 = cap_enter */
	"cap_getmode",			/* 517 = cap_getmode */
	"#518",			/* 518 = pdfork */
	"#519",			/* 519 = pdkill */
	"#520",			/* 520 = pdgetpid */
	"#521",			/* 521 = pdwait */
	"freebsd32_pselect",			/* 522 = freebsd32_pselect */
	"getloginclass",			/* 523 = getloginclass */
	"setloginclass",			/* 524 = setloginclass */
	"rctl_get_racct",			/* 525 = rctl_get_racct */
	"rctl_get_rules",			/* 526 = rctl_get_rules */
	"rctl_get_limits",			/* 527 = rctl_get_limits */
	"rctl_add_rule",			/* 528 = rctl_add_rule */
	"rctl_remove_rule",			/* 529 = rctl_remove_rule */
	"freebsd32_posix_fallocate",			/* 530 = freebsd32_posix_fallocate */
	"#531",			/* 531 = posix_fadvise */
};
