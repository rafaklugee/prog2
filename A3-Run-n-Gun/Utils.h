#ifndef __UTILS__
#define __UTILS__

#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Player1.h"
#include "Enemy.h"

void reset_game_state(player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y);
void draw_hud(player1 *p, ALLEGRO_FONT *font);
void enemy_check_slime_collisions_with_player(
    enemy *enemies,
    player1 *p,
    int current_camera_x,
    int player_hitbox_x,
    int player_hitbox_y,
    int player_hitbox_w,
    int player_hitbox_h,
    int player_screen_y
);

#endif