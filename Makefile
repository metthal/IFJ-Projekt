# Variables
PROJECT = ini

CC = gcc
RM = rm -f
TAR = tar -czf

CFLAGS = -Wall -Wextra -std=c99 -c
CFLAGS_RELEASE = -O2 -s
CFLAGS_DEBUG = -O0 -g
CFLAGS_PROFILE = $(CFLAGS_DEBUG) -pg

LFLAGS =
LFLAGS_PROFILE = -pg

SRC_FILES = $(wildcard *.c)
OBJ_FILES = $(SRC_FILES:.c=.o)
TAR_FILE = xmilko01.tar.gz

# Targets
release: CFLAGS += $(CFLAGS_RELEASE)
release: build

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: build

profile: CFLAGS += $(CFLAGS_PROFILE)
profile: LFLAGS += $(LFLAGS_PROFILE)
profile: build

build: $(OBJ_FILES)
	$(CC) $^ -o $(PROJECT) $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(PROJECT) $(OBJ_FILES) $(TAR_FILE)

pack:
	$(TAR) $(TAR_FILE) $(SRC_FILES)

.PHONY: build debug profile clean pack
