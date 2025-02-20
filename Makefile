MAIN = main
TESTER = testcache

OBJS = inputreader.o keypair.o rodcutsolver.o vec.o cache.o
LIBS = libmemoize.so

CC = gcc
CFLAGS = -g -Wall -Wextra

all: clean build

build: $(MAIN) $(TESTER) $(LIBS)

lib%.so: %.c cache.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

$(MAIN): $(MAIN).o $(OBJS) $(LIBS)
	$(CC) -o $@ $(CFLAGS) $(MAIN).o $(OBJS) -lbsd

$(TESTER): $(TESTER).o $(OBJS) $(LIBS)
	$(CC) -o $@ $(CFLAGS) $(TESTER).o $(OBJS) -lbsd

$(MAIN).o: inputreader.h rodcutsolver.h cache.h

$(TESTER).o: cache.h rodcutsolver.h vec.h

cache.o: cache.c cache.h

inputreader.o: inputreader.c inputreader.h keypair.h vec.h

keypair.o: keypair.c keypair.h

rodcutsolver.o: rodcutsolver.c rodcutsolver.h keypair.h vec.h

vec.o: vec.c vec.h keypair.h

clean:
	rm -f $(MAIN) $(TESTER) $(MAIN).o $(TESTER).o $(OBJS) $(LIBS)
