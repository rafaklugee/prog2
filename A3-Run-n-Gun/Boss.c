#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h> // para al_draw_rectangle
#include "Boss.h"
#include "SlimeBall.h"
#include "Enemy.h"
#include <time.h> // para srand/rand
#include "Player1.h"

extern slime_ball *slime_balls;
extern enemy *enemies;
extern int player_world_x;

boss* boss_create(int x, int y, float scale, const char *idle_sprite) {
    boss *b = malloc(sizeof(boss));
    if (!b) return NULL;
    b->x = x;
    b->y = y;
    b->scale = scale;
    b->idle = al_load_bitmap(idle_sprite);
    b->hurt = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_hurt.png");
    b->attack1 = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_attack2.png");
    b->attack2 = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_attack1.png");
    b->max_frames = 4;
    b->frame = 0;
    b->frame_width = al_get_bitmap_width(b->idle) / b->max_frames;
    b->frame_height = al_get_bitmap_height(b->idle);
    b->frame_delay = 10;
    b->frame_counter = 0;
    b->is_active = 0;
    b->health = 100;
    b->is_hurt = 0;
    b->hurt_max_frames = 2;
    b->hurt_frame = 0;
    b->hurt_frame_width = al_get_bitmap_width(b->hurt) / b->hurt_max_frames;
    b->hurt_frame_height = al_get_bitmap_height(b->hurt);
    b->hurt_frame_delay = 7;
    b->hurt_frame_counter = 0;
    b->hurt_timer = 0;
    b->hitbox_offset_x = 80;
    b->hitbox_offset_y = 80;
    b->hitbox_w = b->frame_width * b->scale - 100;
    b->hitbox_h = b->frame_height * b->scale - 80;
    // Ataque
    b->is_attacking = 0;
    b->attack_type = 0;
    b->attack_frame = 0;
    b->attack_max_frames = 6;
    b->attack_frame_delay = 7;
    b->attack_frame_counter = 0;
    srand(time(NULL));
    b->attack_cooldown = 90 + rand() % 61; // 3-5 segundos (30fps)
    b->zombie_spawn_cooldown = 180 + rand() % 120; // 6~10 segundos (30fps)
    return b;
}

void boss_draw(boss *b, int camera_x, bool show_hitboxes) {
    if (!b || !b->is_active) return;
    int draw_x = b->x - camera_x;
    int draw_y = b->y;

    if (b->is_attacking) {
        ALLEGRO_BITMAP *atk = (b->attack_type == 1) ? b->attack1 : b->attack2;
        int frame_w = al_get_bitmap_width(atk) / b->attack_max_frames;
        int frame_h = al_get_bitmap_height(atk);
        int frame_x = b->attack_frame * frame_w;
        al_draw_scaled_bitmap(
            atk,
            frame_x, 0, frame_w, frame_h,
            draw_x, draw_y,
            frame_w * b->scale, frame_h * b->scale,
            0
        );
    } else if (b->is_hurt && b->hurt_timer > 0) {
        int frame_x = b->hurt_frame * b->hurt_frame_width;
        al_draw_scaled_bitmap(
            b->hurt,
            frame_x, 0, b->hurt_frame_width, b->hurt_frame_height,
            draw_x, draw_y,
            b->hurt_frame_width * b->scale, b->hurt_frame_height * b->scale,
            0
        );
        b->hurt_frame_counter++;
        if (b->hurt_frame_counter >= b->hurt_frame_delay) {
            b->hurt_frame = (b->hurt_frame + 1) % b->hurt_max_frames;
            b->hurt_frame_counter = 0;
        }
        b->hurt_timer--;
        if (b->hurt_timer <= 0) {
            b->is_hurt = 0;
            b->hurt_frame = 0;
        }
    } else {
        int frame_x = b->frame * b->frame_width;
        al_draw_scaled_bitmap(
            b->idle,
            frame_x, 0, b->frame_width, b->frame_height,
            draw_x, draw_y,
            b->frame_width * b->scale, b->frame_height * b->scale,
            0
        );
        b->frame_counter++;
        if (b->frame_counter >= b->frame_delay) {
            b->frame = (b->frame + 1) % b->max_frames;
            b->frame_counter = 0;
        }
    }

    // Desenhar a hitbox se show_hitboxes for true
    if (show_hitboxes) {
        al_draw_rectangle(
            draw_x + b->hitbox_offset_x, draw_y + b->hitbox_offset_y,
            draw_x + b->hitbox_offset_x + b->hitbox_w,
            draw_y + b->hitbox_offset_y + b->hitbox_h,
            al_map_rgb(255, 0, 0), 2
        );
    }
}

void boss_destroy(boss *b) {
    if (b) {
        if (b->idle) al_destroy_bitmap(b->idle);
        if (b->hurt) al_destroy_bitmap(b->hurt);
        if (b->attack1) al_destroy_bitmap(b->attack1); // NOVO
        if (b->attack2) al_destroy_bitmap(b->attack2); // NOVO
        free(b);
    }
}

void boss_update(boss *b) {
    if (!b || !b->is_active) return;

    if (b->is_attacking) {
        b->attack_frame_counter++;
        if (b->attack_frame_counter >= b->attack_frame_delay) {
            b->attack_frame = (b->attack_frame + 1);
            b->attack_frame_counter = 0;
            if (b->attack_frame >= b->attack_max_frames) {
                // --- ATAQUE: JOGAR SLIMES ---
                if (b->attack_type == 1) {
                    // Ataque azul: 3 slimes em leque (cima, meio, baixo)
                    float base_x = b->x + b->frame_width * b->scale / 2;
                    float base_y = b->y + b->frame_height * b->scale / 2;
                    for (int i = -1; i <= 1; i++) {
                        float vx = -7 + i * 2; // espalhamento horizontal
                        float vy;
                        if (i == 0)
                            vy = 0;    // do meio: horizontal
                        else if (i < 0)
                            vy = -4;   // de cima: para cima
                        else
                            vy = 4;    // de baixo: para baixo
                        slime_ball *sb = slime_ball_create(
                            base_x, base_y,
                            vx, vy,
                            SLIME_BLUE
                        );
                        sb->next = slime_balls;
                        slime_balls = sb;
                    }
                } else if (b->attack_type == 2) {
                    // Ataque vermelho: 1 slime grande e rápida
                    float base_x = b->x + b->frame_width * b->scale / 2;
                    float base_y = b->y + b->frame_height * b->scale / 2;
                    slime_ball *sb = slime_ball_create(
                        base_x, base_y,
                        -10, 1, // Tajetória da slime vermelha
                        SLIME_RED
                    );
                    sb->next = slime_balls;
                    slime_balls = sb;
                }
                // --- FIM DO ATAQUE ---

                b->is_attacking = 0;
                b->attack_frame = 0;
                b->attack_cooldown = 90 + rand() % 61;
            }
        }
    } else {
        if (b->attack_cooldown > 0) {
            b->attack_cooldown--;
        } else {
            // Só ataca se o player estiver perto (exemplo: 600 pixels)
            if (abs((int)(b->x - player_world_x)) < 600) {
                b->attack_type = (rand() % 2) + 1;
                b->is_attacking = 1;
                b->attack_frame = 0;
                b->attack_frame_counter = 0;
            }
            // Se o player não está perto, espera mais um pouco
            else {
                b->attack_cooldown = 15; // espera meio segundo antes de checar de novo
            }
        }
    }

    // SPAWN DE ZUMBI
    if (b->zombie_spawn_cooldown > 0) {
        b->zombie_spawn_cooldown--;
    } else {
        // Cria um novo zumbi perto do boss
        float spawn_x = b->x - 150;  // à esquerda do boss
        float spawn_y = b->y;
        enemy *new_zombie = enemy_create(spawn_x, spawn_y, 1.0, "sprites/zombies/Zombie_3/Walk.png");
        new_zombie->next = enemies;
        enemies = new_zombie;
        // Reinicia o cooldown
        b->zombie_spawn_cooldown = 180 + rand() % 120; // 6~10 segundos
    }
}