/*
 * Copyright (c) 1995 John Birrell <jb@cimlogic.com.au>.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
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
 *
 * $FreeBSD$
 */

#include <errno.h>
#include <pthread.h>

#include "thr_private.h"

static int join_common(pthread_t, void **, const struct timespec *);

__weak_reference(_pthread_join, pthread_join);
__weak_reference(_pthread_timedjoin_np, pthread_timedjoin_np);

static void backout_join(void *arg)
{
	struct pthread *curthread = _get_curthread();
	struct pthread *pthread = (struct pthread *)arg;

	THREAD_LIST_LOCK(curthread);
	pthread->joiner = NULL;
	THREAD_LIST_UNLOCK(curthread);
}

int
_pthread_join(pthread_t pthread, void **thread_return)
{
	return (join_common(pthread, thread_return, NULL));
}

int
_pthread_timedjoin_np(pthread_t pthread, void **thread_return,
	const struct timespec *abstime)
{
	if (abstime == NULL || abstime->tv_sec < 0 || abstime->tv_nsec < 0 ||
	    abstime->tv_nsec >= 1000000000)
		return (EINVAL);

	return (join_common(pthread, thread_return, abstime));
}

static int
join_common(pthread_t pthread, void **thread_return,
	const struct timespec *abstime)
{
	struct pthread *curthread = _get_curthread();
	struct timespec ts, ts2, *tsp;
	void *tmp;
	long state;
	int oldcancel;
	int ret = 0;
 
	if (pthread == NULL)
		return (EINVAL);

	if (pthread == curthread)
		return (EDEADLK);

	THREAD_LIST_LOCK(curthread);
	if ((ret = _thr_find_thread(curthread, pthread, 1)) != 0) {
		ret = ESRCH;
	} else if ((pthread->tlflags & TLFLAGS_DETACHED) != 0) {
		ret = ESRCH;
	} else if (pthread->joiner != NULL) {
		/* Multiple joiners are not supported. */
		ret = ENOTSUP;
	}
	if (ret) {
		THREAD_LIST_UNLOCK(curthread);
		return (ret);
	}
	/* Set the running thread to be the joiner: */
	pthread->joiner = curthread;

	THREAD_LIST_UNLOCK(curthread);

	THR_CLEANUP_PUSH(curthread, backout_join, pthread);
	oldcancel = _thr_cancel_enter(curthread);

	while ((state = pthread->state) != PS_DEAD) {
		if (abstime != NULL) {
			clock_gettime(CLOCK_REALTIME, &ts);
			TIMESPEC_SUB(&ts2, abstime, &ts);
			if (ts2.tv_sec < 0) {
				ret = ETIMEDOUT;
				break;
			}
			tsp = &ts2;
		} else
			tsp = NULL;
		ret = _thr_umtx_wait(&pthread->state, state, tsp);
		if (ret == ETIMEDOUT)
			break;
	}

	_thr_cancel_leave(curthread, oldcancel);
	THR_CLEANUP_POP(curthread, 0);

	if (ret == ETIMEDOUT) {
		THREAD_LIST_LOCK(curthread);
		pthread->joiner = NULL;
		THREAD_LIST_UNLOCK(curthread);
	} else {
		tmp = pthread->ret;
		THREAD_LIST_LOCK(curthread);
		pthread->tlflags |= TLFLAGS_DETACHED;
		pthread->joiner = NULL;
		THR_GCLIST_ADD(pthread);
		THREAD_LIST_UNLOCK(curthread);

		if (thread_return != NULL)
			*thread_return = tmp;
	}
	return (ret);
}
