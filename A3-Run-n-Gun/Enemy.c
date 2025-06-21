#include <stdlib.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "Enemy.h"
#include "SlimeBall.h"
#include "Pistol.h"
#include "Player1.h"

// Lista encadeada de slime balls
extern slime_ball *slime_balls;

// Inicialização dos inimigos com atributos iniciais
enemy* enemy_create(float x, float y, float speed) {
    enemy *e = (enemy*)malloc(sizeof(enemy));
    if (!e) 
        return NULL;
    e->x = x;
    e->y = y;
    e->speed = speed;
    e->attack_cooldown_base = 90;
    // Não há necessidade de outra função para dar load pois tem apenas 3 sprites
    e->walk_sprite = al_load_bitmap("sprites/zombies/Zombie_3/Walk.png");
    e->dead_sprite = al_load_bitmap("sprites/zombies/Zombie_3/Dead.png");
    e->attack_sprite = al_load_bitmap("sprites/zombies/Zombie_3/Attack.png");
    if (!e->walk_sprite || !e->dead_sprite || !e->attack_sprite) {
        if (e->walk_sprite) al_destroy_bitmap(e->walk_sprite);
        if (e->dead_sprite) al_destroy_bitmap(e->dead_sprite);
        if (e->attack_sprite) al_destroy_bitmap(e->attack_sprite);
        free(e);
        return NULL;
    }

    // Frames e dimensões
    e->walk_max_frames = 10;
    e->walk_frame = 0;
    e->walk_frame_width = al_get_bitmap_width(e->walk_sprite) / e->walk_max_frames;
    e->walk_frame_height = al_get_bitmap_height(e->walk_sprite);
    e->walk_frame_counter = 0;
    e->walk_frame_delay = 5;

    e->dead_max_frames = 5;
    e->dead_frame = 0;
    e->dead_frame_width = al_get_bitmap_width(e->dead_sprite) / e->dead_max_frames;
    e->dead_frame_height = al_get_bitmap_height(e->dead_sprite);
    e->dead_frame_counter = 0;
    e->dead_frame_delay = 7;

    e->attack_max_frames = 4;
    e->attack_frame = 0;
    e->attack_frame_width = al_get_bitmap_width(e->attack_sprite) / e->attack_max_frames;
    e->attack_frame_height = al_get_bitmap_height(e->attack_sprite);
    e->attack_frame_counter = 0;
    e->attack_frame_delay = 5;
    e->is_attacking = 0;
    e->attack_cooldown = 0;
    e->slime_shot_this_attack = 0;

    e->health = 3; // Cada inimigo comum tem 3 vidas
    e->state = ENEMY_ALIVE;
    e->next = NULL;
    e->facing = -1; // Começa olhando para esquerda
    return e;
}

// Lógicas de update do inimigo
void enemy_update(enemy *e) {
    if (!e) 
        return;

    extern int player_world_x;
    if (e->state == ENEMY_ALIVE) {
        // Atualiza direção do inimigo para sempre olhar para o player
        if (e->x < player_world_x)
            e->facing = 1; // Olha para direita
        else
            e->facing = -1; // Olha para esquerda

        // Ataque se estiver perto do player (400 pixels perto)
        if (abs((int)(e->x - player_world_x)) < 400 && e->attack_cooldown == 0) {
            e->is_attacking = 1;
            e->attack_frame = 0;
            e->attack_frame_counter = 0;
            e->attack_cooldown = e->attack_cooldown_base; // Cooldown do ataque (dificuldades)
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
                    slime_vx = -3; // Velocidade da slime ball (esquerda)
                } else {
                    slime_offset_x = e->x + e->attack_frame_width * 2.0 - 110;
                    slime_vx = 3; // Velocidade da slime ball (direita)
                }

                // Depois de calcular a posição e velocidade, cria a slime ball
                float slime_vy = 0;
                slime_ball *b = slime_ball_create(slime_offset_x, slime_y, slime_vx, slime_vy, SLIME_GREEN);
                b->next = slime_balls;
                slime_balls = b;

                // Marca que já lançou a slime neste ataque
                e->slime_shot_this_attack = 1; 
            }
            // Se já atacou, reseta os frames de ataque
            if (e->attack_frame >= e->attack_max_frames) {
                e->is_attacking = 0;
                e->attack_frame = 0;
                e->slime_shot_this_attack = 0;
            }
        } else {
            // Anda sempre em direção ao player
            if ((e->x > player_world_x && e->speed > 0 && e->x - e->speed > player_world_x) ||
                (e->x < player_world_x && e->speed > 0 && e->x + e->speed < player_world_x)) {
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
        }
        if (e->attack_cooldown > 0) 
            e->attack_cooldown--;
    // Se o inimigo morrer, tem a animação de morte até ele sumir
    } else if (e->state == ENEMY_DEAD) {
        if (e->dead_frame < e->dead_max_frames - 1) {
            e->dead_frame_counter++;
            if (e->dead_frame_counter >= e->dead_frame_delay) {
                e->dead_frame++;
                e->dead_frame_counter = 0;
            }
        } else {
            if (e->dead_cooldown > 0) {
                e->dead_cooldown--;
            }
        }
    }
}

void enemy_draw(enemy *e, int camera_x) {
    if (!e) 
        return;
    // Indica para qual lado deve olhar o inimigo
    int flip;
    if (e->facing == 1) {
        flip = 0;
    } else {
        flip = ALLEGRO_FLIP_HORIZONTAL;
    }
    if (e->state == ENEMY_ALIVE) {
        // Desenha o sprite de ataque
        if (e->is_attacking) {
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
        // Desenha o sprite de andar
        } else {
            int frame_x = e->walk_frame * e->walk_frame_width;
            float walk_x_offset = -20.0;
            al_draw_scaled_bitmap(
                e->walk_sprite,
                frame_x, 0, e->walk_frame_width, e->walk_frame_height,
                e->x + walk_x_offset - camera_x, e->y,
                e->walk_frame_width * 2.0,
                e->walk_frame_height * 2.0,
                flip
            );
        }
    // Se ele morrer, desenha o sprite de morte
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

// Função para obter a colisão do inimigo
void enemy_get_collision_box(enemy *e, int *cx, int *cy, int *cw, int *ch) {
    int enemy_draw_width = e->walk_frame_width * 2.0;
    int enemy_draw_height = e->walk_frame_height * 2.0;
    int enemy_base_y = e->y + enemy_draw_height;
    *cw = (int)(enemy_draw_width * 0.1);   // Largura da hitbox
    *ch = (int)(enemy_draw_height * 0.5);  // Altura da hitbox
    *cx = (int)(e->x + (enemy_draw_width - *cw) / 2);
    *cy = enemy_base_y - *ch;
}

// Função para desenhar a hitbox do inimigo
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

// Função para atualizar todos os inimigos
void enemy_update_all(enemy *head) {
    for (enemy *e = head; e; e = e->next)
        enemy_update(e);
}

// Função para desenhar todos os inimigos
void enemy_draw_all(enemy *head, int camera_x, bool show_hitbox) {
    for (enemy *e = head; e; e = e->next) {
        enemy_draw(e, camera_x);
        enemy_draw_hitbox(e, camera_x, show_hitbox);
    }
}

// Função para destruir um inimigo
void enemy_destroy(enemy *e) {
    if (!e) return;
    if (e->walk_sprite) al_destroy_bitmap(e->walk_sprite);
    if (e->dead_sprite) al_destroy_bitmap(e->dead_sprite);
    if (e->attack_sprite) al_destroy_bitmap(e->attack_sprite);
    free(e);
}

// Função para destroir todos os inimigos na lista
void enemy_destroy_all(enemy *head) {
    while (head) {
        enemy *next = head->next;
        enemy_destroy(head);
        head = next;
    }
}

// Função para checar a colisão de balas com inimigos
void enemy_check_bullet_collisions(enemy *head, pistol *gun) {
    for (enemy *e = head; e; e = e->next) {
        if (e->state != ENEMY_ALIVE) continue;
        bullet **curr = &(gun->shots);
        int cx, cy, cw, ch;
        enemy_get_collision_box(e, &cx, &cy, &cw, &ch);
        while (*curr) {
            bullet *b = *curr;
            // Se a bala acertou ele, diminui sua vida (ou morre)
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

// Função para remover inimigos mortos da lista
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