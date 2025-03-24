CC = cc
CFLAGS = -Wall -Werror -pthread

all: thread_signals

thread_signals: main.o timer.o
	$(CC) $(CFLAGS) -o thread_signals main.o timer.o

main.o: main.c timer.h
	$(CC) $(CFLAGS) -c main.c

timer.o: timer.c timer.h
	$(CC) $(CFLAGS) -c timer.c

clean:
	rm -f *.o thread_signals
