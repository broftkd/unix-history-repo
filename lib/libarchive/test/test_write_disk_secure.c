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

#define UMASK 022

/*
 * Exercise security checks that should prevent certain
 * writes.
 */

DEFINE_TEST(test_write_disk_secure)
{
	struct archive *a;
	struct archive_entry *ae;
	struct stat st;

	/* Start with a known umask. */
	umask(UMASK);

	/* Create an archive_write_disk object. */
	assert((a = archive_write_disk_new()) != NULL);

	/* Write a regular dir to it. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "dir");
	archive_entry_set_mode(ae, S_IFDIR | 0777);
	assert(0 == archive_write_header(a, ae));
	archive_entry_free(ae);
	assert(0 == archive_write_finish_entry(a));

	/* Write a symlink to the dir above. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link_to_dir");
	archive_entry_set_mode(ae, S_IFLNK | 0777);
	archive_entry_set_symlink(ae, "dir");
	archive_write_disk_set_options(a, 0);
	assert(0 == archive_write_header(a, ae));
	assert(0 == archive_write_finish_entry(a));

	/*
	 * Without security checks, we should be able to
	 * extract a file through the link.
	 */
	assert(archive_entry_clear(ae) != NULL);
	archive_entry_copy_pathname(ae, "link_to_dir/filea");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	assert(0 == archive_write_header(a, ae));
	assert(0 == archive_write_finish_entry(a));

	/* But with security checks enabled, this should fail. */
	assert(archive_entry_clear(ae) != NULL);
	archive_entry_copy_pathname(ae, "link_to_dir/fileb");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_write_disk_set_options(a, ARCHIVE_EXTRACT_SECURE_SYMLINKS);
	failure("Extracting a file through a symlink should fail here.");
	assertEqualInt(ARCHIVE_WARN, archive_write_header(a, ae));
	archive_entry_free(ae);
	assert(0 == archive_write_finish_entry(a));

	/* Create another link. */
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "link_to_dir2");
	archive_entry_set_mode(ae, S_IFLNK | 0777);
	archive_entry_set_symlink(ae, "dir");
	archive_write_disk_set_options(a, 0);
	assert(0 == archive_write_header(a, ae));
	assert(0 == archive_write_finish_entry(a));

	/*
	 * With symlink check and unlink option, it should remove
	 * the link and create the dir.
	 */
	assert(archive_entry_clear(ae) != NULL);
	archive_entry_copy_pathname(ae, "link_to_dir2/filec");
	archive_entry_set_mode(ae, S_IFREG | 0777);
	archive_write_disk_set_options(a, ARCHIVE_EXTRACT_SECURE_SYMLINKS | ARCHIVE_EXTRACT_UNLINK);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
	archive_entry_free(ae);
	assert(0 == archive_write_finish_entry(a));


#if ARCHIVE_API_VERSION > 1
	assert(0 == archive_write_finish(a));
#else
	archive_write_finish(a);
#endif

	/* Test the entries on disk. */
	assert(0 == lstat("dir", &st));
	failure("dir: st.st_mode=%o", st.st_mode);
	assert((st.st_mode & 07777) == 0755);

	assert(0 == lstat("link_to_dir", &st));
	failure("link_to_dir: st.st_mode=%o", st.st_mode);
	assert(S_ISLNK(st.st_mode));
#if HAVE_LCHMOD
	/* Systems that lack lchmod() can't set symlink perms, so skip this. */
	failure("link_to_dir: st.st_mode=%o", st.st_mode);
	assert((st.st_mode & 07777) == 0755);
#endif

	assert(0 == lstat("dir/filea", &st));
	failure("dir/filea: st.st_mode=%o", st.st_mode);
	assert((st.st_mode & 07777) == 0755);

	failure("dir/fileb: This file should not have been created");
	assert(0 != lstat("dir/fileb", &st));

	assert(0 == lstat("link_to_dir2", &st));
	failure("link_to_dir2 should have been re-created as a true dir");
	assert(S_ISDIR(st.st_mode));
	failure("link_to_dir2: Implicit dir creation should obey umask, but st.st_mode=%o", st.st_mode);
	assert((st.st_mode & 07777) == 0755);

	assert(0 == lstat("link_to_dir2/filec", &st));
	assert(S_ISREG(st.st_mode));
	failure("link_to_dir2/filec: st.st_mode=%o", st.st_mode);
	assert((st.st_mode & 07777) == 0755);
}
