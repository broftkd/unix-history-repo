/*-
 * Copyright (c) 2003-2007 Tim Kientzle
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "test.h"
__FBSDID("$FreeBSD$");

/*
 * Most of these tests are system-independent, though a few depend on
 * features of the local system.  Such tests are conditionalized on
 * the platform name.  On unsupported platforms, only the
 * system-independent features will be tested.
 *
 * No, I don't want to use config.h in the test files because I want
 * the tests to also serve as a check on the correctness of config.h.
 * A mis-configured library build should cause tests to fail.
 */

DEFINE_TEST(test_entry)
{
	char buff[128];
	wchar_t wbuff[128];
	struct stat st;
	struct archive_entry *e, *e2;
	const struct stat *pst;
	unsigned long set, clear; /* For fflag testing. */
	int type, permset, tag, qual; /* For ACL testing. */
	const char *name; /* For ACL testing. */
	const char *xname; /* For xattr tests. */
	const void *xval; /* For xattr tests. */
	size_t xsize; /* For xattr tests. */
	int c;

	assert((e = archive_entry_new()) != NULL);

	/*
	 * Basic set/read tests for all fields.
	 * We should be able to set any field and read
	 * back the same value.
	 *
	 * For methods that "copy" a string, we should be able
	 * to overwrite the original passed-in string without
	 * changing the value in the entry.
	 *
	 * The following tests are ordered alphabetically by the
	 * name of the field.
	 */
	/* atime */
	archive_entry_set_atime(e, 13579, 24680);
	assertEqualInt(archive_entry_atime(e), 13579);
	assertEqualInt(archive_entry_atime_nsec(e), 24680);
	/* ctime */
	archive_entry_set_ctime(e, 13580, 24681);
	assertEqualInt(archive_entry_ctime(e), 13580);
	assertEqualInt(archive_entry_ctime_nsec(e), 24681);
#if ARCHIVE_VERSION_STAMP >= 1009000
	/* dev */
	archive_entry_set_dev(e, 235);
	assertEqualInt(archive_entry_dev(e), 235);
#else
	skipping("archive_entry_dev()");
#endif
	/* devmajor/devminor are tested specially below. */
#if ARCHIVE_VERSION_STAMP >= 1009000
	/* filetype */
	archive_entry_set_filetype(e, AE_IFREG);
	assertEqualInt(archive_entry_filetype(e), AE_IFREG);
#else
	skipping("archive_entry_filetype()");
#endif
	/* fflags are tested specially below */
	/* gid */
	archive_entry_set_gid(e, 204);
	assertEqualInt(archive_entry_gid(e), 204);
	/* gname */
	archive_entry_set_gname(e, "group");
	assertEqualString(archive_entry_gname(e), "group");
	wcscpy(wbuff, L"wgroup");
	archive_entry_copy_gname_w(e, wbuff);
	assertEqualWString(archive_entry_gname_w(e), L"wgroup");
	memset(wbuff, 0, sizeof(wbuff));
	assertEqualWString(archive_entry_gname_w(e), L"wgroup");
	/* hardlink */
	archive_entry_set_hardlink(e, "hardlinkname");
	assertEqualString(archive_entry_hardlink(e), "hardlinkname");
	strcpy(buff, "hardlinkname2");
	archive_entry_copy_hardlink(e, buff);
	assertEqualString(archive_entry_hardlink(e), "hardlinkname2");
	memset(buff, 0, sizeof(buff));
	assertEqualString(archive_entry_hardlink(e), "hardlinkname2");
	wcscpy(wbuff, L"whardlink");
	archive_entry_copy_hardlink_w(e, wbuff);
	assertEqualWString(archive_entry_hardlink_w(e), L"whardlink");
	memset(wbuff, 0, sizeof(wbuff));
	assertEqualWString(archive_entry_hardlink_w(e), L"whardlink");
#if ARCHIVE_VERSION_STAMP >= 1009000
	/* ino */
	archive_entry_set_ino(e, 8593);
	assertEqualInt(archive_entry_ino(e), 8593);
#else
	skipping("archive_entry_ino()");
#endif
	/* link */
	/* TODO: implement these tests. */
	/* mode */
	archive_entry_set_mode(e, 0123456);
	assertEqualInt(archive_entry_mode(e), 0123456);
	/* mtime */
	archive_entry_set_mtime(e, 13581, 24682);
	assertEqualInt(archive_entry_mtime(e), 13581);
	assertEqualInt(archive_entry_mtime_nsec(e), 24682);
#if ARCHIVE_VERSION_STAMP >= 1009000
	/* nlink */
	archive_entry_set_nlink(e, 736);
	assertEqualInt(archive_entry_nlink(e), 736);
#else
	skipping("archive_entry_nlink()");
#endif
	/* pathname */
	archive_entry_set_pathname(e, "path");
	assertEqualString(archive_entry_pathname(e), "path");
	archive_entry_set_pathname(e, "path");
	assertEqualString(archive_entry_pathname(e), "path");
	strcpy(buff, "path2");
	archive_entry_copy_pathname(e, buff);
	assertEqualString(archive_entry_pathname(e), "path2");
	memset(buff, 0, sizeof(buff));
	assertEqualString(archive_entry_pathname(e), "path2");
	wcscpy(wbuff, L"wpath");
	archive_entry_copy_pathname_w(e, wbuff);
	assertEqualWString(archive_entry_pathname_w(e), L"wpath");
	memset(wbuff, 0, sizeof(wbuff));
	assertEqualWString(archive_entry_pathname_w(e), L"wpath");
#if ARCHIVE_VERSION_STAMP >= 1009000
	/* rdev */
	archive_entry_set_rdev(e, 532);
	assertEqualInt(archive_entry_rdev(e), 532);
#else
	skipping("archive_entry_rdev()");
#endif
	/* rdevmajor/rdevminor are tested specially below. */
	/* size */
	archive_entry_set_size(e, 987654321);
	assertEqualInt(archive_entry_size(e), 987654321);
	/* symlink */
	archive_entry_set_symlink(e, "symlinkname");
	assertEqualString(archive_entry_symlink(e), "symlinkname");
#if ARCHIVE_VERSION_STAMP >= 1009000
	strcpy(buff, "symlinkname2");
	archive_entry_copy_symlink(e, buff);
	assertEqualString(archive_entry_symlink(e), "symlinkname2");
	memset(buff, 0, sizeof(buff));
	assertEqualString(archive_entry_symlink(e), "symlinkname2");
#endif
	archive_entry_copy_symlink_w(e, L"wsymlink");
	assertEqualWString(archive_entry_symlink_w(e), L"wsymlink");
	/* uid */
	archive_entry_set_uid(e, 83);
	assertEqualInt(archive_entry_uid(e), 83);
	/* uname */
	archive_entry_set_uname(e, "user");
	assertEqualString(archive_entry_uname(e), "user");
	wcscpy(wbuff, L"wuser");
	archive_entry_copy_gname_w(e, wbuff);
	assertEqualWString(archive_entry_gname_w(e), L"wuser");
	memset(wbuff, 0, sizeof(wbuff));
	assertEqualWString(archive_entry_gname_w(e), L"wuser");

	/* Test fflags interface. */
	archive_entry_set_fflags(e, 0x55, 0xAA);
	archive_entry_fflags(e, &set, &clear);
	failure("Testing set/get of fflags data.");
	assertEqualInt(set, 0x55);
	failure("Testing set/get of fflags data.");
	assertEqualInt(clear, 0xAA);
#ifdef __FreeBSD__
	/* Converting fflags bitmap to string is currently system-dependent. */
	/* TODO: Make this system-independent. */
	assertEqualString(archive_entry_fflags_text(e),
	    "uappnd,nouchg,nodump,noopaque,uunlnk");
	/* TODO: Test archive_entry_copy_fflags_text_w() */
#endif

	/* See test_acl_basic.c for tests of ACL set/get consistency. */

	/* Test xattrs set/get consistency. */
	archive_entry_xattr_add_entry(e, "xattr1", "xattrvalue1", 12);
	assertEqualInt(1, archive_entry_xattr_reset(e));
	assertEqualInt(0, archive_entry_xattr_next(e, &xname, &xval, &xsize));
	assertEqualString(xname, "xattr1");
	assertEqualString(xval, "xattrvalue1");
	assertEqualInt(xsize, 12);
	assertEqualInt(1, archive_entry_xattr_count(e));
	assertEqualInt(ARCHIVE_WARN,
	    archive_entry_xattr_next(e, &xname, &xval, &xsize));
	archive_entry_xattr_clear(e);
	assertEqualInt(0, archive_entry_xattr_reset(e));
	assertEqualInt(ARCHIVE_WARN,
	    archive_entry_xattr_next(e, &xname, &xval, &xsize));
	archive_entry_xattr_add_entry(e, "xattr1", "xattrvalue1", 12);
	assertEqualInt(1, archive_entry_xattr_reset(e));
	archive_entry_xattr_add_entry(e, "xattr2", "xattrvalue2", 12);
	assertEqualInt(2, archive_entry_xattr_reset(e));
	assertEqualInt(0, archive_entry_xattr_next(e, &xname, &xval, &xsize));
	assertEqualInt(0, archive_entry_xattr_next(e, &xname, &xval, &xsize));
	assertEqualInt(ARCHIVE_WARN,
	    archive_entry_xattr_next(e, &xname, &xval, &xsize));


	/*
	 * Test clone() implementation.
	 */

	/* Set values in 'e' */
	archive_entry_clear(e);
	archive_entry_set_atime(e, 13579, 24680);
	archive_entry_set_ctime(e, 13580, 24681);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_dev(e, 235);
#endif
	archive_entry_set_fflags(e, 0x55, 0xAA);
	archive_entry_set_gid(e, 204);
	archive_entry_set_gname(e, "group");
	archive_entry_set_hardlink(e, "hardlinkname");
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_ino(e, 8593);
#endif
	archive_entry_set_mode(e, 0123456);
	archive_entry_set_mtime(e, 13581, 24682);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_nlink(e, 736);
#endif
	archive_entry_set_pathname(e, "path");
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_rdev(e, 532);
#endif
	archive_entry_set_size(e, 987654321);
	archive_entry_set_symlink(e, "symlinkname");
	archive_entry_set_uid(e, 83);
	archive_entry_set_uname(e, "user");
	/* Add an ACL entry. */
	archive_entry_acl_add_entry(e, ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
	    ARCHIVE_ENTRY_ACL_READ, ARCHIVE_ENTRY_ACL_USER, 77, "user77");
	/* Add an extended attribute. */
	archive_entry_xattr_add_entry(e, "xattr1", "xattrvalue", 11);

	/* Make a clone. */
	e2 = archive_entry_clone(e);

	/* Clone should have same contents. */
	assertEqualInt(archive_entry_atime(e2), 13579);
	assertEqualInt(archive_entry_atime_nsec(e2), 24680);
	assertEqualInt(archive_entry_ctime(e2), 13580);
	assertEqualInt(archive_entry_ctime_nsec(e2), 24681);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_dev(e2), 235);
#endif
	archive_entry_fflags(e, &set, &clear);
	assertEqualInt(clear, 0xAA);
	assertEqualInt(set, 0x55);
	assertEqualInt(archive_entry_gid(e2), 204);
	assertEqualString(archive_entry_gname(e2), "group");
	assertEqualString(archive_entry_hardlink(e2), "hardlinkname");
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_ino(e2), 8593);
#endif
	assertEqualInt(archive_entry_mode(e2), 0123456);
	assertEqualInt(archive_entry_mtime(e2), 13581);
	assertEqualInt(archive_entry_mtime_nsec(e2), 24682);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_nlink(e2), 736);
#endif
	assertEqualString(archive_entry_pathname(e2), "path");
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_rdev(e2), 532);
#endif
	assertEqualInt(archive_entry_size(e2), 987654321);
	assertEqualString(archive_entry_symlink(e2), "symlinkname");
	assertEqualInt(archive_entry_uid(e2), 83);
	assertEqualString(archive_entry_uname(e2), "user");
#if ARCHIVE_VERSION_STAMP < 1009000
	skipping("ACL preserved by archive_entry_clone()");
#else
	/* Verify ACL was copied. */
	assertEqualInt(4, c = archive_entry_acl_reset(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS));
	/* First three are standard permission bits. */
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 4);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_USER_OBJ);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 5);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_GROUP_OBJ);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 6);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_OTHER);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	/* Fourth is custom one. */
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, ARCHIVE_ENTRY_ACL_READ);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_USER);
	assertEqualInt(qual, 77);
	assertEqualString(name, "user77");
#endif
#if ARCHIVE_VERSION_STAMP < 1009000
	skipping("xattr data preserved by archive_entry_clone");
#else
	/* Verify xattr was copied. */
	assertEqualInt(1, c = archive_entry_xattr_reset(e2));
	assertEqualInt(0, archive_entry_xattr_next(e2, &xname, &xval, &xsize));
	assertEqualString(xname, "xattr1");
	assertEqualString(xval, "xattrvalue");
	assertEqualInt(xsize, 11);
#endif

	/* Change the original */
	archive_entry_set_atime(e, 13580, 24690);
	archive_entry_set_ctime(e, 13590, 24691);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_dev(e, 245);
#endif
	archive_entry_set_fflags(e, 0x85, 0xDA);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_filetype(e, AE_IFLNK);
#endif
	archive_entry_set_gid(e, 214);
	archive_entry_set_gname(e, "grouper");
	archive_entry_set_hardlink(e, "hardlinkpath");
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_ino(e, 8763);
#endif
	archive_entry_set_mode(e, 0123654);
	archive_entry_set_mtime(e, 18351, 28642);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_nlink(e, 73);
#endif
	archive_entry_set_pathname(e, "pathest");
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_rdev(e, 132);
#endif
	archive_entry_set_size(e, 987456321);
	archive_entry_set_symlink(e, "symlinkpath");
	archive_entry_set_uid(e, 93);
	archive_entry_set_uname(e, "username");
	archive_entry_acl_clear(e);
	archive_entry_xattr_clear(e);

	/* Clone should still have same contents. */
	assertEqualInt(archive_entry_atime(e2), 13579);
	assertEqualInt(archive_entry_atime_nsec(e2), 24680);
	assertEqualInt(archive_entry_ctime(e2), 13580);
	assertEqualInt(archive_entry_ctime_nsec(e2), 24681);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_dev(e2), 235);
#endif
	archive_entry_fflags(e2, &set, &clear);
	assertEqualInt(clear, 0xAA);
	assertEqualInt(set, 0x55);
	assertEqualInt(archive_entry_gid(e2), 204);
	assertEqualString(archive_entry_gname(e2), "group");
	assertEqualString(archive_entry_hardlink(e2), "hardlinkname");
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_ino(e2), 8593);
#endif
	assertEqualInt(archive_entry_mode(e2), 0123456);
	assertEqualInt(archive_entry_mtime(e2), 13581);
	assertEqualInt(archive_entry_mtime_nsec(e2), 24682);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_nlink(e2), 736);
#endif
	assertEqualString(archive_entry_pathname(e2), "path");
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_rdev(e2), 532);
#endif
	assertEqualInt(archive_entry_size(e2), 987654321);
	assertEqualString(archive_entry_symlink(e2), "symlinkname");
	assertEqualInt(archive_entry_uid(e2), 83);
	assertEqualString(archive_entry_uname(e2), "user");
#if ARCHIVE_VERSION_STAMP < 1009000
	skipping("ACL held by clone of archive_entry");
#else
	/* Verify ACL was unchanged. */
	assertEqualInt(4, c = archive_entry_acl_reset(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS));
	/* First three are standard permission bits. */
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 4);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_USER_OBJ);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 5);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_GROUP_OBJ);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, 6);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_OTHER);
	assertEqualInt(qual, -1);
	assertEqualString(name, NULL);
	/* Fourth is custom one. */
	assertEqualInt(0, archive_entry_acl_next(e2,
			   ARCHIVE_ENTRY_ACL_TYPE_ACCESS,
			   &type, &permset, &tag, &qual, &name));
	assertEqualInt(type, ARCHIVE_ENTRY_ACL_TYPE_ACCESS);
	assertEqualInt(permset, ARCHIVE_ENTRY_ACL_READ);
	assertEqualInt(tag, ARCHIVE_ENTRY_ACL_USER);
	assertEqualInt(qual, 77);
	assertEqualString(name, "user77");
#endif
#if ARCHIVE_VERSION_STAMP < 1009000
	skipping("xattr preserved in archive_entry copy");
#else
	/* Verify xattr was unchanged. */
	assertEqualInt(1, archive_entry_xattr_reset(e2));
#endif

	/* Release clone. */
	archive_entry_free(e2);

	/*
	 * Test clear() implementation.
	 */
	archive_entry_clear(e);
	assertEqualInt(archive_entry_atime(e), 0);
	assertEqualInt(archive_entry_atime_nsec(e), 0);
	assertEqualInt(archive_entry_ctime(e), 0);
	assertEqualInt(archive_entry_ctime_nsec(e), 0);
	assertEqualInt(archive_entry_dev(e), 0);
	archive_entry_fflags(e, &set, &clear);
	assertEqualInt(clear, 0);
	assertEqualInt(set, 0);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_filetype(e), 0);
#endif
	assertEqualInt(archive_entry_gid(e), 0);
	assertEqualString(archive_entry_gname(e), NULL);
	assertEqualString(archive_entry_hardlink(e), NULL);
	assertEqualInt(archive_entry_ino(e), 0);
	assertEqualInt(archive_entry_mode(e), 0);
	assertEqualInt(archive_entry_mtime(e), 0);
	assertEqualInt(archive_entry_mtime_nsec(e), 0);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_nlink(e), 0);
#endif
	assertEqualString(archive_entry_pathname(e), NULL);
	assertEqualInt(archive_entry_rdev(e), 0);
	assertEqualInt(archive_entry_size(e), 0);
	assertEqualString(archive_entry_symlink(e), NULL);
	assertEqualInt(archive_entry_uid(e), 0);
	assertEqualString(archive_entry_uname(e), NULL);
	/* ACLs should be cleared. */
	assertEqualInt(archive_entry_acl_count(e, ARCHIVE_ENTRY_ACL_TYPE_ACCESS), 0);
	assertEqualInt(archive_entry_acl_count(e, ARCHIVE_ENTRY_ACL_TYPE_DEFAULT), 0);
	/* Extended attributes should be cleared. */
	assertEqualInt(archive_entry_xattr_count(e), 0);

	/*
	 * Test archive_entry_copy_stat().
	 */
	memset(&st, 0, sizeof(st));
	/* Set all of the standard 'struct stat' fields. */
	st.st_atime = 456789;
	st.st_ctime = 345678;
	st.st_dev = 123;
	st.st_gid = 34;
	st.st_ino = 234;
	st.st_mode = 077777;
	st.st_mtime = 234567;
	st.st_nlink = 345;
	st.st_size = 123456789;
	st.st_uid = 23;
#ifdef __FreeBSD__
	/* On FreeBSD, high-res timestamp data should come through. */
	st.st_atimespec.tv_nsec = 6543210;
	st.st_ctimespec.tv_nsec = 5432109;
	st.st_mtimespec.tv_nsec = 3210987;
#endif
	/* Copy them into the entry. */
	archive_entry_copy_stat(e, &st);
	/* Read each one back separately and compare. */
	assertEqualInt(archive_entry_atime(e), 456789);
	assertEqualInt(archive_entry_ctime(e), 345678);
	assertEqualInt(archive_entry_dev(e), 123);
	assertEqualInt(archive_entry_gid(e), 34);
	assertEqualInt(archive_entry_ino(e), 234);
	assertEqualInt(archive_entry_mode(e), 077777);
	assertEqualInt(archive_entry_mtime(e), 234567);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(archive_entry_nlink(e), 345);
#endif
	assertEqualInt(archive_entry_size(e), 123456789);
	assertEqualInt(archive_entry_uid(e), 23);
#if __FreeBSD__
	/* On FreeBSD, high-res timestamp data should come through. */
	assertEqualInt(archive_entry_atime_nsec(e), 6543210);
	assertEqualInt(archive_entry_ctime_nsec(e), 5432109);
	assertEqualInt(archive_entry_mtime_nsec(e), 3210987);
#endif

	/*
	 * Test archive_entry_stat().
	 */
	/* First, clear out any existing stat data. */
	memset(&st, 0, sizeof(st));
	archive_entry_copy_stat(e, &st);
	/* Set a bunch of fields individually. */
	archive_entry_set_atime(e, 456789, 321);
	archive_entry_set_ctime(e, 345678, 432);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_dev(e, 123);
#endif
	archive_entry_set_gid(e, 34);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_ino(e, 234);
#endif
	archive_entry_set_mode(e, 012345);
	archive_entry_set_mode(e, 012345);
	archive_entry_set_mtime(e, 234567, 543);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_nlink(e, 345);
#endif
	archive_entry_set_size(e, 123456789);
	archive_entry_set_uid(e, 23);
	/* Retrieve a stat structure. */
	assert((pst = archive_entry_stat(e)) != NULL);
	/* Check that the values match. */
	assertEqualInt(pst->st_atime, 456789);
	assertEqualInt(pst->st_ctime, 345678);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(pst->st_dev, 123);
#endif
	assertEqualInt(pst->st_gid, 34);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(pst->st_ino, 234);
#endif
	assertEqualInt(pst->st_mode, 012345);
	assertEqualInt(pst->st_mtime, 234567);
#if ARCHIVE_VERSION_STAMP >= 1009000
	assertEqualInt(pst->st_nlink, 345);
#endif
	assertEqualInt(pst->st_size, 123456789);
	assertEqualInt(pst->st_uid, 23);
#ifdef __FreeBSD__
	/* On FreeBSD, high-res timestamp data should come through. */
	assertEqualInt(pst->st_atimespec.tv_nsec, 321);
	assertEqualInt(pst->st_ctimespec.tv_nsec, 432);
	assertEqualInt(pst->st_mtimespec.tv_nsec, 543);
#endif

	/* Changing any one value should update struct stat. */
	archive_entry_set_atime(e, 456788, 0);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_atime, 456788);
	archive_entry_set_ctime(e, 345677, 431);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_ctime, 345677);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_dev(e, 122);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_dev, 122);
#endif
	archive_entry_set_gid(e, 33);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_gid, 33);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_ino(e, 233);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_ino, 233);
#endif
	archive_entry_set_mode(e, 012344);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_mode, 012344);
	archive_entry_set_mtime(e, 234566, 542);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_mtime, 234566);
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_nlink(e, 344);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_nlink, 344);
#endif
	archive_entry_set_size(e, 123456788);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_size, 123456788);
	archive_entry_set_uid(e, 22);
	assert((pst = archive_entry_stat(e)) != NULL);
	assertEqualInt(pst->st_uid, 22);
	/* We don't need to check high-res fields here. */

	/*
	 * Test dev/major/minor interfaces.  Setting 'dev' or 'rdev'
	 * should change the corresponding major/minor values, and
	 * vice versa.
	 *
	 * The test here is system-specific because it assumes that
	 * makedev(), major(), and minor() are defined in sys/stat.h.
	 * I'm not too worried about it, though, because the code is
	 * simple.  If it works on FreeBSD, it's unlikely to be broken
	 * anywhere else.  Note: The functionality is present on every
	 * platform even if these tests only run some places;
	 * libarchive's more extensive configuration logic should find
	 * the necessary definitions on every platform.
	 */
#if __FreeBSD__
#if ARCHIVE_VERSION_STAMP >= 1009000
	archive_entry_set_dev(e, 0x12345678);
	assertEqualInt(archive_entry_devmajor(e), major(0x12345678));
	assertEqualInt(archive_entry_devminor(e), minor(0x12345678));
	assertEqualInt(archive_entry_dev(e), 0x12345678);
	archive_entry_set_devmajor(e, 0xfe);
	archive_entry_set_devminor(e, 0xdcba98);
	assertEqualInt(archive_entry_devmajor(e), 0xfe);
	assertEqualInt(archive_entry_devminor(e), 0xdcba98);
	assertEqualInt(archive_entry_dev(e), makedev(0xfe, 0xdcba98));
	archive_entry_set_rdev(e, 0x12345678);
	assertEqualInt(archive_entry_rdevmajor(e), major(0x12345678));
	assertEqualInt(archive_entry_rdevminor(e), minor(0x12345678));
	assertEqualInt(archive_entry_rdev(e), 0x12345678);
	archive_entry_set_rdevmajor(e, 0xfe);
	archive_entry_set_rdevminor(e, 0xdcba98);
	assertEqualInt(archive_entry_rdevmajor(e), 0xfe);
	assertEqualInt(archive_entry_rdevminor(e), 0xdcba98);
	assertEqualInt(archive_entry_rdev(e), makedev(0xfe, 0xdcba98));
#endif
#endif

	/* Release the experimental entry. */
	archive_entry_free(e);
}
