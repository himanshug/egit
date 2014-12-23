CC = cc 

#-lz is for libz.so providing zlib
#CFLAGS=-g -O2 -Wall -Wextra -Isrc/lib -lz -lcrypto -rdynamic -DNDEBUG $(OPTFLAGS)
CFLAGS=-g -O2 -Isrc/lib -Isrc/main -lz -lcrypto -rdynamic $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

LIB_SOURCES=$(wildcard src/lib/*.c)
OBJECTS=$(patsubst %.c,%.o,$(LIB_SOURCES))

BIN_SOURCES=src/main/git.c	src/main/git-init-db.c	src/main/zlib.c
BINS=$(patsubst %.c,%,$(BIN_SOURCES))

TEST_SOURCES=$(wildcard src/test/*.c)
TESTS=$(patsubst %.c,%,$(TEST_SOURCES))

TARGET=build/libsrv.a

# The Target Build
all: clean $(TARGET) bin test

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

build:
	@mkdir -p build

bin: CFLAGS += $(TARGET)
bin:
	for i in $(BINS); do cc $$i.c -o $$i $(CFLAGS); done

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(BINS) $(TESTS)

test: CFLAGS += $(TARGET)
test:
	for i in $(TESTS); do cc $$i.c -o $$i $(CFLAGS); done
	src/test/runtests.sh
