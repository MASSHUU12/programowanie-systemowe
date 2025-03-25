CC = gcc
CFLAGS = -Wall -Werror -g -O3
TARGET = memory_manager_demo
OBJS = mem_man.o main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

mem_man.o: mem_man.c mem_man.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c mem_man.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)
