/*-
 * Copyright (c) 2016 Jilles Tjoelker
 * All rights reserved.
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/procctl.h>
#include <sys/wait.h>

#include <atf-c.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

ATF_TC_WITHOUT_HEAD(reaper_wait_child_first);
ATF_TC_BODY(reaper_wait_child_first, tc)
{
	pid_t parent, child, grandchild, pid;
	int status, r;
	int pip[2];

	/* Be paranoid. */
	pid = waitpid(-1, NULL, WNOHANG);
	ATF_REQUIRE(pid == -1 && errno == ECHILD);

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	r = pipe(pip);
	ATF_REQUIRE_EQ(0, r);

	child = fork();
	ATF_REQUIRE(child != -1);
	if (child == 0) {
		if (close(pip[1]) != 0)
			_exit(100);
		grandchild = fork();
		if (grandchild == -1)
			_exit(101);
		else if (grandchild == 0) {
			if (read(pip[0], &(uint8_t){ 0 }, 1) != 0)
				_exit(102);
			if (getppid() != parent)
				_exit(103);
			_exit(2);
		} else
			_exit(3);
	}

	pid = waitpid(child, &status, 0);
	ATF_REQUIRE_EQ(child, pid);
	r = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	ATF_CHECK_EQ(3, r);

	r = close(pip[1]);
	ATF_REQUIRE_EQ(0, r);

	pid = waitpid(-1, &status, 0);
	ATF_REQUIRE(pid > 0 && pid != child);
	r = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	ATF_CHECK_EQ(2, r);

	r = close(pip[0]);
	ATF_REQUIRE_EQ(0, r);
}

ATF_TC_WITHOUT_HEAD(reaper_wait_grandchild_first);
ATF_TC_BODY(reaper_wait_grandchild_first, tc)
{
	pid_t parent, child, grandchild, pid;
	int status, r;

	/* Be paranoid. */
	pid = waitpid(-1, NULL, WNOHANG);
	ATF_REQUIRE(pid == -1 && errno == ECHILD);

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	child = fork();
	ATF_REQUIRE(child != -1);
	if (child == 0) {
		grandchild = fork();
		if (grandchild == -1)
			_exit(101);
		else if (grandchild == 0)
			_exit(2);
		else {
			if (waitid(P_PID, grandchild, NULL,
			    WNOWAIT | WEXITED) != 0)
				_exit(102);
			_exit(3);
		}
	}

	pid = waitpid(child, &status, 0);
	ATF_REQUIRE_EQ(child, pid);
	r = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	ATF_CHECK_EQ(3, r);

	pid = waitpid(-1, &status, 0);
	ATF_REQUIRE(pid > 0 && pid != child);
	r = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	ATF_CHECK_EQ(2, r);
}

ATF_TC_WITHOUT_HEAD(reaper_status);
ATF_TC_BODY(reaper_status, tc)
{
	struct procctl_reaper_status st;
	ssize_t sr;
	pid_t parent, child, pid;
	int r, status;
	int pip[2];

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_STATUS, &st);
	ATF_REQUIRE_EQ(0, r);
	ATF_CHECK_EQ(0, st.rs_flags & REAPER_STATUS_OWNED);
	ATF_CHECK(st.rs_children > 0);
	ATF_CHECK(st.rs_descendants > 0);
	ATF_CHECK(st.rs_descendants >= st.rs_children);
	ATF_CHECK(st.rs_reaper != parent);
	ATF_CHECK(st.rs_reaper > 0);

	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	r = procctl(P_PID, parent, PROC_REAP_STATUS, &st);
	ATF_REQUIRE_EQ(0, r);
	ATF_CHECK_EQ(REAPER_STATUS_OWNED,
	    st.rs_flags & (REAPER_STATUS_OWNED | REAPER_STATUS_REALINIT));
	ATF_CHECK_EQ(0, st.rs_children);
	ATF_CHECK_EQ(0, st.rs_descendants);
	ATF_CHECK(st.rs_reaper == parent);
	ATF_CHECK_EQ(-1, st.rs_pid);

	r = pipe(pip);
	ATF_REQUIRE_EQ(0, r);
	child = fork();
	ATF_REQUIRE(child != -1);
	if (child == 0) {
		if (close(pip[0]) != 0)
			_exit(100);
		if (procctl(P_PID, parent, PROC_REAP_STATUS, &st) != 0)
			_exit(101);
		if (write(pip[1], &st, sizeof(st)) != (ssize_t)sizeof(st))
			_exit(102);
		if (procctl(P_PID, getpid(), PROC_REAP_STATUS, &st) != 0)
			_exit(103);
		if (write(pip[1], &st, sizeof(st)) != (ssize_t)sizeof(st))
			_exit(104);
		_exit(0);
	}
	r = close(pip[1]);
	ATF_REQUIRE_EQ(0, r);

	sr = read(pip[0], &st, sizeof(st));
	ATF_REQUIRE_EQ((ssize_t)sizeof(st), sr);
	ATF_CHECK_EQ(REAPER_STATUS_OWNED,
	    st.rs_flags & (REAPER_STATUS_OWNED | REAPER_STATUS_REALINIT));
	ATF_CHECK_EQ(1, st.rs_children);
	ATF_CHECK_EQ(1, st.rs_descendants);
	ATF_CHECK(st.rs_reaper == parent);
	ATF_CHECK_EQ(child, st.rs_pid);
	sr = read(pip[0], &st, sizeof(st));
	ATF_REQUIRE_EQ((ssize_t)sizeof(st), sr);
	ATF_CHECK_EQ(0,
	    st.rs_flags & (REAPER_STATUS_OWNED | REAPER_STATUS_REALINIT));
	ATF_CHECK_EQ(1, st.rs_children);
	ATF_CHECK_EQ(1, st.rs_descendants);
	ATF_CHECK(st.rs_reaper == parent);
	ATF_CHECK_EQ(child, st.rs_pid);

	r = close(pip[0]);
	ATF_REQUIRE_EQ(0, r);
	pid = waitpid(child, &status, 0);
	ATF_REQUIRE_EQ(child, pid);
	ATF_CHECK_EQ(0, status);

	r = procctl(P_PID, parent, PROC_REAP_STATUS, &st);
	ATF_REQUIRE_EQ(0, r);
	ATF_CHECK_EQ(REAPER_STATUS_OWNED,
	    st.rs_flags & (REAPER_STATUS_OWNED | REAPER_STATUS_REALINIT));
	ATF_CHECK_EQ(0, st.rs_children);
	ATF_CHECK_EQ(0, st.rs_descendants);
	ATF_CHECK(st.rs_reaper == parent);
	ATF_CHECK_EQ(-1, st.rs_pid);
}

ATF_TC_WITHOUT_HEAD(reaper_getpids);
ATF_TC_BODY(reaper_getpids, tc)
{
	struct procctl_reaper_pidinfo info[10];
	ssize_t sr;
	pid_t parent, child, grandchild, pid;
	int r, status, childidx;
	int pipa[2], pipb[2];

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(0, info[0].pi_flags & REAPER_PIDINFO_VALID);

	r = pipe(pipa);
	ATF_REQUIRE_EQ(0, r);
	r = pipe(pipb);
	ATF_REQUIRE_EQ(0, r);
	child = fork();
	ATF_REQUIRE(child != -1);
	if (child == 0) {
		if (close(pipa[1]) != 0)
			_exit(100);
		if (close(pipb[0]) != 0)
			_exit(100);
		if (read(pipa[0], &(uint8_t){ 0 }, 1) != 1)
			_exit(101);
		grandchild = fork();
		if (grandchild == -1)
			_exit(102);
		if (grandchild == 0) {
			if (write(pipb[1], &(uint8_t){ 0 }, 1) != 1)
				_exit(103);
			if (read(pipa[0], &(uint8_t){ 0 }, 1) != 1)
				_exit(104);
			_exit(0);
		}
		for (;;)
			pause();
	}
	r = close(pipa[0]);
	ATF_REQUIRE_EQ(0, r);
	r = close(pipb[1]);
	ATF_REQUIRE_EQ(0, r);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(REAPER_PIDINFO_VALID | REAPER_PIDINFO_CHILD,
	    info[0].pi_flags & (REAPER_PIDINFO_VALID | REAPER_PIDINFO_CHILD));
	ATF_CHECK_EQ(child, info[0].pi_pid);
	ATF_CHECK_EQ(child, info[0].pi_subtree);
	ATF_CHECK_EQ(0, info[1].pi_flags & REAPER_PIDINFO_VALID);

	sr = write(pipa[1], &(uint8_t){ 0 }, 1);
	ATF_REQUIRE_EQ(1, sr);
	sr = read(pipb[0], &(uint8_t){ 0 }, 1);
	ATF_REQUIRE_EQ(1, sr);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(REAPER_PIDINFO_VALID,
	    info[0].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(REAPER_PIDINFO_VALID,
	    info[1].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(0, info[2].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(child, info[0].pi_subtree);
	ATF_CHECK_EQ(child, info[1].pi_subtree);
	childidx = info[1].pi_pid == child ? 1 : 0;
	ATF_CHECK_EQ(REAPER_PIDINFO_CHILD,
	    info[childidx].pi_flags & REAPER_PIDINFO_CHILD);
	ATF_CHECK_EQ(0, info[childidx ^ 1].pi_flags & REAPER_PIDINFO_CHILD);
	ATF_CHECK(info[childidx].pi_pid == child);
	grandchild = info[childidx ^ 1].pi_pid;
	ATF_CHECK(grandchild > 0);
	ATF_CHECK(grandchild != child);
	ATF_CHECK(grandchild != parent);

	r = kill(child, SIGTERM);
	ATF_REQUIRE_EQ(0, r);

	pid = waitpid(child, &status, 0);
	ATF_REQUIRE_EQ(child, pid);
	ATF_CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(REAPER_PIDINFO_VALID,
	    info[0].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(0, info[1].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(child, info[0].pi_subtree);
	ATF_CHECK_EQ(REAPER_PIDINFO_CHILD,
	    info[0].pi_flags & REAPER_PIDINFO_CHILD);
	ATF_CHECK_EQ(grandchild, info[0].pi_pid);

	sr = write(pipa[1], &(uint8_t){ 0 }, 1);
	ATF_REQUIRE_EQ(1, sr);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(REAPER_PIDINFO_VALID,
	    info[0].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(0, info[1].pi_flags & REAPER_PIDINFO_VALID);
	ATF_CHECK_EQ(child, info[0].pi_subtree);
	ATF_CHECK_EQ(REAPER_PIDINFO_CHILD,
	    info[0].pi_flags & REAPER_PIDINFO_CHILD);
	ATF_CHECK_EQ(grandchild, info[0].pi_pid);

	pid = waitpid(grandchild, &status, 0);
	ATF_REQUIRE_EQ(grandchild, pid);
	ATF_CHECK_EQ(0, status);

	memset(info, '\0', sizeof(info));
	r = procctl(P_PID, parent, PROC_REAP_GETPIDS,
	    &(struct procctl_reaper_pids){
	    .rp_count = sizeof(info) / sizeof(info[0]),
	    .rp_pids = info
	    });
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(0, info[0].pi_flags & REAPER_PIDINFO_VALID);

	r = close(pipa[1]);
	ATF_REQUIRE_EQ(0, r);
	r = close(pipb[0]);
	ATF_REQUIRE_EQ(0, r);
}

ATF_TC_WITHOUT_HEAD(reaper_kill_badsig);
ATF_TC_BODY(reaper_kill_badsig, tc)
{
	struct procctl_reaper_kill params;
	pid_t parent;
	int r;

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	params.rk_sig = -1;
	params.rk_flags = 0;
	r = procctl(P_PID, parent, PROC_REAP_KILL, &params);
	ATF_CHECK(r == -1 && errno == EINVAL);
}

ATF_TC_WITHOUT_HEAD(reaper_kill_sigzero);
ATF_TC_BODY(reaper_kill_sigzero, tc)
{
	struct procctl_reaper_kill params;
	pid_t parent;
	int r;

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	params.rk_sig = 0;
	params.rk_flags = 0;
	r = procctl(P_PID, parent, PROC_REAP_KILL, &params);
	ATF_CHECK(r == -1 && errno == EINVAL);
}

ATF_TC_WITHOUT_HEAD(reaper_kill_empty);
ATF_TC_BODY(reaper_kill_empty, tc)
{
	struct procctl_reaper_kill params;
	pid_t parent;
	int r;

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	params.rk_sig = SIGTERM;
	params.rk_flags = 0;
	params.rk_killed = 77;
	r = procctl(P_PID, parent, PROC_REAP_KILL, &params);
	ATF_CHECK(r == -1 && errno == ESRCH);
	ATF_CHECK_EQ(0, params.rk_killed);
}

ATF_TC_WITHOUT_HEAD(reaper_kill_normal);
ATF_TC_BODY(reaper_kill_normal, tc)
{
	struct procctl_reaper_kill params;
	ssize_t sr;
	pid_t parent, child, grandchild, pid;
	int r, status;
	int pip[2];

	parent = getpid();
	r = procctl(P_PID, parent, PROC_REAP_ACQUIRE, NULL);
	ATF_REQUIRE_EQ(0, r);

	r = pipe(pip);
	ATF_REQUIRE_EQ(0, r);
	child = fork();
	ATF_REQUIRE(child != -1);
	if (child == 0) {
		if (close(pip[0]) != 0)
			_exit(100);
		grandchild = fork();
		if (grandchild == -1)
			_exit(101);
		if (grandchild == 0) {
			if (write(pip[1], &(uint8_t){ 0 }, 1) != 1)
				_exit(102);
			for (;;)
				pause();
		}
		for (;;)
			pause();
	}
	r = close(pip[1]);
	ATF_REQUIRE_EQ(0, r);

	sr = read(pip[0], &(uint8_t){ 0 }, 1);
	ATF_REQUIRE_EQ(1, sr);

	params.rk_sig = SIGTERM;
	params.rk_flags = 0;
	params.rk_killed = 77;
	r = procctl(P_PID, parent, PROC_REAP_KILL, &params);
	ATF_CHECK_EQ(0, r);
	ATF_CHECK_EQ(2, params.rk_killed);

	pid = waitpid(child, &status, 0);
	ATF_REQUIRE_EQ(child, pid);
	ATF_CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM);

	pid = waitpid(-1, &status, 0);
	ATF_REQUIRE(pid > 0);
	ATF_CHECK(pid != parent);
	ATF_CHECK(pid != child);
	ATF_CHECK(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM);

	r = close(pip[0]);
	ATF_REQUIRE_EQ(0, r);
}

ATF_TP_ADD_TCS(tp)
{

	ATF_TP_ADD_TC(tp, reaper_wait_child_first);
	ATF_TP_ADD_TC(tp, reaper_wait_grandchild_first);
	ATF_TP_ADD_TC(tp, reaper_status);
	ATF_TP_ADD_TC(tp, reaper_getpids);
	ATF_TP_ADD_TC(tp, reaper_kill_badsig);
	ATF_TP_ADD_TC(tp, reaper_kill_sigzero);
	ATF_TP_ADD_TC(tp, reaper_kill_empty);
	ATF_TP_ADD_TC(tp, reaper_kill_normal);
	return (atf_no_error());
}
