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

#ifndef PROTECTOR_LIBTSSB_H
#define PROTECTOR_LIBTSSB_H

#include <stdint.h>
#include <stdlib.h>

unsigned long max_acceptable_dimension_size = 150; // how BIG any tssb table dimension could be? Modify it if you need.
#define TSSB_CALCULATE(structure) (8 + structure.size + structure.rows * sizeof(void *) + (structure.cols + 1) * structure.rows * sizeof(void *))

typedef struct {
	const char *errreasonstr; // if something BAD happens, here will be pointer to null terminated string with appropriate error reason
	size_t size; // the actual size in byetes of whole tssb file/ojbect. May be used by library user if he's planning to use stack allocation
	size_t rows; // amount of rows, declared in tssb header. Should be used by library user
	size_t cols; // amount of cols, declared in tssb header. Should be used by library user
	size_t sizestorage; // how much bytes we need for storing value of binary sizes. Can be used by user to determine which macro from GETU**SSB family can be used
	char *source; // pointer to memory area for filename and, later, to memory are with tssb. Must not be used by user
} tssb;

tssb check_tssb(const char *filename);
// above
// check if it's ssb file and retrieve size in bytes, number of rows, numbers of cols

tssb prepare_tssb(const char *filename, void *stackmem, size_t msize);
// above
// Evaluates requered preparations before parsing
// Pass non-NULL value to stackmem if you already have memory space for our needs.
//     How much memory will be used from stackmem? Here is its: 8 + u.size + u.rows * sizeof(void *) + (u.cols + 1) * u.rows * sizeof(void *). You also can use TSSB_CALCULATE macros for that.
//     You also must pass msize if you used stackmem because we going to recheck if we will fit.

char ***parse_tssb(tssb *p);
// above
// Returns twodimensional array with pointers memory objects.
// When you are done with this data and you were not passed non-NULL pointer as an stackmem argument from
// previous prepare_tssb() call, use free() on this pointer.

size_t getssbsize(void *cell, tssb u, size_t *var);
// above
// Moves to size_t variable amount of bytes which are stored in choosen cell.
// Returns amount of bytes which are stored in choosen cell.

#define GETU08SSB(cell) (*((uint8_t *) ((char *) (cell) - sizeof(uint8_t))))
#define GETU16SSB(cell) (*((uint16_t *) ((char *) (cell) - sizeof(uint16_t))))
#define GETU32SSB(cell) (*((uint32_t *) ((char *) (cell) - sizeof(uint32_t))))
#define GETU64SSB(cell) (*((uint64_t *) ((char *) (cell) - sizeof(uint64_t))))
// above
// Immediatly retrieve amount of bytes of data, which is stored in choosen cell. You MUST use only correct correct macros,
// otherwise returned value will be definitely incorrect. This set of macroses is useful when you know (at compile time)
// amount of bytes for storing sizes in SSB object. Overall, they are generally should be faster then getssbsize().
// You probably should take care of using these macroses because dereferenced pointers will be not aligned to (sizeof *),
// so it's better to compile libraries with -O3 and your programs with -O2. Or use getssbisize() function instead.


#endif // PROTECTOR_LIBTSSB_H
