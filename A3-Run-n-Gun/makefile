TARGET = game
SOURCES = MeuJogo.c Joystick.c Bullet.c Pistol.c
ALLEGRO_FLAGS = $(shell pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_primitives-5 allegro_image-5 --libs --cflags)
CFLAGS = -Wall

all:
	gcc $(CFLAGS) $(SOURCES) -o $(TARGET) $(ALLEGRO_FLAGS)

clean:
	rm -f $(TARGET) *.o
