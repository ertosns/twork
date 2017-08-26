hdr := $(wildcard src/*.h) $(wildcard *.h)
src := $(wildcard src/*.c) $(wildcard *.c)

PROF := $(HOME)/twork
TMP_PROF := /tmp/twork

all: bison flex tw
gdb: bison flex twdbg

bison:
	bison -d src/parse.y
flex:
	flex src/lex.l

tw: $(hdr) $(src)
	gcc -std=gnu11 -g3 -o twork.out $(src) -lsqlite3 -lm -lX11
	exec ./init.sh

twdbg: $(hdr) $(src)
	mkdir -p /tmp/twork
	gcc -std=gnu11 -g3 -o $(TMP_PROF)/twork.out $(src) -lsqlite3 -lm -lX11
	chmod 777 $(TMP_PROF)/twork.out
	gdb -q --args $(TMP_PROF)/twork.out -d
