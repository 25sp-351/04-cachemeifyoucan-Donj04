MAIN = main
TESTER = tester

OBJS = inputreader.o keypair.o rodcutsolver.o vec.o cache.o

LIB = lib-least_recently_used.so lib-first_in_first_out.so
LIB_DEBUG = $(patsubst lib%, libdebug%, $(LIB))

CC = gcc
CFLAGS = -g -Wall -Wextra

USAGE_MSG = echo "command usage: make $(CMD) FILE=\"lengths_file.txt\""


help:
	@echo "help:  display command info"
	@echo "all:   compile all source files and libraries, plus debug versions"
	@echo "build: compile source files and libraries with no debug messages"
	@echo "debug: compile source files and debug libraries"
	@echo "clean: remove generated object files and executables"
	@echo ""
	@echo "to run main program:"
	@echo "   ./$(MAIN) lengths_file.txt [./cache.so]"
	@echo "to run the tester:"
	@echo "   ./$(TESTER) lengths_file.txt [./cache.so]"


# compile commands

all: build debug

build: $(MAIN) $(TESTER) $(LIB)

debug: $(MAIN) $(TESTER) $(LIB_DEBUG)


# compile libraries

lib-%.so: %.c cache.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

libdebug-%.so: %.c cache.h
	$(CC) -shared -fPIC $(CFLAGS) -DDEBUG -o $@ $<


# dependencies

$(MAIN): $(MAIN).o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(MAIN).o $(OBJS)

$(TESTER): $(TESTER).o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(TESTER).o $(OBJS) -lbsd


$(MAIN).o: $(MAIN).c inputreader.h rodcutsolver.h cache.h

$(TESTER).o: $(TESTER).c cache.h rodcutsolver.h vec.h


cache.o: cache.c cache.h

inputreader.o: inputreader.c inputreader.h keypair.h vec.h

keypair.o: keypair.c keypair.h

rodcutsolver.o: rodcutsolver.c rodcutsolver.h keypair.h vec.h

vec.o: vec.c vec.h keypair.h


# remove generated files

clean:
	rm -f $(MAIN) $(TESTER) $(MAIN).o $(TESTER).o $(OBJS) $(LIB) $(LIB_DEBUG)
