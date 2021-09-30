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

typedef struct {
	const char *errreasonstr; // if something BAD happens, here will be pointer to null terminated string with appropriate error reason
	char *records;
	uint32_t records_amount;
	uint32_t records_total_size;
	int32_t *record_size;
	int32_t *record_seek;
} essb;

#define ESSB_RETRIEVE(essb_object, number) ((essb_object).records+(essb_object).record_seek[number])

typedef enum {SOURCE_FILE, SOURCE_ADDR, SOURCE_ADDR_INPLACE, SOURCE_WEB} source_type;

bool parse_essb(essb *e, source_type t, const void *source, void *stackmem);
// above
// depending on source_type, this function evaluates different actions
// If SOURCE_FILE:         Read data from file and place parsed data to freshly allocated memory.
//                         If _stackmem_ has passed, place it there instead of allocation
// If SOURCE_ADDR:         Just like SOURCE_FILE, but read data from memory address instead of file.
//                         Therefore, instead of filename, pass memory address to _source_ pointer
// If SOURCE_ADDR_INPLACE: Evaluate reading AND placing parsed result from/in same memory area
//                         In order to use this feature, pass same pointer to _source_ and to _stackmem_
// If SOURCE_WEB:          Just like SOURCE_FILE, but instead of reading from file, attempt to download
//                         template from http or https resource. None of file will be written, library
//                         will attempt to use as less memory as possible.
// All parsing results are available through essb structure, which must be zeroed and it's address must
// be passed to parse_essb()
//
// If you want to know how much memory do you need to pass for _stackmem_, use check_essb() for that

uint32_t check_essb(source_type t, const void *source);
// above
// evaluates reading from source just to retrieve amount of bytes that you'll need for stackmem memory

#endif // PROTECTOR_LIBESSB_H
