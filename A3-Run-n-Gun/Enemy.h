#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <allegro5/allegro5.h>
#include "Pistol.h"

// Estado do inimigo (vivo ou morto)
typedef enum { ENEMY_ALIVE, ENEMY_DEAD } enemy_state;

typedef struct enemy {
    float x, y;
    float speed;
    int health;
    int facing;
    
    // Andar
    ALLEGRO_BITMAP *walk_sprite;
    int walk_frame, walk_max_frames, walk_frame_width, walk_frame_height,
        walk_frame_counter, walk_frame_delay;
    // Morrer
    ALLEGRO_BITMAP *dead_sprite;
    int dead_frame, dead_max_frames, dead_frame_width, dead_frame_height,
        dead_frame_counter, dead_frame_delay, dead_cooldown;
    // Atacar
    ALLEGRO_BITMAP *attack_sprite;
    int attack_frame, attack_max_frames, attack_frame_width, attack_frame_height,
        attack_frame_counter, attack_frame_delay, is_attacking, attack_cooldown;
    int slime_shot_this_attack;

    // Variáveis de controle geral
    int attack_cooldown_base;
    struct enemy *next;
    enemy_state state;
} enemy;

enemy* enemy_create(float x, float y, float speed);
void enemy_update(enemy *e);
void enemy_draw(enemy *e, int camera_x);
void enemy_draw_all(enemy *head, int camera_x, bool show_hitbox);
void enemy_get_collision_box(enemy *e, int *cx, int *cy, int *cw, int *ch);
void enemy_draw_hitbox(enemy *e, int camera_x, bool show);
void enemy_update_all(enemy *head);
void enemy_check_bullet_collisions(enemy *head, pistol *gun);
void enemy_remove_dead(enemy **head);
void enemy_destroy(enemy *e);
void enemy_destroy_all(enemy *head);
void enemy_apply_difficulty(enemy *head, int difficulty);

#endif