# @(#)$Id: makefile,v 1.9 2017/10/26 21:47:08 jleffler Exp $
#
# Release Makefile for SCC (Strip C/C++ Comments)
#
# No access to JLSS libraries - use scc.mk for that.

PROGRAM = scc
SOURCE  = errhelp.c filter.c filterio.c stderr.c scc.c
OBJECT  = errhelp.o filter.o filterio.o stderr.o scc.o
DEBRIS  = a.out core *~
OFLAGS  = -O3
WFLAGS  = # -Wall -Wmissing-prototypes -Wstrict-prototypes -std=c11 -pedantic
UFLAGS  = # Set on command line only
IFLAGS  = # -I directory options
DFLAGS  = # -D define options
CFLAGS  = ${OFLAGS} ${UFLAGS} ${WFLAGS} ${IFLAGS} ${DFLAGS}

BASH = bash

.PHONEY: all clean realclean

all: ${PROGRAM}

${PROGRAM}: ${OBJECT}
	${CC} -o $@ ${CFLAGS} ${OBJECT} ${LDFLAGS} ${LDLIBES}

clean:
	rm -f ${OBJECT} ${DEBRIS}

realclean: clean
	rm -f ${PROGRAM} ${SCRIPT}


# DO NOT DELETE THIS LINE or the blank line after it -- make depend uses them.

errhelp.o: errhelp.c
errhelp.o: stderr.h
filter.o: filter.c
filter.o: filter.h
filter.o: stderr.h
scc.o: filter.h
scc.o: posixver.h
scc.o: scc.c
scc.o: stderr.h
stderr.o: stderr.c
stderr.o: stderr.h
