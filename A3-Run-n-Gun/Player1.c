#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "Player1.h"

#define DIR_RIGHT 1
#define DIR_LEFT -1

player1* create_player1(unsigned short initial_y, unsigned short max_x, unsigned short max_y) {
    player1 *p = (player1 *)malloc(sizeof(player1));
    if (!p) return NULL;

    p->control = joystick_create();
    p->gun = NULL; // p->gun = pistol_create();
    p->player_scale = 2.0f;
    p->gravity = 1.2f;
    p->jump_strength = -18.0f;
    p->is_jumping = 0;
    p->player_y = initial_y;
    p->last_dir = DIR_RIGHT;
    p->frame_delay = 3;
    p->frame_counter = 0;
    p->is_squatting = 0;
    p->squat_anim_done = 0;
    p->is_squatting_out = 0;
    return p;
}

int player1_load_sprites(player1 *p, const char *idle, const char *idle_left, const char *run, const char *squat, const char *jump) {
    p->idle = al_load_bitmap(idle);
    p->idle_left = al_load_bitmap(idle_left);
    p->run = al_load_bitmap(run);
    p->squat = al_load_bitmap(squat);
    p->jump = al_load_bitmap(jump);

    if (!p->idle || !p->idle_left || !p->run || !p->squat || !p->jump)
        return 0;

    // Frames e dimensões
    p->run_max_frames = 10;
    p->run_frame = 0;
    p->run_frame_width = al_get_bitmap_width(p->run) / p->run_max_frames;
    p->run_frame_height = al_get_bitmap_height(p->run);

    p->squat_max_frames = 3;
    p->squat_frame = 0;
    p->squat_frame_width = al_get_bitmap_width(p->squat) / p->squat_max_frames;
    p->squat_frame_height = al_get_bitmap_height(p->squat);

    p->jump_max_frames = 10;
    p->jump_frame = 0;
    p->jump_frame_width = al_get_bitmap_width(p->jump) / p->jump_max_frames;
    p->jump_frame_height = al_get_bitmap_height(p->jump);

    p->idle_max_frames = 6;
    p->idle_frame = 0;
    p->idle_frame_width = al_get_bitmap_width(p->idle) / p->idle_max_frames;
    p->idle_frame_height = al_get_bitmap_height(p->idle);
    p->idle_frame_delay = 7;

    return 1;
}

void player1_update(player1 *p, int *player_world_x, int world_width, int player_screen_y) {
    // Atualiza agachamento
    if (!p->is_squatting && !(p->control->left && p->control->right)) {
        if (p->control->left && *player_world_x - PLAYER1_STEP >= 0) {
            *player_world_x -= PLAYER1_STEP;
        }
        if (p->control->right) {
            *player_world_x += PLAYER1_STEP;
        }
    }

    // Atualiza salto
    if (p->is_jumping) {
        p->jump_velocity += p->gravity;
        p->player_y += p->jump_velocity;
        if (p->player_y >= player_screen_y) {
            p->player_y = player_screen_y;
            p->is_jumping = 0;
            p->jump_frame = 0;
            p->jump_velocity = 0;
        }
    } else {
        p->player_y = player_screen_y;
    }
}

void player1_draw(player1 *p, int player_screen_x, int player_screen_y) {
    ALLEGRO_BITMAP *sprite = p->idle;
    int inicio_x = 0;
    int flip = 0;
    int idle_frame_height = al_get_bitmap_height(p->idle);
    int squat_y_offset = (idle_frame_height - p->squat_frame_height) * p->player_scale;

    // Lógica de animação
    if (p->is_jumping) {
        sprite = p->jump;
        flip = (p->last_dir == DIR_LEFT) ? ALLEGRO_FLIP_HORIZONTAL : 0;
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->jump_frame = (p->jump_frame + 1) % p->jump_max_frames;
            p->frame_counter = 0;
        }
        inicio_x = p->jump_frame * p->jump_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->jump_frame_width, p->jump_frame_height,
            player_screen_x, p->player_y,
            p->jump_frame_width * p->player_scale,
            p->jump_frame_height * p->player_scale,
            flip
        );
        return;
    }
    else if (p->control->down) {
        // Squat In
        if (!p->squat_anim_done) {
            sprite = p->squat;
            flip = (p->last_dir == DIR_LEFT) ? ALLEGRO_FLIP_HORIZONTAL : 0;
            p->frame_counter++;
            if (p->frame_counter >= p->frame_delay) {
                p->squat_frame++;
                p->frame_counter = 0;
            }
            if (p->squat_frame >= p->squat_max_frames - 1) {
                p->squat_frame = p->squat_max_frames - 1;
                p->squat_anim_done = 1;
                p->is_squatting = 1;
            }
            inicio_x = p->squat_frame * p->squat_frame_width;
        } else {
            sprite = p->squat;
            flip = (p->last_dir == DIR_LEFT) ? ALLEGRO_FLIP_HORIZONTAL : 0;
            inicio_x = (p->squat_max_frames - 1) * p->squat_frame_width;
            p->is_squatting = 1;
        }
        float squat_scale = p->player_scale;
        float x_offset = 0;
        if (flip == ALLEGRO_FLIP_HORIZONTAL) {
            x_offset = -40; // ajuste negativo para flip, mude o valor conforme necessário
        }
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->squat_frame_width, p->squat_frame_height,
            player_screen_x + 20 + x_offset, p->player_y + squat_y_offset,
            p->squat_frame_width * squat_scale,
            p->squat_frame_height * squat_scale,
            flip
        );
        return;
    }
    else if (p->control->left && p->control->right) {
        p->squat_frame = 0;
        p->squat_anim_done = 0;
        p->is_squatting = 0;
        sprite = (p->last_dir == DIR_LEFT) ? p->idle_left : p->idle;
    }
    else if (p->control->left) {
        sprite = p->run;
        flip = ALLEGRO_FLIP_HORIZONTAL;
        p->last_dir = DIR_LEFT;
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->run_frame = (p->run_frame + 1) % p->run_max_frames;
            p->frame_counter = 0;
        }
        inicio_x = p->run_frame * p->run_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->run_frame_width, p->run_frame_height,
            player_screen_x, p->player_y,
            p->run_frame_width * p->player_scale,
            p->run_frame_height * p->player_scale,
            flip
        );
        return;
    }
    else if (p->control->right) {
        sprite = p->run;
        flip = 0;
        p->last_dir = DIR_RIGHT;
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->run_frame = (p->run_frame + 1) % p->run_max_frames;
            p->frame_counter = 0;
        }
        inicio_x = p->run_frame * p->run_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->run_frame_width, p->run_frame_height,
            player_screen_x, p->player_y,
            p->run_frame_width * p->player_scale,
            p->run_frame_height * p->player_scale,
            flip
        );
        return;
    }
    // Idle
    p->squat_frame = 0;
    p->squat_anim_done = 0;
    p->is_squatting = 0;
    sprite = (p->last_dir == DIR_LEFT) ? p->idle_left : p->idle;

    // Atualiza frame do Idle
    p->frame_counter++;
    if (p->frame_counter >= p->idle_frame_delay) {
        p->idle_frame = (p->idle_frame + 1) % p->idle_max_frames;
        p->frame_counter = 0;
    }
    int idle_frame_x = p->idle_frame * p->idle_frame_width;

    al_draw_scaled_bitmap(
        sprite,
        idle_frame_x, 0, p->idle_frame_width, p->idle_frame_height,
        player_screen_x, p->player_y,
        p->idle_frame_width * p->player_scale,
        p->idle_frame_height * p->player_scale,
        0
    );
}

void player1_handle_event(player1 *p, ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (event->keyboard.keycode == ALLEGRO_KEY_A) {
            if (p->is_squatting) { p->control->down = 0; p->is_squatting = 0; p->squat_anim_done = 0; p->squat_frame = 0; }
            p->control->left = 1;
        }
        else if (event->keyboard.keycode == ALLEGRO_KEY_D) {
            if (p->is_squatting) { p->control->down = 0; p->is_squatting = 0; p->squat_anim_done = 0; p->squat_frame = 0; }
            p->control->right = 1;
        }
        else if (event->keyboard.keycode == ALLEGRO_KEY_W) {
            if (!p->is_jumping && !p->is_squatting) {
                p->is_jumping = 1;
                p->jump_velocity = p->jump_strength;
            }
        }
        else if (event->keyboard.keycode == ALLEGRO_KEY_S)
            p->control->down = 1;
    }
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        if (event->keyboard.keycode == ALLEGRO_KEY_A)
            p->control->left = 0;
        else if (event->keyboard.keycode == ALLEGRO_KEY_D)
            p->control->right = 0;
        else if (event->keyboard.keycode == ALLEGRO_KEY_S) {
            p->control->down = 0;
            if (p->is_squatting) {
                p->is_squatting = 0;
                p->squat_anim_done = 0;
                p->squat_frame = 0;
                p->is_squatting_out = 1;
                p->frame_counter = 0;
            }
        }
    }
}

void player1_destroy(player1 *p) {
    joystick_destroy(p->control);
    if (p->idle) al_destroy_bitmap(p->idle);
    if (p->idle_left) al_destroy_bitmap(p->idle_left);
    if (p->run) al_destroy_bitmap(p->run);
    if (p->squat) al_destroy_bitmap(p->squat);
    if (p->jump) al_destroy_bitmap(p->jump);
    //if (p->gun) pistol_destroy(p->gun);
    free(p);
}