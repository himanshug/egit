CC = cc 

CFLAGS=-g -O2 -Wall -Wextra -Isrc/lib -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

LIB_SOURCES=$(wildcard src/lib/*.c)
OBJECTS=$(patsubst %.c,%.o,$(LIB_SOURCES))

BIN_SOURCES=$(wildcard src/main/*.c)
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
