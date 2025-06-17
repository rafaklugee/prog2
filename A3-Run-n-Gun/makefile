TARGET = game
SOURCES = MeuJogo.c Joystick.c Bullet.c Pistol.c Player1.c Background.c Menu.c
ALLEGRO_FLAGS = $(shell pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_primitives-5 allegro_image-5 allegro_ttf-5 --libs --cflags)
CFLAGS = -Wall

all:
	gcc $(CFLAGS) $(SOURCES) -o $(TARGET) $(ALLEGRO_FLAGS) -lm

clean:
	rm -f $(TARGET) *.o
