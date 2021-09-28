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

#ifndef PROTECTOR_LIBESSB_C
#define PROTECTOR_LIBESSB_C

#include "libssb_common.c"
#include "libessb.h"

#if !defined(strizeof)
#define strizeof(a) (sizeof(a)-1)
#endif

#if !defined(IS_BIG_ENDIAN)
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)
#endif

#if !defined(POSIXERR_AND_JUMP)
#define POSIXERR_AND_JUMP(a) {u.errreasonstr = strerror(errno); goto a;}
#endif

const char essb_signature_0[] = "SSBTEMPLATE0";

const char err_not_a_valid_essb[] = "This is not a valid essb file.";
const char err_invalid_arg[] = "Invalid argument(s).";

struct essb_format {
	char signature[strizeof(essb_signature_0)];
	uint32_t records_amount;
	uint32_t records_total_size;
	char records[];
};

static bool check_signature(essb *e, const struct essb_format *format) {
	if (format->records_amount == 0 || format->records_total_size == 0 ||
		memcmp(format->signature, essb_signature_0, strizeof(essb_signature_0)) != 0) {
		e->errreasonstr = err_not_a_valid_essb;
		return false;
	}

	e->records_amount = format->records_amount;
	e->records_total_size = format->records_total_size;
	return true;
}

static int check_file_signature(essb *e, const void *p) {

	int fd = open(p, O_RDONLY);
	if (fd < 0) {
		e->errreasonstr = strerror(errno);
		return POSIX_FAILURE_RETVAL;
	}

	struct essb_format buffer;
	ssize_t got = read(fd, &buffer, sizeof(buffer));
	if (got < 0) { // lseek(fd, strizeof(essb_signature_0), SEEK_SET) < 0
		e->errreasonstr = strerror(errno);
		close(fd);
		return POSIX_FAILURE_RETVAL;
	}

	if (got < (ssize_t) sizeof(buffer)) {
		e->errreasonstr = err_not_a_valid_essb;
		close(fd);
		return POSIX_FAILURE_RETVAL;
	}

	if (check_signature(e, &buffer) == false) {
		close(fd);
		return POSIX_FAILURE_RETVAL;
	}

	return fd;
}

bool parse_essb(essb *e, source_type t, void *source, void *stackmem) {
	if (e == NULL) {
		return false;
	}

	if (source == NULL) {
		e->errreasonstr = err_invalid_arg;
		return false;
	}

	e->errreasonstr = NULL;

	int fd;
	struct essb_format *format = source;

	switch (t) {
	case SOURCE_FILE:
		fd = check_file_signature(e, format);
		if (fd < 0) return false;
		ssize_t expectations = ESSB_CALCULATE_FILE(*e);
		if (stackmem) e->records = stackmem; else e->records = malloc(ESSB_CALCULATE(*e));
		if (read(fd, e->records, expectations) < expectations) {
			e->errreasonstr = err_not_a_valid_essb;
			close(fd);
			if (stackmem == NULL) {
				free(e->records);
				e->records = NULL;
			}
			return false;
		}
		char *fly = e->records + e->records_total_size;
		fly += ESSB_CALCULATE_RESIDUE(*e);
		e->record_size = (void *) fly;
		fly += e->records_amount * sizeof(int32_t);
		e->record_seek = (void *) fly;
		int32_t total = 0;
		for (unsigned i = 0; i < e->records_amount; i++) {
			e->record_seek[i] = total;
			total += e->record_size[i] < 0 ? - e->record_size[i] : e->record_size[i];
		}
//		setbuf(stdout, NULL);
//		for (unsigned i = 0; i < e->records_amount; i++) {
//			printf("e->record_size[%u] = %d   ;   e->record_seek[%u] = %d \n", i, e->record_size[i], i, e->record_seek[i]);
//		}

		return true;

	case SOURCE_ADDR:
		return 0;
		if (check_signature(e, format) == false) return false;
		if (stackmem) e->records = stackmem; else e->records = malloc(ESSB_CALCULATE(*e));
		memcpy(e->records, format->records, ESSB_CALCULATE(*e));
		return false;

	case SOURCE_ADDR_INPLACE:
		if (stackmem) {
			e->errreasonstr = err_invalid_arg;
			return false;
		}
		if (check_signature(e, format) == false) return false;
		e->records = format->records;

		return true;

	default:
		e->errreasonstr = err_invalid_arg;
		return false;
	}
}


#endif // PROTECTOR_LIBESSB_C
