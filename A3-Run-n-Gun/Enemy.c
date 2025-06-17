#include <stdlib.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "Enemy.h"
#include "SlimeBall.h"
#include "Pistol.h"
#include "Player1.h"

enemy* enemy_create(float x, float y, float speed, const char *walk_sprite_path) {
    enemy *e = (enemy*)malloc(sizeof(enemy));
    if (!e) return NULL;
    e->x = x;
    e->y = y;
    e->speed = speed;
    e->walk_sprite = al_load_bitmap(walk_sprite_path);
    e->dead_sprite = al_load_bitmap("sprites/zombies/Zombie_3/Dead.png"); // Caminho fixo para sprite de morte
    e->attack_sprite = al_load_bitmap("sprites/zombies/Zombie_3/Attack.png");
    if (!e->walk_sprite || !e->dead_sprite || !e->attack_sprite) {
        if (e->walk_sprite) al_destroy_bitmap(e->walk_sprite);
        if (e->dead_sprite) al_destroy_bitmap(e->dead_sprite);
        if (e->attack_sprite) al_destroy_bitmap(e->attack_sprite);
        free(e);
        return NULL;
    }
    e->walk_max_frames = 10;
    e->walk_frame = 0;
    e->walk_frame_width = al_get_bitmap_width(e->walk_sprite) / e->walk_max_frames;
    e->walk_frame_height = al_get_bitmap_height(e->walk_sprite);
    e->walk_frame_counter = 0;
    e->walk_frame_delay = 5;

    // Inicialização da animação de morte
    e->dead_max_frames = 5; // Ajuste conforme seu sprite
    e->dead_frame = 0;
    e->dead_frame_width = al_get_bitmap_width(e->dead_sprite) / e->dead_max_frames;
    e->dead_frame_height = al_get_bitmap_height(e->dead_sprite);
    e->dead_frame_counter = 0;
    e->dead_frame_delay = 7;

    // Inicialização da animação de ataque
    e->attack_max_frames = 4; // ajuste conforme seu sprite
    e->attack_frame = 0;
    e->attack_frame_width = al_get_bitmap_width(e->attack_sprite) / e->attack_max_frames;
    e->attack_frame_height = al_get_bitmap_height(e->attack_sprite);
    e->attack_frame_counter = 0;
    e->attack_frame_delay = 5;
    e->is_attacking = 0;
    e->attack_cooldown = 0;

    e->health = 3; // 3 tiros para morrer
    e->state = ENEMY_ALIVE;
    e->next = NULL;
    e->slimes = NULL;
    e->facing = -1; // Começa olhando para esquerda
    e->slime_shot_this_attack = 0;
    return e;
}

void enemy_update(enemy *e) {
    if (!e) return;

    extern int player_world_x;
    // Atualiza direção do inimigo para sempre olhar para o player
    if (e->x < player_world_x)
        e->facing = 1; // olha para a direita
    else
        e->facing = -1; // olha para a esquerda

    if (e->state == ENEMY_ALIVE) {
        // Ataque se perto do player (exemplo: 400 pixels)
        if (abs((int)(e->x - player_world_x)) < 400 && e->attack_cooldown == 0) {
            e->is_attacking = 1;
            e->attack_frame = 0;
            e->attack_frame_counter = 0;
            e->attack_cooldown = 90; // 3 segundos de recarga
        }
        if (e->is_attacking) {
            e->attack_frame_counter++;
            if (e->attack_frame_counter >= e->attack_frame_delay) {
                e->attack_frame++;
                e->attack_frame_counter = 0;
            }
            // Só lança a slime uma vez por ataque
            if (e->attack_frame == 2 && !e->slime_shot_this_attack) {
                float slime_offset_x;
                float slime_y = e->y + e->attack_frame_height * 1.2;
                float slime_vx;

                if (e->facing == -1) {
                    slime_offset_x = e->x + 70;
                    slime_vx = -7;
                } else {
                    slime_offset_x = e->x + e->attack_frame_width * 2.0 - 110;
                    slime_vx = 7;
                }
                float slime_vy = 0;
                slime_ball *b = slime_ball_create(slime_offset_x, slime_y, slime_vx, slime_vy);
                b->next = e->slimes;
                e->slimes = b;
                e->slime_shot_this_attack = 1; // Marca que já lançou a slime neste ataque
            }
            if (e->attack_frame >= e->attack_max_frames) {
                e->is_attacking = 0;
                e->attack_frame = 0;
                e->slime_shot_this_attack = 0; // Reseta para o próximo ataque
            }
        } else {
            // Só anda se ainda não passou do player
            if ((e->x > player_world_x && e->speed > 0 && e->x - e->speed > player_world_x) ||
                (e->x < player_world_x && e->speed > 0 && e->x + e->speed < player_world_x)) {
                // Anda em direção ao player
                if (e->x > player_world_x)
                    e->x -= e->speed;
                else
                    e->x += e->speed;

                e->walk_frame_counter++;
                if (e->walk_frame_counter >= e->walk_frame_delay) {
                    e->walk_frame = (e->walk_frame + 1) % e->walk_max_frames;
                    e->walk_frame_counter = 0;
                }
            }
            // Senão, fica parado olhando para o player
        }
        if (e->attack_cooldown > 0) e->attack_cooldown--;
    } else if (e->state == ENEMY_DEAD) {
        // Animação de morte
        if (e->dead_frame < e->dead_max_frames - 1) {
            e->dead_frame_counter++;
            if (e->dead_frame_counter >= e->dead_frame_delay) {
                e->dead_frame++;
                e->dead_frame_counter = 0;
            }
        } else {
            // Começa a contar o cooldown após a animação de morte terminar
            if (e->dead_cooldown > 0) {
                e->dead_cooldown--;
            }
        }
    }
}

void enemy_draw(enemy *e, int camera_x) {
    if (!e) return;
    int flip = (e->facing == 1) ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    if (e->state == ENEMY_ALIVE) {
        if (e->is_attacking) {
            // Desenha o sprite de ataque
            int frame_x = e->attack_frame * e->attack_frame_width;
            float attack_x_offset = -20.0; // ajuste conforme necessário
            al_draw_scaled_bitmap(
                e->attack_sprite,
                frame_x, 0, e->attack_frame_width, e->attack_frame_height,
                e->x + attack_x_offset - camera_x, e->y,
                e->attack_frame_width * 2.0,
                e->attack_frame_height * 2.0,
                flip
            );
        } else {
            // Desenha o sprite de caminhada normalmente
            int frame_x = e->walk_frame * e->walk_frame_width;
            float walk_x_offset = -20.0; // ajuste conforme necessário
            al_draw_scaled_bitmap(
                e->walk_sprite,
                frame_x, 0, e->walk_frame_width, e->walk_frame_height,
                e->x + walk_x_offset - camera_x, e->y,
                e->walk_frame_width * 2.0,
                e->walk_frame_height * 2.0,
                flip
            );
        }
        slime_ball_draw(e->slimes, camera_x);
    } else if (e->state == ENEMY_DEAD) {
        int frame_x = e->dead_frame * e->dead_frame_width;
        al_draw_scaled_bitmap(
            e->dead_sprite,
            frame_x, 0, e->dead_frame_width, e->dead_frame_height,
            e->x - camera_x, e->y,
            e->dead_frame_width * 2.0,
            e->dead_frame_height * 2.0,
            flip
        );
    }
}

void enemy_destroy(enemy *e) {
    if (!e) return;
    if (e->walk_sprite) al_destroy_bitmap(e->walk_sprite);
    if (e->dead_sprite) al_destroy_bitmap(e->dead_sprite);
    if (e->attack_sprite) al_destroy_bitmap(e->attack_sprite);
    while (e->slimes) {
        slime_ball *next = e->slimes->next;
        slime_ball_destroy(e->slimes);
        e->slimes = next;
    }
    free(e);
}

void enemy_get_collision_box(enemy *e, int *cx, int *cy, int *cw, int *ch) {
    int enemy_draw_width = e->walk_frame_width * 2.0;
    int enemy_draw_height = e->walk_frame_height * 2.0;
    int enemy_base_y = e->y + enemy_draw_height;
    *cw = (int)(enemy_draw_width * 0.1);   // Largura da hitbox
    *ch = (int)(enemy_draw_height * 0.5);  // Altura da hitbox
    *cx = (int)(e->x + (enemy_draw_width - *cw) / 2);
    *cy = enemy_base_y - *ch;
}

void enemy_draw_hitbox(enemy *e, int camera_x, bool show) {
    if (!show) return;
    int cx, cy, cw, ch;
    enemy_get_collision_box(e, &cx, &cy, &cw, &ch);
    al_draw_rectangle(
        cx - camera_x, cy,
        cx + cw - camera_x, cy + ch,
        al_map_rgb(255, 0, 0), 2
    );
}

void enemy_update_all(enemy *head) {
    for (enemy *e = head; e; e = e->next)
        enemy_update(e);
}

void enemy_draw_all(enemy *head, int camera_x, bool show_hitbox) {
    for (enemy *e = head; e; e = e->next) {
        enemy_draw(e, camera_x);
        enemy_draw_hitbox(e, camera_x, show_hitbox);
    }
}

void enemy_destroy_all(enemy *head) {
    while (head) {
        enemy *next = head->next;
        enemy_destroy(head);
        head = next;
    }
}

void enemy_update_slimes(enemy *head, int world_width) {
    for (enemy *e = head; e; e = e->next) {
        slime_ball_update(&e->slimes, world_width);
    }
}

void enemy_check_bullet_collisions(enemy *head, pistol *gun) {
    for (enemy *e = head; e; e = e->next) {
        if (e->state != ENEMY_ALIVE) continue;
        bullet **curr = &(gun->shots);
        int cx, cy, cw, ch;
        enemy_get_collision_box(e, &cx, &cy, &cw, &ch);
        while (*curr) {
            bullet *b = *curr;
            if (b->x > cx && b->x < cx + cw &&
                b->y > cy && b->y < cy + ch) {
                e->health--;
                if (e->health <= 0) {
                    e->state = ENEMY_DEAD;
                    e->dead_frame = 0;
                    e->dead_frame_counter = 0;
                    e->dead_cooldown = 45;
                }
                *curr = (bullet*)b->next;
                bullet_destroy(b);
            } else {
                curr = (bullet**)&((*curr)->next);
            }
        }
    }
}

void enemy_remove_dead(enemy **head) {
    while (*head) {
        enemy *e = *head;
        if (e->state == ENEMY_DEAD && e->dead_frame >= e->dead_max_frames - 1 && e->dead_cooldown <= 0) {
            *head = e->next;
            enemy_destroy(e);
        } else {
            head = &((*head)->next);
        }
    }
}