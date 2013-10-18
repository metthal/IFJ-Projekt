# Variables
PROJECT = ini
PROJECT_TEST = $(PROJECT)-test

CC = gcc
RM = rm -f
TAR = tar -czf

STD_C99 = -std=c99
STD_GNU99 = -std=gnu99

CFLAGS = -Wall -Wextra -c
CFLAGS_RELEASE = -O2 -s
CFLAGS_DEBUG = -O0 -g -DDEBUG
CFLAGS_PROFILE = $(CFLAGS_DEBUG) -pg

LFLAGS =
LFLAGS_PROFILE = -pg

TEST_SRC_FILES = $(wildcard test*.c)
TEST_HEADER_FILES = $(wildcard test*.h)
TEST_OBJ_FILES = $(patsubst %.c, %.o, $(wildcard *.c))

SRC_FILES = $(filter-out $(TEST_SRC_FILES), $(wildcard *.c))
HEADER_FILES = $(filter-out $(TEST_HEADER_FILES), $(wildcard *.h))
OBJ_FILES = $(patsubst %.c, %.o, $(SRC_FILES))
TAR_FILE = xmilko01.tar.gz
PACKED_FILES = $(SRC_FILES) $(HEADER_FILES) Makefile
TEMP_FILES = gmon.out

# Targets
release: CFLAGS += $(CFLAGS_RELEASE)
release: build

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: build

profile: CFLAGS += $(CFLAGS_PROFILE)
profile: LFLAGS += $(LFLAGS_PROFILE)
profile: build

callgraph:
	@$(eval COMMIT_HASH := $(shell git log --pretty=format:%h -n 1))
	@$(eval DATE := $(shell date "+%H.%M.%S.%d.%m.%y"))
	@gprof $(PROJECT) > profile.log.$(DATE).$(COMMIT_HASH)

analyze:
	@cppcheck --enable=all $(SRC_FILES) $(HEADER_FILES)

analyzeAll:
	@cppcheck --enable=all $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES)

build: CFLAGS += $(STD_C99)
build: $(OBJ_FILES)
	$(CC) $^ -o $(PROJECT) $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(PROJECT) $(PROJECT_TEST) $(OBJ_FILES) $(TAR_FILE) $(TEMP_FILES) $(TEST_OBJ_FILES)

pack:
	$(TAR) $(TAR_FILE) $(PACKED_FILES)

test: CFLAGS += $(CFLAGS_DEBUG) $(STD_GNU99)
test: $(TEST_OBJ_FILES)
	$(CC) $^ -o $(PROJECT_TEST) $(LFLAGS)

ctags:
	@ctags $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES)

.PHONY: build release debug profile clean pack analyze analyzeAll callgraph test ctags
