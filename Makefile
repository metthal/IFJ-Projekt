# Variables
PROJECT = ini
PROJECT_TEST = $(PROJECT)-test

CC = gcc
RM = rm -rf
TAR = tar -czf

STD_C99 = -std=c99
STD_GNU99 = -std=gnu99

CFLAGS = -Wall -Wextra -c -Wno-unused-function
CFLAGS_RELEASE = -O3 -s
CFLAGS_DEBUG = -O0 -g -DDEBUG
CFLAGS_PROFILE = $(CFLAGS_DEBUG) -pg

LFLAGS =
LFLAGS_PROFILE = -pg

TEST_SRC_FILES = $(wildcard test*.c)
TEST_HEADER_FILES = $(wildcard test*.h)
TEST_OBJ_FILES = $(filter-out main.o, $(patsubst %.c, %.o, $(wildcard *.c)))

SRC_FILES = $(filter-out $(TEST_SRC_FILES), $(wildcard *.c))
HEADER_FILES = $(filter-out $(TEST_HEADER_FILES), $(wildcard *.h))
OBJ_FILES = $(patsubst %.c, %.o, $(SRC_FILES))
TAR_FILE = xmilko01.tgz
PACKED_FILES = $(SRC_FILES) $(HEADER_FILES) Makefile
TEMP_FILES = gmon.out
DOXYFILE = doxyconfig
DOXY_DIR = doxydoc
DOC_FILES = texput.log documentation.dvi documentation.aux documentation.toc documentation.log documentation.out documentation.pdf
DOC_DIR = doc

ANALYZE_FLAGS = --enable=all --std=c99
ANALYZE_FILTER = 2>&1 | grep --color=always -Ev "\-\-check\-config|never used" 1>&2 | true
TEST_BUILD = no
SHELL = /bin/bash

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
	@cppcheck $(ANALYZE_FLAGS) $(SRC_FILES) $(HEADER_FILES) $(ANALYZE_FILTER)

analyzeAll:
	@cppcheck $(ANALYZE_FLAGS) $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES) $(ANALYZE_FILTER)

build: CFLAGS += $(STD_C99)
build: $(OBJ_FILES)
	$(CC) $^ -o $(PROJECT) $(LFLAGS)

%.o: %.c
ifeq ($(TEST_BUILD),yes)
	@$(CC) $(CFLAGS) $< -o $@
else
	$(CC) $(CFLAGS) $< -o $@
endif

clean:
	$(RM) $(PROJECT) $(PROJECT_TEST) $(OBJ_FILES) $(TAR_FILE) $(TEMP_FILES) $(TEST_OBJ_FILES) $(DOXY_DIR) $(DOC_FILES)

pack:
	$(TAR) $(TAR_FILE) $(PACKED_FILES)

test: CFLAGS += $(CFLAGS_DEBUG) $(STD_GNU99)
test: ANALYZE_FLAGS += -q
test:
	@echo -e "\033[1;34mStarting build...\033[00m" && \
		$(MAKE) $(TEST_OBJ_FILES) TEST_BUILD=yes CFLAGS="$(CFLAGS)" >/dev/null && \
		$(CC) $(TEST_OBJ_FILES) -o $(PROJECT_TEST) $(LFLAGS)
	@-echo -e "\n\033[1;34mStarting static analysis...\033[00m" && \
		$(MAKE) analyzeAll ANALYZE_FLAGS="$(ANALYZE_FLAGS)" >/dev/null &&\
		echo -e "\n\033[1;34mStarting tests...\033[00m" && \
		./$(PROJECT_TEST) -ff

ctags:
	@ctags $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES)

doc:
	@command -v doxygen >/dev/null 2>&1 \
		&& { doxygen $(DOXYFILE); true; } \
		|| echo "ini-make: Doxygen not installed."
	@make -sC $(DOC_DIR)/img
	@command -v pdflatex >/dev/null 2>&1 \
		&& pdflatex -interaction=batchmode $(DOC_DIR)/documentation.tex \
		|| { \
			latex $(DOC_DIR)/documentation.tex; \
			dvipdf documentation.dvi; \
		}

statistics:
	@echo -n "Lines of code: " && wc -l $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES) | tail -n 1 | cut -d' ' -f 3
	@echo -n "Size of code: " && du -hsc $(SRC_FILES) $(TEST_SRC_FILES) $(HEADER_FILES) $(TEST_HEADER_FILES) | tail -n 1 | cut -f 1
	@test -e ini && echo -n "Size of executable: " && du -hs ini | cut -f 1 || echo "ini-make: Executable not found."

.PHONY: build release debug profile clean pack analyze analyzeAll callgraph test ctags doc
