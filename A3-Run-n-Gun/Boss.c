#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <time.h>
#include "Boss.h"
#include "SlimeBall.h"
#include "Enemy.h"
#include "Player1.h"
#include "Menu.h"
#include "Background.h"

extern slime_ball *slime_balls;
extern enemy *enemies;
extern int player_world_x;

// Inicialização do boss com os atributos iniciais
boss* boss_create(int x, int y, float scale) {
    boss *b = malloc(sizeof(boss));
    if (!b) 
        return NULL;
    b->x = x;
    b->y = y;
    b->scale = scale;
    // Sprites
    b->idle = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_idle.png");
    b->hurt = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_hurt.png");
    b->attack1 = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_attack2.png");
    b->attack2 = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_attack1.png");
    b->death = al_load_bitmap("sprites/boss/3_Big_Bloated/Big_bloated_death.png");
    // Frames e dimensões
    b->max_frames = 4;
    b->frame = 0;
    b->frame_width = al_get_bitmap_width(b->idle) / b->max_frames;
    b->frame_height = al_get_bitmap_height(b->idle);
    b->frame_delay = 10;
    b->frame_counter = 0;
    b->is_active = 0;
    b->health = 3;
    b->is_hurt = 0;
    // Dano
    b->hurt_max_frames = 2;
    b->hurt_frame = 0;
    b->hurt_frame_width = al_get_bitmap_width(b->hurt) / b->hurt_max_frames;
    b->hurt_frame_height = al_get_bitmap_height(b->hurt);
    b->hurt_frame_delay = 7;
    b->hurt_frame_counter = 0;
    b->hurt_timer = 0;
    // Hitbox
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
    b->attack_cooldown_base = 90;
    srand(time(NULL));
    b->attack_cooldown = 90 + rand() % 61; // 3-5 segundos
    b->zombie_spawn_cooldown = 180 + rand() % 120; // 6-10 segundos
    // Morte
    b->is_dead = 0;
    b->death_frame = 0;
    b->death_max_frames = 4;
    b->death_frame_delay = 10;
    b->death_frame_counter = 0;
    b->death_timer = 1;
    return b;
}

// 
void boss_draw(boss *b, int camera_x, bool show_hitboxes) {
    if (!b || (!b->is_active && !b->is_dead)) 
        return;
    int draw_x = b->x - camera_x;
    int draw_y = b->y;

    // Morte
    if (b->is_dead) {
        int frame_w = al_get_bitmap_width(b->death) / b->death_max_frames;
        int frame_h = al_get_bitmap_height(b->death);
        int frame_x = b->death_frame * frame_w;
        al_draw_scaled_bitmap(
            b->death,
            frame_x, 0, frame_w, frame_h,
            draw_x, draw_y,
            frame_w * b->scale, frame_h * b->scale,
            0
        );
    }
    // Ataque
    else if (b->is_attacking) {
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
    }
    // Dano (hurt)
    else if (b->is_hurt && b->hurt_timer > 0) {
        int frame_x = b->hurt_frame * b->hurt_frame_width;
        al_draw_scaled_bitmap(
            b->hurt,
            frame_x, 0, b->hurt_frame_width, b->hurt_frame_height,
            draw_x, draw_y,
            b->hurt_frame_width * b->scale, b->hurt_frame_height * b->scale,
            0
        );
    }
    // Idle (parado)
    else {
        int frame_x = b->frame * b->frame_width;
        al_draw_scaled_bitmap(
            b->idle,
            frame_x, 0, b->frame_width, b->frame_height,
            draw_x, draw_y,
            b->frame_width * b->scale, b->frame_height * b->scale,
            0
        );
    }

    // Desenha a hitbox do boss
    if (show_hitboxes) {
        al_draw_rectangle(
            draw_x + b->hitbox_offset_x, draw_y + b->hitbox_offset_y,
            draw_x + b->hitbox_offset_x + b->hitbox_w,
            draw_y + b->hitbox_offset_y + b->hitbox_h,
            al_map_rgb(255, 0, 0), 2
        );
    }
}

// Destrói o boss (único)
void boss_destroy(boss *b) {
    if (b) {
        if (b->idle) al_destroy_bitmap(b->idle);
        if (b->hurt) al_destroy_bitmap(b->hurt);
        if (b->attack1) al_destroy_bitmap(b->attack1);
        if (b->attack2) al_destroy_bitmap(b->attack2);
        if (b->death) al_destroy_bitmap(b->death);
        free(b);
    }
}

// Lógicas de update do boss
void boss_update(boss *b) {
    if (!b || (!b->is_active && !b->is_dead)) 
        return;

    // Se o boss morreu, executa a animação de morte
    if (b->is_dead) {
        // Avança a animação de morte
        if (b->death_frame < b->death_max_frames - 1) {
            b->death_frame_counter++;
            if (b->death_frame_counter >= b->death_frame_delay) {
                b->death_frame++;
                b->death_frame_counter = 0;
            }
        } else {
            // Após a animação, decrementa death_timer a cada frame
            if (b->death_timer > 0) b->death_timer--;
        }
        return;
    }

    // Se o boss está atacando, atualiza os frames de ataque
    if (b->is_attacking) {
        b->attack_frame_counter++;
        if (b->attack_frame_counter >= b->attack_frame_delay) {
            b->attack_frame = (b->attack_frame + 1);
            b->attack_frame_counter = 0;
            if (b->attack_frame >= b->attack_max_frames) {
                // Ataque slime ball azul: 3 slimes em leque (cima, meio, baixo)
                if (b->attack_type == 1) {
                    float base_x = b->x + b->frame_width * b->scale / 2;
                    float base_y = b->y + b->frame_height * b->scale / 2;
                    for (int i = -1; i <= 1; i++) {
                        float vx = -7 + i * 2;
                        float vy;
                        if (i == 0)
                            vy = 0;    // Do meio: horizontal
                        else if (i < 0)
                            vy = -4;   // De cima: para cima
                        else
                            vy = 4;    // De baixo: para baixo
                        slime_ball *sb = slime_ball_create(
                            base_x, base_y,
                            vx, vy,
                            SLIME_BLUE
                        );
                        sb->next = slime_balls;
                        slime_balls = sb;
                    }
                // Ataque slime ball vermelha: 1 slime grande e rápida
                } else if (b->attack_type == 2) {        
                    float base_x = b->x + b->frame_width * b->scale / 2;
                    float base_y = b->y + b->frame_height * b->scale / 2;
                    slime_ball *sb = slime_ball_create(
                        base_x, base_y,
                        -10, 1, // Trajetória da slime vermelha
                        SLIME_RED
                    );
                    sb->next = slime_balls;
                    slime_balls = sb;
                }

                // No fim do ataque, reseta os frames de ataque
                b->is_attacking = 0;
                b->attack_frame = 0;
                b->attack_cooldown = b->attack_cooldown_base + rand() % 61;
            }
        }
    // Se o cooldown de ataque está ativo, decrementa
    } else {
        if (b->attack_cooldown > 0) {
            b->attack_cooldown--;
        // Ataca, mas só ataca se o player estiver perto (600 pixels)
        } else {
            if (abs((int)(b->x - player_world_x)) < 600) {
                b->attack_type = (rand() % 2) + 1;
                b->is_attacking = 1;
                b->attack_frame = 0;
                b->attack_frame_counter = 0;
            }
            // Se o player não está perto, espera mais um pouco antes de atacar
            else {
                b->attack_cooldown = 15;
            }
        }
    }

    // Avanço de frame para dano (hurt)
    if (b->is_hurt && b->hurt_timer > 0) {
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
    }

    // Avanço de frame para idle (parado)
    if (!b->is_attacking && !b->is_hurt && !b->is_dead) {
        b->frame_counter++;
        if (b->frame_counter >= b->frame_delay) {
            b->frame = (b->frame + 1) % b->max_frames;
            b->frame_counter = 0;
        }
    }

    // Invocação de zumbis pelo boss
    // Se estão em cooldown, decrementa
    if (b->zombie_spawn_cooldown > 0) {
        b->zombie_spawn_cooldown--;
    // Cria um novo zumbi perto do boss em um intervalo de 6-10 segundos
    } else {
        float spawn_x = b->x - 150;
        float spawn_y = b->y;
        enemy *new_zombie = enemy_create(spawn_x, spawn_y, 1.0);
        new_zombie->next = enemies;
        enemies = new_zombie;
        b->zombie_spawn_cooldown = 180 + rand() % 120;
    }
}

// Checa colisão de balas no boss
void boss_check_bullet_collision(boss *b, pistol *g) {
    if (!b || !b->is_active || b->health <= 0 || !g) 
        return;
    bullet **curr = &(g->shots);
    int bx = b->x + b->hitbox_offset_x;
    int by = b->y + b->hitbox_offset_y;
    int bw = b->hitbox_w;
    int bh = b->hitbox_h;
    while (*curr) {
        bullet *bul = *curr;
        // Se a bala acertou o boss, diminui sua vida
        if (bul->x > bx && bul->x < bx + bw &&
            bul->y > by && bul->y < by + bh) {
            if (!b->is_hurt) {
                b->health--;
                b->is_hurt = 1;
                b->hurt_timer = 10;
                if (b->health <= 0 && !b->is_dead) {
                    b->is_dead = 1;
                    b->death_frame = 0;
                    b->death_frame_counter = 0;
                    b->death_timer = 60;
                    b->is_hurt = 0;
                    b->hurt_timer = 0;
                }
            }
            *curr = (bullet*)bul->next;
            bullet_destroy(bul);
            break;
        } else {
            curr = (bullet**)&((*curr)->next);
        }
    }
}

// Evento de morte do boss
int boss_handle_death_end(
    boss *b, ALLEGRO_DISPLAY *disp, ALLEGRO_FONT *font, ALLEGRO_FONT *big_font,
    ALLEGRO_EVENT_QUEUE *queue, Background *bg,
    player1 **p, enemy **enemies, int *player_world_x, int *current_camera_x, int player_screen_y, int bg_repeat
) {
    static int boss_death_end_timer = 0;
    // Se o boss está morto e a animação de morte terminou, mostra o menu de vitória
    if (b && b->is_dead && b->death_frame >= b->death_max_frames - 1 && b->death_timer <= 0) {
        boss_death_end_timer++;
        if (boss_death_end_timer > 5) {
            int menu_choice = show_victory_menu(disp, font, big_font, queue, bg);
            if (menu_choice == 1) {
                // Sair do jogo
                return 1;
            } else {
                int menu_result = show_main_menu(disp, font, big_font, queue, bg);
                if (menu_result == 1) {
                     // Sair do jogo
                    return 1;
                } else {
                    boss_death_end_timer = 0;
                    // Jogar novamente
                    reset_game_state(p, enemies, player_world_x, current_camera_x, player_screen_y, bg, bg_repeat);
                }
            }
        }
    // Se o boss não está morto, reseta o timer de morte
    } else if (!(b && b->is_dead)) {
        boss_death_end_timer = 0;
    }
    return 0;
}