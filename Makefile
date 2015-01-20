PROG = eserv

CC = gcc

ifeq ($(shell uname),linux)
SOURCE_FLAGS = -D_POSIX_SOURCE -D_BSD_SOURCE
endif
# -Wall -Werror
CFLAGS = -Os -Wextra \
	 $(SOURCE_FLAGS) -g \
	 -std=gnu99 \
	 -Iinclude
# -D NDEBUG

# CFLAGS += -m32
CFLAGS += -m64
CFLAGS += -D NDEBUG
LIBS = -Llib/eserv
LDFLAGS = -leserv -lpthread -lcrypt

OBJS = main.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

all: $(PROG)

$(PROG): $(OBJS)
	@cd lib/eserv;make all;
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	@cd lib/eserv;make clean;
	rm -f $(PROG) $(OBJS)
