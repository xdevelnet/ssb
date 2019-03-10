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

#include <libtssb.c>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	if (argc < 2) return EXIT_FAILURE;

	ssbu u = prepare_tssb(argv[1], NULL, 0);
	if (u.errreasonstr != NULL) return printf("%s\n", u.errreasonstr);
	char ***table = parse_tssb(&u);
	if (table == NULL) return printf("%s\n", u.errreasonstr);

	size_t row = 0, col = 0;

	while (row < u.rows) {
		write(STDOUT_FILENO, table[row][col], GETU16SSB(table[row][col]));
		write(STDOUT_FILENO, " ", 1);
		col++;
		if (col >= u.cols) {
			row++;
			col = 0;
			write(STDOUT_FILENO, "\n", 1);
		}
	}

	free(u.source);
	return EXIT_SUCCESS;
}
