#include <stdlib.h>
#include <allegro5/allegro_primitives.h>
#include "SlimeBall.h"
#include <stdbool.h>
#include "Enemy.h"
#include "Player1.h"
#include <allegro5/allegro_image.h>

extern bool show_hitboxes;

ALLEGRO_BITMAP *slime_sprite_green = NULL;
ALLEGRO_BITMAP *slime_sprite_blue = NULL;
ALLEGRO_BITMAP *slime_sprite_red = NULL;

// Sprites das slime balls
void slime_ball_load_sprite() {
    if (!slime_sprite_green)
        slime_sprite_green = al_load_bitmap("sprites/slime/Green_Slime/Slime_Coming.png");
    if (!slime_sprite_blue)
        slime_sprite_blue = al_load_bitmap("sprites/slime/Blue_Slime/Slime_Coming.png");
    if (!slime_sprite_red)
        slime_sprite_red = al_load_bitmap("sprites/slime/Red_Slime/Slime_Coming.png");
}

// Inicializa as slime balls com os atributos iniciais
slime_ball* slime_ball_create(float x, float y, float vx, float vy, slime_type type) {
    slime_ball *b = malloc(sizeof(slime_ball));
    b->x = x; b->y = y; b->vx = vx; b->vy = vy;
    b->active = 1;
    b->has_hit_player = 0;
    b->next = NULL;
    b->anim_frame = 0;
    b->anim_max_frames = 2;
    b->anim_frame_counter = 0;
    b->anim_frame_delay = 3;
    b->type = type;
    b->zigzag = 0;
    b->boss_zigzag = 0;

    // A slime ball vermelha tem tamanho maior
    if (type == SLIME_RED)
        b->scale = 1.5f;
    else
        b->scale = 0.5f;

    // Define as dimensões do frame de animação de acordo com o sprite da slime ball
    ALLEGRO_BITMAP *sprite = slime_sprite_green;
    if (type == SLIME_BLUE) sprite = slime_sprite_blue;
    if (type == SLIME_RED) sprite = slime_sprite_red;
    if (sprite) {
        b->anim_frame_width = al_get_bitmap_width(sprite) / b->anim_max_frames;
        b->anim_frame_height = al_get_bitmap_height(sprite);
    } else {
        b->anim_frame_width = 32;
        b->anim_frame_height = 32;
    }
    return b;
}

// Lógicas de update das slime balls
void slime_ball_update(slime_ball **head, int world_width) {
    slime_ball **curr = head;
    while (*curr) {
        slime_ball *b = *curr;

        // Movimento em zigzag para slime verde
        if (b->type == SLIME_GREEN && b->boss_zigzag) {
            b->zigzag++;
            int periodo = 12; // Quantos frames dura cada subida ou descida
            float amplitude = 3; // O quanto sobe ou desce por frame

            // Alterna o vy a cada período de frames
            if ((b->zigzag / periodo) % 2 == 0) {
                b->vy = amplitude;
            } else {
                b->vy = -amplitude;
            }
        }

        b->x += b->vx;
        b->y += b->vy;

        // Atualiza animação da slime ball
        b->anim_frame_counter++;
        if (b->anim_frame_counter >= b->anim_frame_delay) {
            b->anim_frame = (b->anim_frame + 1) % b->anim_max_frames;
            b->anim_frame_counter = 0;
        }

        // Remove a slime ball se sair da tela ou atingir 600 pixels de distância
        if (b->x < 0 || b->x > world_width || b->y < 0 || b->y > 600) {
            *curr = b->next;
            slime_ball_destroy(b);
        } else {
            curr = &b->next;
        }
    }
}

// Desenho das slime balls
void slime_ball_draw(slime_ball *head, int camera_x) {
    // Cada slime ball é desenhada na posição relativa à câmera
    for (slime_ball *b = head; b; b = b->next) {
        float cx = b->x - camera_x;
        float scale = b->scale;

        ALLEGRO_BITMAP *sprite = slime_sprite_green;
        if (b->type == SLIME_BLUE) sprite = slime_sprite_blue;
        if (b->type == SLIME_RED) sprite = slime_sprite_red;
        float cy = b->y - 20;

        // Desenha a slime ball propriamente
        if (sprite) {
            int frame_x = b->anim_frame * b->anim_frame_width;
            al_draw_scaled_bitmap(
                sprite,
                frame_x, 0, b->anim_frame_width, b->anim_frame_height,
                cx - b->anim_frame_width * scale / 2, cy - b->anim_frame_height * scale / 2,
                b->anim_frame_width * scale, b->anim_frame_height * scale,
                0
            );
        }
        // Desenha a hitbox das slime balls de acordo com seus tamanhos/tipos
        if (show_hitboxes) {
            float r = b->anim_frame_width * b->scale / 2.0f;
            if (b->type == SLIME_RED) {
                r *= 0.75f;
            }
            if (b->type == SLIME_GREEN) {
                r *= 0.6f;
            }
            float hitbox_cy = cy;
            if (b->type == SLIME_RED) {
                hitbox_cy += 80;
            }
            if (b->type == SLIME_GREEN) {
                hitbox_cy += 10;
            }
            al_draw_rectangle(cx - r, hitbox_cy - r, cx + r, hitbox_cy + r, al_map_rgb(255, 0, 0), 2);
        }
    }
}

// Destrói uma slime ball
void slime_ball_destroy(slime_ball *b) {
    free(b);
}

// Destrói todas as slime balls na lista encadeada
void slime_ball_destroy_all(slime_ball **head) {
    while (*head) {
        slime_ball *tmp = *head;
        *head = tmp->next;
        free(tmp);
    }
}