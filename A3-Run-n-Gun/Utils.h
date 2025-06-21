#ifndef __UTILS__
#define __UTILS__

#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Player1.h"
#include "Enemy.h"
#include "Background.h"
#include "Menu.h"

typedef struct boss boss;

void reset_game_state(
    player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y,
    Background *bg, int bg_repeat
);
void check_slime_collision_with_player(
    player1 *p,
    int current_camera_x,
    int player_hitbox_x,
    int player_hitbox_y,
    int player_hitbox_w,
    int player_hitbox_h,
    int player_screen_y
);
void draw_hud(player1 *p, ALLEGRO_FONT *font);
int handle_player_death_menu(
    player1 **p, enemy **enemies, boss *final_boss,
    int *player_world_x, int *current_camera_x, int player_screen_y,
    ALLEGRO_DISPLAY *disp, ALLEGRO_FONT *font, ALLEGRO_FONT *big_font,
    ALLEGRO_EVENT_QUEUE *queue, Background *bg, int bg_repeat
);


#endif