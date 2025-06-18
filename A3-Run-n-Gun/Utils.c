#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Enemy.h"
#include "Player1.h"
#include "SlimeBall.h"

#define X_SCREEN 800
#define Y_SCREEN 600

void reset_game_state(player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y) {
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

        if (!b->has_hit_player &&
            slime_screen_x > player_hitbox_x && slime_screen_x < player_hitbox_x + player_hitbox_w &&
            slime_screen_y > player_hitbox_y && slime_screen_y < player_hitbox_y + player_hitbox_h) {
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