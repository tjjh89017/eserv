PROG = eserv

CC = gcc

ifeq ($(shell uname),linux)
SOURCE_FLAGS = -D_POSIX_SOURCE -D_BSD_SOURCE
endif

CFLAGS = -Os -Wall -Wextra -Werror \
	 $(SOURCE_FLAGS) -g \
	 -std=gnu99 \
	 -Iinclude
# -D NDEBUG

CFLAGS += -m32
LIBS = -Llib/eserv
LDFLAGS = -leserv -lpthread

OBJS = main.o \
	cgi_custom.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)
