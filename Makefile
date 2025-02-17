SRCS = $(TARGET).c inputreader.c keypair.c rodcutsolver.c vec.c
HDRS = inputreader.h keypair.h rodcutsolver.h vec.h

TARGET = rodcut

OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -g -Wall -Wextra -Werror

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

rodcut.o: rodcut.c inputreader.h keypair.h rodcutsolver.h vec.h

inputreader.o: inputreader.c inputreader.h

keypair.o: keypair.c keypair.h

rodcutsolver.o: rodcutsolver.c rodcutsolver.h

vec.o: vec.c vec.h

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
