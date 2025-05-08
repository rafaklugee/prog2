CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c99
TARGET = vinac
OBJS = main.o lz.o lista.o vinac.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c vinac.h lista.h
	$(CC) $(CFLAGS) -c main.c

vinac.o: vinac.c vinac.h lz.h lista.h
	$(CC) $(CFLAGS) -c vinac.c

lz.o: lz.c lz.h
	$(CC) $(CFLAGS) -c lz.c

lista.o: lista.c lista.h
	$(CC) $(CFLAGS) -c lista.c

clean:
	rm -f $(TARGET) $(OBJS) *.vc
