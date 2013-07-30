CC = gcc
RANLIB = ranlib
PROG = libeserv.a

ifeq ($(shell uname),linux)
SOURCE_FLAGS = -D_POSIX_SOURCE -D_BSD_SOURCE
endif
#-Werror
CFLAGS = -Os -Wall -Wextra \
	$(SOURCE_FLAGS) -g \
	-std=gnu99 \
	-I../../include/eserv

CFLAGS += -m32

LDFLAGS = -lpthread -lcrypt

OBJS = \
    mempool.o \
	hash.o \
	io.o \
	http.o \
	request.o \
	analysis.o \
	entry.o \
	cgi.o \
	misc.o \
	session.o

.c.o:
	$(CC) $(CFLAGS)  -c $*.c

all: $(PROG)

$(PROG): $(OBJS)
	ar rv $@ $?
	$(RANLIB) $@

clean:
	rm -f $(PROG) *.o *~
