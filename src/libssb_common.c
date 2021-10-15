/*
 * Copyright (c) 2021, Xdevelnet (xdevelnet at xdevelnet dot org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PROTECTOR_LIBSSB_COMMON_C
#define PROTECTOR_LIBSSB_COMMON_C

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define SSB_POSIX_0
#endif

#include <stdlib.h> // size_t
#include <stdint.h> // uintblablabla_t
#include <stdbool.h>
#include <string.h>
#include <iso646.h>

#define SSB_ALIGN_FUCKING_POINTERS 8 // When you are operating with pointers which storing in manually allocated space
// and they are using in loop...
// And you (or me?) decides to use -O3, so compiler will aggressively optimize that loop, you may be TOTALLY SCREWED.
// So let's use a few more bytes to avoid completely dumb segfaults that happens out of blue.
// Thank god we have -fsanitize=undefined which is MUCH better in clang rather then in gcc.
// Because when I compiled my code with gcc and -fsantitize=undefined there was no segfaults.

#if !defined(strizeof)
#define strizeof(a) (sizeof(a)-1)
#endif

#if !defined(IS_BIG_ENDIAN)
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)
#endif

#if defined(SSB_POSIX_0)
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

#define POSIX_FAILURE_RETVAL -1

int fstat_getsize(int fd, size_t *size) {
	// above
	// Retrieve size of file from _fd_ file descriptor and save it to _size_ addr.
	// Return value is same as you gonna use fstat()

	struct stat st;
	int rval = fstat(fd, &st);
	if (rval < 0) return rval;
	*size = st.st_size;
	return rval;
}

inline ssize_t nposix_pread(int fd, void *buf, size_t count, off_t offset) {
	// above
	// it's pread() when available
	// If system haven't required standard, then use non-atomic usage of lseek() and read().

#if (_XOPEN_SOURCE) >= 500 || (_POSIX_C_SOURCE) >= 200809L
	return pread(fd, buf, count, offset);
#else
	off_t backup = lseek(fd, 0, SEEK_CUR);
	if (backup < 0 or lseek(fd, offset, SEEK_SET) < 0) return POSIX_FAILURE_RETVAL;
	ssize_t rval = read(fd, buf, count); // for various reasons there is no reason to check return values of these calls
	lseek(fd, backup, SEEK_SET); // e.g. do we need to restore if read() will fail? Ughh... yes? At least an attempt?
	return rval;
#endif
}

#endif

void swapbytes_priv_ssb(void *pv, size_t n) {
	// above
	// Swap bytes in custom length block

	if (!n) return;

	char *p = pv;
	size_t lo, hi;
	for(lo = 0, hi = n - 1; hi > lo; lo++, hi--) {
		char tmp = p[lo];
		p[lo] = p[hi];
		p[hi] = tmp;
	}
}

void *mcalloc(size_t size) {
	// above
	// It is like malloc, but everything is initialized to zero.
	// in other words, mcalloc() is like calloc, but without redundant argument.

	return calloc(sizeof(char), size); // whatever
}

#endif // PROTECTOR_LIBSSB_COMMON_C
