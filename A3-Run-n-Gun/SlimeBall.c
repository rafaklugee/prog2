#include <stdlib.h>
#include <allegro5/allegro_primitives.h>
#include "SlimeBall.h"
#include <stdbool.h>
#include "Enemy.h"
#include "Player1.h"
#include <allegro5/allegro_image.h>

extern bool show_hitboxes;

ALLEGRO_BITMAP *slime_sprite = NULL;

void slime_ball_load_sprite() {
    if (!slime_sprite)
        slime_sprite = al_load_bitmap("sprites/slime/Green_Slime/Slime_Coming.png");
}

void slime_ball_unload_sprite() {
    if (slime_sprite) {
        al_destroy_bitmap(slime_sprite);
        slime_sprite = NULL;
    }
}

slime_ball* slime_ball_create(float x, float y, float vx, float vy) {
    slime_ball *b = malloc(sizeof(slime_ball));
    b->x = x; b->y = y; b->vx = vx; b->vy = vy;
    b->active = 1;
    b->has_hit_player = 0;
    b->next = NULL;
    b->anim_frame = 0;
    b->anim_max_frames = 2;
    b->anim_frame_counter = 0;
    b->anim_frame_delay = 3;
    if (slime_sprite) {
        b->anim_frame_width = al_get_bitmap_width(slime_sprite) / b->anim_max_frames;
        b->anim_frame_height = al_get_bitmap_height(slime_sprite);
    } else {
        b->anim_frame_width = 32;
        b->anim_frame_height = 32;
    }
    return b;
}

void slime_ball_update(slime_ball **head, int world_width) {
    slime_ball **curr = head;
    while (*curr) {
        slime_ball *b = *curr;
        b->x += b->vx;
        b->y += b->vy;

        // Atualiza animação
        b->anim_frame_counter++;
        if (b->anim_frame_counter >= b->anim_frame_delay) {
            b->anim_frame = (b->anim_frame + 1) % b->anim_max_frames;
            b->anim_frame_counter = 0;
        }

        // Remove se sair da tela
        if (b->x < 0 || b->x > world_width || b->y < 0 || b->y > 600) {
            *curr = b->next;
            slime_ball_destroy(b);
        } else {
            curr = &b->next;
        }
    }
}

void slime_ball_draw(slime_ball *head, int camera_x) {
    for (slime_ball *b = head; b; b = b->next) {
        float cx = b->x - camera_x;
        float scale = 0.5; // Escala do slime
        float cy = b->y - 20; // Posição y do slime
        if (slime_sprite) {
            int frame_x = b->anim_frame * b->anim_frame_width;
            al_draw_scaled_bitmap(
                slime_sprite,
                frame_x, 0, b->anim_frame_width, b->anim_frame_height,
                cx - b->anim_frame_width * scale / 2, cy - b->anim_frame_height * scale / 2,
                b->anim_frame_width * scale, b->anim_frame_height * scale,
                0
            );
        } else {
            float r = 10 * scale;
            al_draw_filled_circle(cx, cy, r, al_map_rgb(0, 255, 0));
        }
        // Só desenha a hitbox se show_hitboxes estiver ativado
        if (show_hitboxes) {
            float r = b->anim_frame_width * scale / 2;
            al_draw_rectangle(cx - r, cy - r, cx + r, cy + r, al_map_rgb(255, 0, 0), 2);
        }
    }
}

void slime_ball_destroy(slime_ball *b) {
    free(b);
}