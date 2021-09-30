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
#include <stdbool.h>
#include <errno.h>
#include <execinfo.h>

#define RETURN_IF_FAIL(expr)      do {                  \
 if (!(expr))                                           \
 {                                                      \
         fprintf(stderr,                                \
                "file %s: line %d (%s): precondition `%s' failed.", \
                __FILE__,                                           \
                __LINE__,                                           \
                __PRETTY_FUNCTION__,                                \
                #expr);                                             \
         print_stack_trace(2);                                      \
         return;                                                    \
 };               } while(0)
#define RETURN_VAL_IF_FAIL(expr, val)  do {                         \
 if (!(expr))                                                       \
 {                                                                  \
        fprintf(stderr,                                             \
                "file %s: line %d (%s): precondition `%s' failed.",     \
                __FILE__,                                               \
                __LINE__,                                               \
                __PRETTY_FUNCTION__,                                    \
                #expr);                                                 \
         print_stack_trace(2);                                          \
         return val;                                                    \
 };               } while(0)

void print_stack_trace(int fd)
{
	void *array[256];
	backtrace_symbols_fd(array, backtrace (array, 256), fd);
}

#define ABSOLUTE(a) (a < 0 ? (-(a)) : (a))

const char binary[108] = "SSBTEMPLATE0\x09\x00\x00\x00\x34\x00\x00\x00\x46irst text1sttagSCNDSABCD EFGBEBRASKOTINYAKI_TAKI!z\n\x0A\x00\x00\x00\xFA\xFF\xFF\xFF\x04\x00\x00\x00\xFF\xFF\xFF\xFF\xF8\xFF\xFF\xFF\x05\x00\x00\x00\xF0\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01\x00\x00\x00";
//[0] 10   0
//[1] -6  10
//[2]  4  16
//[3] -1  20
//[4] -8  21
//[5]  5  29
//[6] -16 34
//[7] -1  50
//[8]  1  51

#define TESTT(operand, operator, operand2) if(!(operand operator operand2)) do {printf("Condition: %s Evaluated %ld Expected: %ld\n", #operand " " #operator " " #operand2, (long) operand, (long) operand2); retval = false;} while(0)
#define TESTTSTR(tested_str, expected) if (memcmp(tested_str, expected, strizeof(expected)) != 0) do{printf("Condition: %s Expected %s\n", #tested_str , expected); retval = false;} while(0)

static bool consistency_check(essb *e) {
	bool retval = true;
	TESTT(e->records_amount, ==, 9);

	TESTT(e->record_size[0], ==, 10); TESTT(e->record_seek[0], ==,  0); TESTTSTR(ESSB_RETRIEVE(*e, 0), "First text");
	TESTT(e->record_size[1], ==, -6); TESTT(e->record_seek[1], ==, 10); TESTTSTR(ESSB_RETRIEVE(*e, 1), "1sttag");
	TESTT(e->record_size[2], ==,  4); TESTT(e->record_seek[2], ==, 16); TESTTSTR(ESSB_RETRIEVE(*e, 2), "SCND");
	TESTT(e->record_size[3], ==, -1); TESTT(e->record_seek[3], ==, 20); TESTTSTR(ESSB_RETRIEVE(*e, 3), "S");
	TESTT(e->record_size[4], ==, -8); TESTT(e->record_seek[4], ==, 21); TESTTSTR(ESSB_RETRIEVE(*e, 4), "ABCD EFG");
	TESTT(e->record_size[5], ==,  5); TESTT(e->record_seek[5], ==, 29); TESTTSTR(ESSB_RETRIEVE(*e, 5), "BEBRA");
	TESTT(e->record_size[6], ==,-16); TESTT(e->record_seek[6], ==, 34); TESTTSTR(ESSB_RETRIEVE(*e, 6), "SKOTINYAKI_TAKI!");
	TESTT(e->record_size[7], ==, -1); TESTT(e->record_seek[7], ==, 50); TESTTSTR(ESSB_RETRIEVE(*e, 7), "z");
	TESTT(e->record_size[8], ==,  1); TESTT(e->record_seek[8], ==, 51); TESTTSTR(ESSB_RETRIEVE(*e, 8), "\n");
	return retval;
}

static bool prepare_file_and_essb(essb *e) {
	char filename[] = "testdata_essb.ssb";
	int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0600);
	if (fd < 0) {
		printf("Can't create file for testing essb. Reason: %s\n", strerror(errno));
		return false;
	}

	ssize_t got = write(fd, binary, sizeof(binary));
	if (got < 0) {
		printf("Write test data in file for testing essb. Reason: %s\n", strerror(errno));
		unlink(filename);
		return false;
	}

	if (got < (ssize_t) sizeof(binary)) {
		printf("Write test data in file for testing essb. Reason: %s\n", strerror(errno));
		unlink(filename);
		return false;
	}
	close(fd);

	parse_essb(e, SOURCE_FILE, filename, NULL);
	if (e->errreasonstr != NULL) {
		printf("Error during parsing essb: %s\n", e->errreasonstr);
		unlink(filename);
		return false;
	}
	unlink(filename);

	return true;
}

#define TEST(a, foo) do {printf("Test: %s. Result: %s\n", a, (foo) ? "passed" : (retval = EXIT_FAILURE, "failed"));} while(0)

int main(int argc, char **argv) {
	essb e[4] = {};

	int retval = EXIT_SUCCESS;

	if (prepare_file_and_essb(e + 0) == false) goto exit;
	TEST("1", consistency_check(e + 0));

	if (parse_essb(e + 1, SOURCE_ADDR, binary, NULL) == false) {printf("%s\n", e[1].errreasonstr); retval = EXIT_FAILURE; goto exit;}
	TEST("2", consistency_check(e + 0));

	char temp[350];
	if (parse_essb(e + 2, SOURCE_ADDR, binary, temp) == false) {printf("%s\n", e[2].errreasonstr); retval = EXIT_FAILURE; goto exit;}
	TEST("3", consistency_check(e + 0));

	memcpy(temp, binary, sizeof(binary));
	if (parse_essb(e + 3, SOURCE_ADDR_INPLACE, temp, temp) == false) {printf("%s\n", e[2].errreasonstr); retval = EXIT_FAILURE; goto exit;}
	TEST("3", consistency_check(e + 0));

	exit:
	free(e[0].records);
	free(e[1].records);
	// ITS STACK! DON'T free(e[2].records);
	// ITS STACK! DON'T free(e[3].records);
	return retval;
}
