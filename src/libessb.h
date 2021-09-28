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

#ifndef PROTECTOR_LIBESSB_H
#define PROTECTOR_LIBESSB_H

#define ESSB_CALCULATE_RESIDUE(s) ((s).records_total_size % 4 ? 4 - (s).records_total_size % 4 : 0)
#define ESSB_CALCULATE(structure) ((structure).records_total_size + ESSB_CALCULATE_RESIDUE(structure) + (structure).records_amount * sizeof(int32_t) * 2)
#define ESSB_CALCULATE_FILE(structure) ((structure).records_total_size + ESSB_CALCULATE_RESIDUE(structure) + (structure).records_amount * sizeof(int32_t))

typedef struct {
	const char *errreasonstr; // if something BAD happens, here will be pointer to null terminated string with appropriate error reason
	char *records;
	uint32_t records_amount;
	uint32_t records_total_size;
	int32_t *record_size;
	int32_t *record_seek;
} essb;

typedef enum {SOURCE_FILE, SOURCE_ADDR, SOURCE_ADDR_INPLACE, SOURCE_WEB} source_type;

bool parse_essb(essb *e, source_type t, void *source, void *stackmem);
// above
// evaluate parsing essb file or data.
// Pass non-NULL value to stackmem if you already have memory space for our needs.
//     How much memory will be used from stackmem? User ESSB_CALCULATE() for that.

#endif // PROTECTOR_LIBESSB_H
