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

#ifndef PROTECTOR_LIBTSSB_C
#define PROTECTOR_LIBTSSB_C

#include <libssb_common.c>
#include <libtssb.h>

extern unsigned long max_acceptable_dimension_size;

const char err_file_is_changed[] = "File is changed during program execution";
const char err_not_a_valid_tssb[] = "This is not a valid tssb file.";
const char err_out_of_table[] = "Proposed table size is out of acceptable size.";
const char err_parse_fail[] = "An error occured during parsing.";

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

static inline unsigned check_signature(int fd, tssb *u) {
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

static inline bool get_ssb_dimensions(int fd, tssb *u) {
	// above
	// Retrieve amount of cols and rows from TSSB file

	uint32_t rowncol[2];
	ssize_t got = nposix_pread(fd, rowncol, sizeof(rowncol), (off_t) strlen(signatures[u->sizestorage]));
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

#if !defined(POSIXERR_AND_JUMP)
	#define POSIXERR_AND_JUMP(a) {u.errreasonstr = strerror(errno); goto a;}
#endif
// above
// Useful macro for prepare_tssb() function that avoid redundant huge block of code
// Probably could be avoided, but code's beauty is very subjective thing.
// Someone will like it, other person - not.

#define SERR_AND_JUMP(b, a) {u.errreasonstr = b; goto a;}
// above
// Like macro above, but instead of setting POSIX errno string we're using user's string.

tssb prepare_tssb(const char *filename, void *stackmem, size_t msize) {
	// above
	// Prepares required space for working with TSSB file, performs every (probably) possible check/recheck for
	// weird or bad situations that may happen.

	tssb u = {.errreasonstr = NULL};

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

tssb check_tssb(const char *filename) {
	tssb u = {.errreasonstr = NULL};

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

static inline char ***set_2ndptrs(tssb u) {
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

char ***parse_tssb(tssb *p) {
	// above
	// Evaluates parsing of TSSB object and points every pointer from twodimensional array to corresponding block.

	if (p->errreasonstr != NULL) return NULL;
	tssb u = *p;
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
	p->errreasonstr = err_parse_fail;
	return NULL;
}

size_t getssbsize(void *cell, tssb u, size_t *var) {
	cell = (char *) cell - u.sizestorage;
	*var = 0;
	if (IS_BIG_ENDIAN) memcpy((char *) var + (sizeof(size_t) - u.sizestorage), cell, u.sizestorage); else memcpy(var, cell, u.sizestorage);
	return *var;
}

//~ void getu16ssbsize(void *cell, void *var) {
	//~ * (uint16_t *) var = *((uint16_t *) ((char *) cell - sizeof(uint16_t)));
//~ }


#endif // PROTECTOR_LIBTSSB_C
