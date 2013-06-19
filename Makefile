PROG = eserv

CC = gcc

ifeq ($(shell uname),linux)
SOURCE_FLAGS = -D_POSIX_SOURCE -D_BSD_SOURCE
endif

CFLAGS = -Os -Wall -Wextra -Werror \
	 $(SOURCE_FLAGS) -g \
	 -std=gnu99
# -D NDEBUG

CFLAGS += -m32

LDFLAGS = -lpthread

OBJS = \
	libeserv/mempool.o \
	libeserv/hash.o \
	libeserv/io.o \
	libeserv/http.o \
	libeserv/request.o \
	libeserv/analysis.o \
	libeserv/entry.o \
	libeserv/cgi.o \
	libeserv/misc.o \
	main.o \
	\
	cgi_custom.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)
