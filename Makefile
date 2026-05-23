# SPDX-License-Identifier: GPL-2.0
#
# Snake game Makefile
#

TARGET   = snake
SRCS     = snake.c
OBJS     = $(SRCS:.c=.o)
CC       = gcc
RM       = rm -f
INSTALL  = install
PREFIX   = /usr/local
BINDIR   = $(DESTDIR)$(PREFIX)/bin

CFLAGS   = -Wall -Wextra -O2
LDFLAGS  = -lncurses

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build (default)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJS)

distclean: clean

install: $(TARGET)
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -m 755 $(TARGET) $(BINDIR)

uninstall:
	$(RM) $(BINDIR)/$(TARGET)

.PHONY: all clean distclean install uninstall debug