/*
 * Copyright (c) 2019, Xdevelnet (xdevelnet at xdevelnet dot org)
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

#include <libtssb.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iso646.h>
#include <limits.h>

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

extern unsigned long max_acceptable_dimension_size;

const char err_file_is_changed[] = "File is changed during program execution";
const char err_not_a_valid_tssb[] = "This is not a valid tssb file.";
const char err_out_of_table[] = "Proposed table size is out of acceptable size.";
const char err_parse_fail[] = "An error occured during parsing.";
const char err_parse_not_available[] = "You are not able to parse tssb object with errors.";

const char tssb_signature_08bit[] = "SSBTRANSLATI0NS_0";
const char tssb_signature_16bit[] = "SSBTRANSLATI0NS_1";
const char tssb_signature_32bit[] = "SSBTRANSLATI0NS_2";
const char tssb_signature_64bit[] = "SSBTRANSLATI0NS_3";
const char empty_string = '\0';
const char * const signatures[] = { // signatures must be regular null-terminated strings because we're going to use strlen() on it
	&empty_string, // 0
	tssb_signature_08bit, // 1
	tssb_signature_16bit, // 2
	&empty_string, // 3
	tssb_signature_32bit, // 4
	&empty_string, // 5
	&empty_string, // 6        MUHAHAHA, yes, I am genius. Probably. Kind of.
	&empty_string, // 7
	tssb_signature_64bit, // 8
	NULL
};

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

static inline void *mcalloc(size_t size) {
	// above
	// It is like malloc, but everything is initialized to zero.
	// in other words, mcalloc() is like calloc, but without redundant argument.

	return calloc(sizeof(char), size); // whatever
}

static inline ssize_t nposix_pread(int fd, void *buf, size_t count, off_t offset) {
	// above
	// it's pread() when available
	// If system haven't required standard, then use non-atomic usage of lseek() and read().

	const signed posix_fail_val = -1;
	#if (_XOPEN_SOURCE) >= 500 || (_POSIX_C_SOURCE) >= 200809L
		return pread(fd, buf, count, offset);
	#else
		off_t backup = lseek(fd, 0, SEEK_CUR);
		if (backup < 0 or lseek(fd, offset, SEEK_SET) < 0) return posix_fail_val;
		ssize_t rval = read(fd, buf, count); // for various reasons there is no reason to check return values of these calls
		lseek(fd, backup, SEEK_SET); // e.g. do we need to restore if read() will fail? Ughh... yes? At least an attempt?
		return rval;
	#endif
}

static inline unsigned check_signature(int fd, ssbu *u) {
	// above
	// Check TSSB signature.
	// If signature is not correct - 0 will be returned.
	// Otherwise, return value is correspons amount of bytes that
	// will be used for storing sizes (1, 2, 4 or 8).

	char temp[sizeof(tssb_signature_08bit) + sizeof(uint32_t)];
	unsigned current_signature = 0;
	if (nposix_pread(fd, temp, sizeof(temp), 0) < 0) goto posix_error;
	while(signatures[current_signature] != NULL) {
		if (signatures[current_signature] == &empty_string) {current_signature++; continue;}
		if (memcmp(signatures[current_signature], temp, strlen(signatures[current_signature])) == 0) return current_signature;
		current_signature++;
	}
	u->errreasonstr = err_not_a_valid_tssb;
	return 0;
	posix_error:
	u->errreasonstr = strerror(errno);
	return 0;
}

static inline bool get_ssb_dimensions(int fd, ssbu *u) {
	// above
	// Retrieve amount of cols and rows from TSSB file

	uint32_t rowncol[2];
	ssize_t got = nposix_pread(fd, rowncol, sizeof(rowncol), strlen(signatures[u->sizestorage]));
	if (got < 0) {
		u->errreasonstr = strerror(errno);
		return false;
	}
	if ((size_t) got < sizeof(rowncol)) {
		u->errreasonstr = err_not_a_valid_tssb;
		return false;
	}
	if (IS_BIG_ENDIAN) {
		swapbytes_priv_ssb(&rowncol[0], sizeof(uint32_t));
		swapbytes_priv_ssb(&rowncol[1], sizeof(uint32_t));
	}
	if (rowncol[0] > max_acceptable_dimension_size or rowncol[0] == 0 or rowncol[1] > max_acceptable_dimension_size or rowncol[1] == 0) {
		u->errreasonstr = err_out_of_table;
		return false;
	}
	u->rows = rowncol[0];
	u->cols = rowncol[1];
	return true;
}

#define POSIXERR_AND_JUMP(a) {u.errreasonstr = strerror(errno); goto a;}
// above
// Useful macro for prepare_tssb() function that avoid redundant huge block of code
// Probably could be avoided, but code's beauty is very subjective thing.
// Someone will like it, other person - not.

#define SERR_AND_JUMP(b, a) {u.errreasonstr = b; goto a;}
// above
// Like macro above, but instead of setting POSIX errno string we're using user's string.

ssbu prepare_tssb(const char *filename, void *stackmem, size_t msize) {
	// above
	// Prepares required space for working with TSSB file, performs every (probably) possible check/recheck for
	// weird or bad situations that may happen.

	ssbu u = {.errreasonstr = NULL};

	int fd = open(filename, O_RDONLY);
	if (fd < 0) POSIXERR_AND_JUMP(ret);
	if (fstat_getsize(fd, &u.size) < 0) POSIXERR_AND_JUMP(reclose);
	u.sizestorage = check_signature(fd, &u);
	if (u.sizestorage == 0) goto reclose;
	if (get_ssb_dimensions(fd, &u) == false) goto reclose;
	size_t expected_amount_of_space = SSB_ALIGN_FUCKING_POINTERS + u.size + u.rows * sizeof(void *) + (u.cols + 1) * u.rows * sizeof(void *);
	char *data;
	if (stackmem == NULL) {
		data = mcalloc(expected_amount_of_space);
		if (data == NULL) POSIXERR_AND_JUMP(reclose);
	} else {
		if (msize != expected_amount_of_space) SERR_AND_JUMP(err_file_is_changed, reclose);
		data = stackmem;
	}
	lseek(fd, 0, SEEK_CUR);
	ssize_t got = read(fd, data, u.size);
	if (got < 0) POSIXERR_AND_JUMP(refreeclose);
	if (u.size != (size_t) got or read(fd, &u.source, sizeof(void *)) > 0) SERR_AND_JUMP(err_file_is_changed, refreeclose);
	close(fd);
	u.source = data;
	return u;

	refreeclose: if (stackmem == NULL) free(data);
	reclose: close(fd);
	ret: return u;
}

ssbu check_tssb(const char *filename) {
	ssbu u = {.errreasonstr = NULL};

	int fd = open(filename, O_RDONLY);
	if (fd < 0) POSIXERR_AND_JUMP(ret);
	if (fstat_getsize(fd, &u.size) < 0) POSIXERR_AND_JUMP(reclose);
	u.sizestorage = check_signature(fd, &u);
	if (u.sizestorage == 0) goto reclose;
	get_ssb_dimensions(fd, &u);
	reclose: close(fd);
	ret: return u;
}

static inline void *alignto(void *addr, size_t alignment) {
	// above
	// Move addr to next addres which is a multiple of alignment
	char *a = addr;
	a += alignment - ((size_t )a % alignment);
	return a;
}

static inline char ***set_2ndptrs(ssbu u) {
	// above
	// Handy procedure that sets pointers for first dimension for twodimensional array. Sets last element of second
	// dimension to NULL

	char ***t = (char ***) (u.source + u.size);
	// the address in t variable is not aligned. Read commends at SSB_ALIGN_FUCKING_POINTERS macro description if you
	// want to know why i'm going to align it. Not because i'm byte spender or douchebag.
	t = alignto(t, SSB_ALIGN_FUCKING_POINTERS);
	size_t rowscount = 0;

	while(rowscount < u.rows) {
		t[rowscount] = (char **) (t + u.rows + rowscount * (u.cols + 1));
		t[rowscount][u.cols] = NULL;
		rowscount++;
	}

	return t;
}

char ***parse_tssb(ssbu *tssb) {
	// above
	// Evaluates parsing of TSSB object and points every pointer from twodimensional array to corresponding block.

	if (tssb->errreasonstr != NULL) return NULL;
	ssbu u = *tssb;
	const uint8_t newline_sigil[8] = {UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX};
	char ***t = set_2ndptrs(u);
	size_t currentpos = strlen(signatures[u.sizestorage]) + sizeof(uint32_t) + sizeof(uint32_t);
	if (memcmp(u.source + currentpos, newline_sigil, u.sizestorage) != 0) return NULL;
	size_t bsize = 0, a = 0, b = 0; a--;

	while(currentpos < u.size) {
		if (memcmp(u.source + currentpos, newline_sigil, u.sizestorage) == 0) {
			a++;
			if (a >= u.rows) goto parse_failure;
			currentpos += u.sizestorage;
			b = 0;
			continue;
		}
		if (b >= u.cols) goto parse_failure;
		if (IS_BIG_ENDIAN) {
			swapbytes_priv_ssb(u.source + currentpos, u.sizestorage);
			memcpy((char *)  &bsize + (sizeof(size_t) - u.sizestorage), u.source + currentpos, u.sizestorage );
		} else memcpy(&bsize, u.source + currentpos, u.sizestorage);
		currentpos += u.sizestorage;
		t[a][b++] = u.source + currentpos;
		currentpos += bsize;
	}

	return t;
	parse_failure:
	tssb->errreasonstr = err_parse_fail;
	return NULL;
}

size_t getssbsize(void *cell, ssbu u, size_t *var) {
	cell = (char *) cell - u.sizestorage;
	*var = 0;
	if (IS_BIG_ENDIAN) memcpy((char *) var + (sizeof(size_t) - u.sizestorage), cell, u.sizestorage); else memcpy(var, cell, u.sizestorage);
	return *var;
}

//~ void getu16ssbsize(void *cell, void *var) {
	//~ * (uint16_t *) var = *((uint16_t *) ((char *) cell - sizeof(uint16_t)));
//~ }
