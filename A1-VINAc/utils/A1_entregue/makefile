CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c99
TARGET = vina
OBJS = main.o lz.o lista.o vinac.o
OUT_DIR = rrk24
OUT_TARGET = $(OUT_DIR)/$(TARGET)

all: $(OUT_DIR) $(OUT_TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUT_TARGET) $(OBJS)

main.o: main.c vinac.h lista.h
	$(CC) $(CFLAGS) -c main.c

vinac.o: vinac.c vinac.h lz.h lista.h
	$(CC) $(CFLAGS) -c vinac.c

lz.o: lz.c lz.h
	$(CC) $(CFLAGS) -c lz.c

lista.o: lista.c lista.h
	$(CC) $(CFLAGS) -c lista.c

clean:
	rm -f $(OUT_TARGET) $(OBJS)

	
