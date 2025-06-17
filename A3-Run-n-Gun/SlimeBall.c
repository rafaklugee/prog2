#include <stdlib.h>
#include <allegro5/allegro_primitives.h>
#include "SlimeBall.h"
#include <stdbool.h>
#include "Enemy.h"
#include "Player1.h"

extern bool show_hitboxes;

slime_ball* slime_ball_create(float x, float y, float vx, float vy) {
    slime_ball *b = malloc(sizeof(slime_ball));
    b->x = x; b->y = y; b->vx = vx; b->vy = vy;
    b->active = 1;
    b->has_hit_player = 0;
    b->next = NULL;
    return b;
}

void slime_ball_update(slime_ball **head, int world_width) {
    slime_ball **curr = head;
    while (*curr) {
        slime_ball *b = *curr;
        b->x += b->vx;
        b->y += b->vy;
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
        float cy = b->y;
        float r = 10;
        al_draw_filled_circle(cx, cy, r, al_map_rgb(0, 255, 0));
        // Só desenha a hitbox se show_hitboxes estiver ativado
        if (show_hitboxes) {
            al_draw_rectangle(cx - r, cy - r, cx + r, cy + r, al_map_rgb(255, 0, 0), 2);
        }
    }
}

void slime_ball_destroy(slime_ball *b) {
    free(b);
}