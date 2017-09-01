hdr := $(wildcard src/*.h) $(wildcard *.h)
src := $(wildcard src/*.c) $(wildcard *.c)

TWORK_PROF	= "/home/ertosns/twork"
TWORK_ALERT 	= "/home/ertosns/twork/media"
TWORK_BACKUP	= "/home/ertosns/twork/backup"
TWORK_DEVELOP	= "/home/ertosns/prj/twork"
CC 		= gcc
#difference between different compilers.
CFLAGS		= -g -O2 #-std=gnu11 -g3
LDFLAGS		= -lsqlite3 -lm -lX11

all: bison flex tw install 
gdb: bison flex twdbg
bison:
	bison -d src/parse.y
flex:
	flex src/lex.l

tw: $(hdr) $(src)
	$(CC)  $(CFLAGS) -o twork $(src) $(LDFLAGS)
twdbg: $(hdr) $(src)
	mkdir -p /tmp/twork
	$(CC) $(CFLAGS) -o $(TWORK_DEVELOP)/twork $(src) $(LDFLAGS)
	chmod 777 $(TWORK_DEVELOP)/twork
	gdb -q --args $(TWORK_DEVELOP)/twork -d
install:
	mkdir -p $PROF $TWORK_ALERT $TWORK_BACKUP
	cp twork $PROF
	chmod 770 $PROF/twork
	ln -f -s ${TWORK_PROF}/twork /usr/local/bin/twork
	cp --backup=t $(TWORK_PROF)/twork.db $(TWORK_BACKUP)
