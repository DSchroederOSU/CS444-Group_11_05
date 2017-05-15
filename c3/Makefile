#default is to compile and create the pdf
default: all

all: prog


#~~~~~~~~~~~~~~~~~~~~ C ~~~~~~~~~~~~~~~~~~~~#
CC = gcc
CFLAGS = -Wall -std=gnu99 -openmp -O3 -g -I.
#overkill on the flags, but that doesn't hurt anything
LDFLAGS = -lrt -lpthread 
CTARGET = Prog
CSOURCE = c3.c
CINCLUDES = 


prog: ${CSOURCE} ${CINCLUDES}
	${CC} ${CFLAGS} ${CSOURCE} -o ${CTARGET} ${LDFLAGS}

debug: ${CSOURCE} ${CINCLUDES}
	${CC} ${CFLAGS} ${CSOURCE} -o ${CTARGET} ${LDFLAGS} -DDEBUG

clean:
	rm -f ${CTARGET}


