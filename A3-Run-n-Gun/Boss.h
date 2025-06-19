#ifndef __BOSS_H__
#define __BOSS_H__

#include <allegro5/allegro5.h>

typedef struct {
    int x, y;
    float scale;
    ALLEGRO_BITMAP *idle;
    ALLEGRO_BITMAP *hurt;
    ALLEGRO_BITMAP *attack1; // NOVO
    ALLEGRO_BITMAP *attack2; // NOVO
    ALLEGRO_BITMAP *death; // NOVO
    int frame, max_frames, frame_width, frame_height, frame_delay, frame_counter;
    int is_active;
    int health;
    int is_hurt;
    int hurt_frame, hurt_max_frames, hurt_frame_width, hurt_frame_height, hurt_frame_delay, hurt_frame_counter;
    int hurt_timer;
    int hitbox_offset_x, hitbox_offset_y, hitbox_w, hitbox_h;
    // Ataque
    int is_attacking;      // NOVO
    int attack_type;       // 1 ou 2
    int attack_frame;      // frame atual do ataque
    int attack_max_frames; // 6
    int attack_frame_delay;
    int attack_frame_counter;
    int attack_cooldown;   // frames até próximo ataque
    int zombie_spawn_cooldown; // cooldown para spawnar zumbi
    // Morte
    int is_dead;           // NOVO
    int death_frame;       // NOVO
    int death_max_frames;  // NOVO
    int death_frame_delay; // NOVO
    int death_frame_counter; // NOVO
    int death_timer;       // NOVO (delay após animação)
} boss;

boss* boss_create(int x, int y, float scale, const char *idle_sprite);
void boss_draw(boss *b, int camera_x, bool show_hitboxes);
void boss_destroy(boss *b);
void boss_update(boss *b);

#endif