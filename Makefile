CC=gcc
DEPS=main.h

ctc: main.c
	cc -o ctc main.c

clean: 
	rm ctc
	rm -rf updates