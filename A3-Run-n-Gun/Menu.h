#ifndef MENU_H
#define MENU_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Background.h"

int show_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* queue, Background* bg);
int show_gameover_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_FONT* big_font, ALLEGRO_EVENT_QUEUE* queue, Background* bg);
int show_victory_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_FONT* big_font, ALLEGRO_EVENT_QUEUE* queue, Background* bg);

#endif