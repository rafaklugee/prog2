#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <allegro5/allegro5.h>
#include "SlimeBall.h"
#include "Pistol.h"

typedef enum { ENEMY_ALIVE, ENEMY_DEAD } enemy_state; // Novo enum para estado

typedef struct enemy {
    float x, y;
    float speed;
    int walk_frame, walk_max_frames, walk_frame_width, walk_frame_height;
    int walk_frame_counter, walk_frame_delay;
    int health;
    int dead_cooldown;
    int facing;
    enemy_state state;
    int dead_frame, dead_max_frames, dead_frame_width, dead_frame_height;
    int dead_frame_counter, dead_frame_delay;
    ALLEGRO_BITMAP *walk_sprite;
    ALLEGRO_BITMAP *dead_sprite;
    // Ataque
    ALLEGRO_BITMAP *attack_sprite;
    int attack_frame, attack_max_frames, attack_frame_width, attack_frame_height;
    int attack_frame_counter, attack_frame_delay;
    int is_attacking, attack_cooldown;
    struct enemy *next;
    // Slimes
    slime_ball *slimes;
    int slime_shot_this_attack;
} enemy;

enemy* enemy_create(float x, float y, float speed, const char *walk_sprite_path);
void enemy_update(enemy *e);
void enemy_draw(enemy *e, int camera_x);
void enemy_destroy(enemy *e);
void enemy_get_collision_box(enemy *e, int *cx, int *cy, int *cw, int *ch);
void enemy_draw_hitbox(enemy *e, int camera_x, bool show);
void enemy_update_all(enemy *head);
void enemy_draw_all(enemy *head, int camera_x, bool show_hitbox);
void enemy_destroy_all(enemy *head);
void enemy_update_slimes(enemy *head, int world_width);
void enemy_check_bullet_collisions(enemy *head, pistol *gun);
void enemy_remove_dead(enemy **head);

#endif