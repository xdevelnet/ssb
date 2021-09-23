.PHONY: all clean
all:
	cc --std=c99 print_tssb.c -O3 -o print_tssb -I../src/ -Wall -Wextra -Wno-unused-result -Werror
	cc --std=c99 print_tssb_nomalloc.c -O3 -o print_tssb_nomalloc -I../src/ -Wall -Wextra -Wno-unused-result -Werror
clean:
	rm -f print_tssb print_tssb_nomalloc
