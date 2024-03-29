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

#define ESSB_CALCULATE_RESIDUE(s) ((s).records_total_size % 4 ? 4 - (s).records_total_size % 4 : 0)
#define ESSB_CALCULATE(structure) ((structure).records_total_size + ESSB_CALCULATE_RESIDUE(structure) + (structure).records_amount * sizeof(int32_t) * 2)
#define ESSB_CALCULATE_FILE(structure) ((structure).records_total_size + ESSB_CALCULATE_RESIDUE(structure) + (structure).records_amount * sizeof(int32_t))

const char essb_signature_0[] = "SSBTEMPLATE0";

const char err_not_a_valid_essb[] = "This is not a valid essb file.";
const char err_invalid_arg[] = "Invalid argument(s).";
const char err_not_supported[] = "This feature is not supported or disabled.";
const char err_essb_reuse[] = "This essb object is already on use. If it's not, memset() it to zero before reuse.";

struct essb_format {
	char signature[strizeof(essb_signature_0)];
	uint32_t records_amount;
	uint32_t records_total_size;
	char records[];
};

static bool check_signature(essb *e, const struct essb_format *format) {
	if (format->records_amount == 0 or format->records_total_size == 0 or
		memcmp(format->signature, essb_signature_0, strizeof(essb_signature_0)) != 0) {
		e->errreasonstr = err_not_a_valid_essb;
		return false;
	}

	e->records_amount = format->records_amount;
	e->records_total_size = format->records_total_size;
	return true;
}
#if defined(SSB_POSIX_0)
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
#endif // SSB_POSIX_0

static void parse(essb *e) {
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
}

uint32_t check_essb(source_type t, const void *source) {
	if (source == NULL) return 0;

	essb e = {.records_total_size = 0};
	switch (t) {
	case SOURCE_FILE:
		close(check_file_signature(&e, source));
		return ESSB_CALCULATE(e);
	case SOURCE_ADDR:
	case SOURCE_ADDR_INPLACE:
		check_signature(&e, source);
		return ESSB_CALCULATE(e);
	case SOURCE_WEB:
	default:
		return 0;
	}
}

bool parse_essb(essb *e, source_type t, const void *source, void *stackmem) {
	if (e == NULL) {
		return false;
	}

	if (source == NULL) {
		e->errreasonstr = err_invalid_arg;
		return false;
	}

	if (e->records) {
		e->errreasonstr = err_essb_reuse;
		return false;
	}

	e->errreasonstr = NULL;

	int fd;
	const struct essb_format *format = source;

	switch (t) {
	case SOURCE_FILE:
#if defined(SSB_POSIX_0)
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
		parse(e);
		return true;
#endif // SSB_POSIX_0
		e->errreasonstr = err_not_supported;
		return false;
	case SOURCE_ADDR:
		if (check_signature(e, format) == false) return false;
		if (stackmem) e->records = stackmem; else e->records = malloc(ESSB_CALCULATE(*e));
		memcpy(e->records, format->records, ESSB_CALCULATE(*e));
		parse(e);
		return true;

	case SOURCE_ADDR_INPLACE:
		if (stackmem == NULL) {
			e->errreasonstr = err_invalid_arg;
			return false;
		}
		if (check_signature(e, format) == false) return false;
		e->records = stackmem;
		parse(e);
		return true;

	case SOURCE_WEB:
		e->errreasonstr = err_not_supported;
		return false;

	default:
		e->errreasonstr = err_invalid_arg;
		return false;
	}
}


#endif // PROTECTOR_LIBESSB_C
