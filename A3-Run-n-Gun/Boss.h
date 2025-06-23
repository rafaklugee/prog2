#ifndef __BOSS_H__
#define __BOSS_H__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include "Player1.h"
#include "Pistol.h"
#include "Background.h"
#include "Enemy.h"
#include "Utils.h"

typedef struct boss {
    // Sprites
    ALLEGRO_BITMAP *idle;
    ALLEGRO_BITMAP *hurt;
    ALLEGRO_BITMAP *attack1; 
    ALLEGRO_BITMAP *attack2;
    ALLEGRO_BITMAP *attack3; 
    ALLEGRO_BITMAP *death;

    // Variáveis de controle geral
    int x, y;
    float scale;
    int frame, max_frames, frame_width, frame_height, frame_delay, frame_counter;
    int is_active;
    int health;
    int zombie_spawn_cooldown;
    int hitbox_offset_x, hitbox_offset_y, hitbox_w, hitbox_h;

    // Dano (hurt)
    int hurt_frame, hurt_max_frames, hurt_frame_width, hurt_frame_height, 
        hurt_frame_delay, hurt_frame_counter, is_hurt, hurt_timer;
    
    // Ataque
    int is_attacking, attack_type, attack_frame, attack_max_frames, attack_cooldown_base, 
        attack_frame_delay, attack_frame_counter, attack_cooldown; 

    // Morte
    int is_dead, death_frame, death_max_frames, death_frame_delay, death_frame_counter, death_timer;              
} boss;

boss* boss_create(int x, int y, float scale);
void boss_draw(boss *b, int camera_x, bool show_hitboxes);
void boss_destroy(boss *b);
void boss_update(boss *b);
void boss_check_bullet_collision(boss *b, pistol *g);
void boss_apply_difficulty(boss *b, int difficulty);
int boss_handle_death_end(
    boss *b, ALLEGRO_DISPLAY *disp, ALLEGRO_FONT *font, ALLEGRO_FONT *big_font,
    ALLEGRO_EVENT_QUEUE *queue, Background *bg,
    player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y, int bg_repeat
);

#endif