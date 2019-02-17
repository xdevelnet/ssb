all:
	cc --std=c99 examples/main.c -g -O3 -o main -Wall -I./src/ -Wall -Wextra -Wno-unused-result -Werror
