CC = cc 

#-lz is for libz.so providing zlib
#CFLAGS=-g -O2 -Wall -Wextra -Isrc/lib -lz -lcrypto -rdynamic -DNDEBUG $(OPTFLAGS)
CFLAGS=-g -O2 -Isrc/lib -Isrc/main -lz -lcrypto -rdynamic $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

LIB_SOURCES=$(wildcard src/lib/*.c)
LIBS=$(patsubst %.c,%,$(LIB_SOURCES))
OBJECTS=$(patsubst %.c,%.o,$(LIB_SOURCES))

BIN_SOURCES=$(wildcard src/main/*.c)
BINS=$(patsubst %.c,%,$(BIN_SOURCES))

TEST_SOURCES=$(wildcard src/test/*.c)
TESTS=$(patsubst %.c,%,$(TEST_SOURCES))

#all: clean object bin test
all: clean object bin

object:
	for i in $(LIBS); do cc -c $$i.c -o $$i.o $(CFLAGS); done

bin:
	for i in $(BINS); do cc $$i.c -o $$i $(OBJECTS) $(CFLAGS); done

clean:
	rm -rf build $(OBJECTS) $(BINS) $(TESTS)

test: CFLAGS += $(TARGET)
test:
	for i in $(TESTS); do cc $$i.c -o $$i $(OBJECTS) $(CFLAGS); done
	src/test/runtests.sh
