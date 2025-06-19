#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "Enemy.h"
#include "Player1.h"
#include "SlimeBall.h"
#include "Background.h"
#include "Boss.h"

#define X_SCREEN 800
#define Y_SCREEN 600

extern boss *final_boss;
extern slime_ball *slime_balls;

void reset_game_state(
    player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y,
    Background *bg, int bg_repeat
) {
    player1_destroy(*p);
    *p = create_player1(player_screen_y, X_SCREEN, Y_SCREEN);
    player1_load_sprites(
        *p,
        "sprites/gangsters/Gangsters_1/Idle.png",
        "sprites/gangsters/Gangsters_1/Idle_Otherside.png",
        "sprites/gangsters/Gangsters_1/Run.png",
        "sprites/gangsters/Gangsters_1/Squat.png",
        "sprites/gangsters/Gangsters_1/Jump.png",
        "sprites/gangsters/Gangsters_1/Shot.png",
        "sprites/gangsters/Gangsters_1/Squat_Shot.png",
        "sprites/gangsters/Gangsters_1/Hurt.png",
        "sprites/gangsters/Gangsters_1/Dead.png",
        "sprites/gangsters/Gangsters_1/Gangster_Health.png"
    );
    enemy_destroy_all(*enemies);
    *enemies = enemy_create(900, Y_SCREEN - 325, 1.0, "sprites/zombies/Zombie_3/Walk.png");
    (*enemies)->next = enemy_create(1300, Y_SCREEN - 325, 0.75, "sprites/zombies/Zombie_3/Walk.png");
    (*enemies)->next->next = enemy_create(1800, Y_SCREEN - 325, 0.5, "sprites/zombies/Zombie_3/Walk.png");
    *player_world_x = 50;
    *current_camera_x = 0;
    (*p)->is_dead = 0;
    (*p)->health = 3;

    // Limpa todas as slime balls
    slime_ball_destroy_all(&slime_balls);

    // Reseta o boss
    if (final_boss) {
        boss_destroy(final_boss);
    }
    final_boss = boss_create(
        bg_repeat * bg->scaled_w_near - 300, // posição final do mapa
        Y_SCREEN - 325,
        3.5f,
        "sprites/boss/3_Big_Bloated/Big_bloated_idle.png"
    );
    final_boss->is_active = 0; // só ativa quando os inimigos acabam
}

void draw_hud(player1 *p, ALLEGRO_FONT *font) {
    // Desenhe até 3 chapéus, espaçados
    int health_size = 70;
    int spacing = 2;
    int start_x = 10;
    int y = 0;
    for (int i = 0; i < p->health && i < 3; i++) {
        al_draw_scaled_bitmap(
            p->hud_health,
            0, 0,
            al_get_bitmap_width(p->hud_health), al_get_bitmap_height(p->hud_health),
            start_x + i * (health_size + spacing), y,
            health_size, health_size,
            0
        );
    }
    // al_draw_textf(font, al_map_rgb(255,0,0), 20, 20, 0, "VIDA: %d", p->health);
}

// Função auxiliar para colisão círculo-retângulo
static int circle_rect_collision(float cx, float cy, float r, int rx, int ry, int rw, int rh) {
    float closest_x = fmaxf(rx, fminf(cx, rx + rw));
    float closest_y = fmaxf(ry, fminf(cy, ry + rh));
    float dx = cx - closest_x;
    float dy = cy - closest_y;
    return (dx * dx + dy * dy) < (r * r);
}

void check_slime_collision_with_player(
    player1 *p,
    int current_camera_x,
    int player_hitbox_x,
    int player_hitbox_y,
    int player_hitbox_w,
    int player_hitbox_h,
    int player_screen_y
) {
    slime_ball **curr = &slime_balls;
    while (*curr) {
        slime_ball *b = *curr;
        int slime_screen_x = b->x - current_camera_x;
        int slime_screen_y = b->y;

        if (p->is_dead) {
            curr = &b->next;
            continue;
        }

        float r = b->anim_frame_width * b->scale / 2.0f;
        int slime_hitbox_y = slime_screen_y - 20;
        if (b->type == SLIME_RED) {
            slime_hitbox_y += 80;
            r *= 0.75f;
        }
        if (b->type == SLIME_GREEN) {
            slime_hitbox_y += 10;
            r *= 0.6f;
        }
        if (!b->has_hit_player &&
            circle_rect_collision(
                slime_screen_x, slime_hitbox_y, r,
                player_hitbox_x, player_hitbox_y, player_hitbox_w, player_hitbox_h
            )) {
            p->health--;
            p->is_hurt = 1;
            p->hurt_frame = 0;
            p->hurt_frame_counter = 0;
            b->has_hit_player = 1;
            *curr = b->next;
            slime_ball_destroy(b);
            if (p->health <= 0 && !p->is_dead) {
                p->is_dead = 1;
                p->dead_frame = 0;
                p->dead_frame_counter = 0;
                p->dead_menu_cooldown = 45;
                p->player_y = player_screen_y;
            }
        } else {
            curr = &b->next;
        }
    }
}