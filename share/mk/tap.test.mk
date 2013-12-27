# $FreeBSD$
#
# Logic to build and install TAP-compliant test programs.
#
# This is provided to support existing tests in the FreeBSD source tree
# (particularly those coming from tools/regression/) that comply with the
# Test Anything Protocol.  It should not be used for new tests.

.include <bsd.init.mk>

# List of C, C++ and shell test programs to build.
#
# Programs listed here are built according to the semantics of bsd.prog.mk for
# PROGS, PROGS_CXX and SCRIPTS, respectively.
#
# Test programs registered in this manner are set to be installed into TESTSDIR
# (which should be overriden by the Makefile) and are not required to provide a
# manpage.
TAP_TESTS_C?=
TAP_TESTS_CXX?=
TAP_TESTS_SH?=

.if !empty(TAP_TESTS_C)
PROGS+= ${TAP_TESTS_C}
_TESTS+= ${TAP_TESTS_C}
.for _T in ${TAP_TESTS_C}
BINDIR.${_T}= ${TESTSDIR}
MAN.${_T}?= # empty
SRCS.${_T}?= ${_T}.c
TEST_INTERFACE.${_T}= tap
.endfor
.endif

.if !empty(TAP_TESTS_CXX)
PROGS_CXX+= ${TAP_TESTS_CXX}
_TESTS+= ${TAP_TESTS_CXX}
.for _T in ${TAP_TESTS_CXX}
BINDIR.${_T}= ${TESTSDIR}
MAN.${_T}?= # empty
SRCS.${_T}?= ${_T}.cc
TEST_INTERFACE.${_T}= tap
.endfor
.endif

.if !empty(TAP_TESTS_SH)
SCRIPTS+= ${TAP_TESTS_SH}
_TESTS+= ${TAP_TESTS_SH}
.for _T in ${TAP_TESTS_SH}
SCRIPTSDIR_${_T}= ${TESTSDIR}
TEST_INTERFACE.${_T}= tap
CLEANFILES+= ${_T} ${_T}.tmp
# TODO(jmmv): It seems to me that this SED and SRC functionality should
# exist in bsd.prog.mk along the support for SCRIPTS.  Move it there if
# this proves to be useful within the tests.
TAP_TESTS_SH_SED_${_T}?= # empty
TAP_TESTS_SH_SRC_${_T}?= ${_T}.sh
${_T}: ${TAP_TESTS_SH_SRC_${_T}}
	cat ${.ALLSRC} | sed ${TAP_TESTS_SH_SED_${_T}} >${.TARGET}.tmp
	chmod +x ${.TARGET}.tmp
	mv ${.TARGET}.tmp ${.TARGET}
.endfor
.endif

.include <bsd.test.mk>
