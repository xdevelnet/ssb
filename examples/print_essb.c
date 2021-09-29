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

#include <libessb.c>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ABSOLUTE(a) (a < 0 ? (-(a)) : (a))

int main(int argc, char **argv) {
	if (argc < 2) return EXIT_FAILURE;

	essb e[1];

	parse_essb(e, SOURCE_FILE, argv[1], NULL);
	if (e->errreasonstr != NULL) {
		printf("Error during parsing essb: %s", e->errreasonstr);
		return EXIT_FAILURE;
	}

	setbuf(stdout, NULL);
	for (unsigned i = 0; i < e->records_amount; i++) {
		printf("e->record_size[%u] = %d ; e->record_seek[%u] = %d ; Data: ", i, e->record_size[i], i, e->record_seek[i]);
		write(STDOUT_FILENO, &e->records[e->record_seek[i]], ABSOLUTE(e->record_size[i]));
		write(STDOUT_FILENO, "\n", 1);
	}

	free(e->records);

	return EXIT_SUCCESS;
}
