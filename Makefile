CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = hashmap_example
OBJS = hashmap.o hashmap_example.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c

hashmap_example.o: hashmap_example.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap_example.c

clean:
	rm -f $(TARGET) $(OBJS)

test:
	$(MAKE) -f Makefile.test test

.PHONY: all clean test

