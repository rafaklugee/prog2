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
#include "Menu.h"

#define X_SCREEN 800
#define Y_SCREEN 600

extern boss *final_boss;
extern slime_ball *slime_balls;

// Função para reiniciar o jogo (jogar novamente)
void reset_game_state(
    player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y,
    Background *bg, int bg_repeat
) {
    // Destrói tudo criado e cria tudo novamente
    player1_destroy(*p);
    *p = create_player1(player_screen_y, X_SCREEN, Y_SCREEN);
    player1_load_sprites(*p);
    // Destrói todos os inimigos
    enemy_destroy_all(*enemies);
    // Cria novos inimigos
    int enemy_positions[6] = {600, 1100, 1700, 2300, 2900, 3500};
    float enemy_speeds[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    *enemies = enemy_create(enemy_positions[0], Y_SCREEN - 325, enemy_speeds[0]);
    enemy *curr = *enemies;
    for (int i = 1; i < 6; i++) {
        curr->next = enemy_create(enemy_positions[i], Y_SCREEN - 325, enemy_speeds[i]);
        curr = curr->next;
    }
    curr->next = NULL;

    *player_world_x = 50;
    *current_camera_x = 0;
    (*p)->is_dead = 0;
    (*p)->health = 3;

    // Limpa todas as slime balls
    slime_ball_destroy_all(&slime_balls);

    // Destrói o boss
    if (final_boss) {
        boss_destroy(final_boss);
    }
    // Cria o boss
    final_boss = boss_create(
        bg_repeat * bg->scaled_w_near - 300,
        Y_SCREEN - 325,
        3.5f
    );
    final_boss->is_active = 0;
}

// Desenha a HUD do player
void draw_hud(player1 *p, ALLEGRO_FONT *font) {
    // Desenha 3 chapéus
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

    // Desenha a HUD da munição com escala reduzida
    int ammo_x = 30;
    int ammo_y = 55;
    int ammo_w = al_get_bitmap_width(p->hud_ammo);
    int ammo_h = al_get_bitmap_height(p->hud_ammo);
    float scale = 0.13f;

    if (!p->is_dead) {
        // Desenha o fundo da munição
        al_draw_scaled_bitmap(
            p->hud_ammo,
            0, 0, ammo_w, ammo_h,
            ammo_x - 11, ammo_y - 4,
            ammo_w * scale, ammo_h * scale,
            0
        );

        // Desenha o texto da munição
        al_draw_textf(
            font,
            al_map_rgb(204 ,171 , 58),
            ammo_x + (ammo_w * scale) / 2, ammo_y + (ammo_h * scale) / 2 - 10,
            ALLEGRO_ALIGN_CENTRE,
            "%02d / %02d", p->ammo, p->max_ammo
        );

        // Se o player está recarregando, desenha a barrinha de progresso
        if (p->is_reloading) {
            // Tamanho e posição da barrinha
            float bar_w = ammo_w * scale;
            float bar_h = 8;
            float bar_x = ammo_x - 12;
            float bar_y = ammo_y + ammo_h * scale + 2;

            // Progresso do reload
            float progress = (float)p->reload_frame / (float)p->reload_max_frames;
            if (progress > 1.0f) 
                progress = 1.0f;

            // Fundo da barrinha
            al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, al_map_rgb(60, 60, 60));
            // Progresso
            al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_w * progress, bar_y + bar_h, al_map_rgb(42, 19, 61));
            // Borda
            al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, al_map_rgb(255,255,255), 1);
        }
    }
}

// Função auxiliar para melhor cálculo de colisão (círculo-retângulo)
static int circle_rect_collision(float cx, float cy, float r, int rx, int ry, int rw, int rh) {
    float closest_x = fmaxf(rx, fminf(cx, rx + rw));
    float closest_y = fmaxf(ry, fminf(cy, ry + rh));
    float dx = cx - closest_x;
    float dy = cy - closest_y;
    return (dx * dx + dy * dy) < (r * r);
}

// Função que verifica colisão da slime ball com o player (inimigos e boss)
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
    // Enquanto forem lançadas slime balls
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
        // Tamanhos e hitboxes diferentes para cada tipo de slime
        if (b->type == SLIME_RED) {
            slime_hitbox_y += 80;
            r *= 0.75f;
        }
        if (b->type == SLIME_GREEN) {
            slime_hitbox_y += 10;
            r *= 0.6f;
        }
        // Se atinigiu o player, diminui sua vida e some
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

int handle_player_death_menu(
    player1 **p, enemy **enemies, boss *final_boss,
    int *player_world_x, int *current_camera_x, int player_screen_y,
    ALLEGRO_DISPLAY *disp, ALLEGRO_FONT *font, ALLEGRO_FONT *big_font,
    ALLEGRO_EVENT_QUEUE *queue, Background *bg, int bg_repeat
) {
    int enemy_speed_multiplier = 1.0f;
    int enemy_attack_cooldown = 90;

    int menu_choice = show_gameover_menu(disp, font, big_font, queue, bg);
    if (menu_choice == 1) {
        return 1; // Sair do jogo
    } else {
        int new_choice = menu_choose_difficulty(disp, font, big_font, queue, bg);
        if (new_choice < 0) {
            return 1; // Sair do jogo
        } else {
            // Ajuste as variáveis de dificuldade conforme a escolha
            enemy_speed_multiplier = 1.0f;
            enemy_attack_cooldown = 90;
            if (new_choice == 1) {
                enemy_speed_multiplier = 1.3f;
                enemy_attack_cooldown = 60;
            } else if (new_choice == 2) {
                enemy_speed_multiplier = 1.7f;
                enemy_attack_cooldown = 40;
            }
            // Atualize os inimigos e boss com os novos valores
            reset_game_state(p, enemies, player_world_x, current_camera_x, player_screen_y, bg, bg_repeat);
            for (enemy *e = *enemies; e; e = e->next) {
                e->speed = 1.0f * enemy_speed_multiplier;
                e->attack_cooldown = 0;
                e->attack_cooldown_base = enemy_attack_cooldown;
            }
            if (final_boss) {
                if (new_choice == 0) final_boss->attack_cooldown_base = 90;
                else if (new_choice == 1) final_boss->attack_cooldown_base = 60;
                else if (new_choice == 2) final_boss->attack_cooldown_base = 40;
                final_boss->attack_cooldown = final_boss->attack_cooldown_base;
            }
        }
    }
    return 0; // Continua o jogo
}