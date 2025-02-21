MAIN = main
TESTER = testcache

OBJS = inputreader.o keypair.o rodcutsolver.o vec.o cache.o
LRU = libmemoize.so
FIFO = libfirst_in_first_out.so

CC = gcc
CFLAGS = -g -Wall -Wextra


build: $(MAIN) $(TESTER) $(LRU) $(FIFO)


run_lru: $(MAIN) $(LRU)
	@./$(MAIN) $(FILE) ./$(LRU) || echo "\nmake command format: make run_lru FILE=\"lengths_file.txt\""


run_fifo: $(MAIN) $(FIFO)
	@./$(MAIN) $(FILE) ./$(FIFO) || echo "\nmake command format: make run_fifo FILE=\"lengths_file.txt\""


$(MAIN): $(MAIN).o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(MAIN).o $(OBJS)


$(TESTER): $(TESTER).o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(TESTER).o $(OBJS) -lbsd


$(MAIN).o: inputreader.h rodcutsolver.h cache.h


$(TESTER).o: cache.h rodcutsolver.h vec.h


lib%.so: %.c cache.h
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<


cache.o: cache.c cache.h

inputreader.o: inputreader.c inputreader.h keypair.h vec.h

keypair.o: keypair.c keypair.h

rodcutsolver.o: rodcutsolver.c rodcutsolver.h keypair.h vec.h

vec.o: vec.c vec.h keypair.h


clean:
	rm -f $(MAIN) $(TESTER) $(MAIN).o $(TESTER).o $(OBJS) $(LRU) $(FIFO)
