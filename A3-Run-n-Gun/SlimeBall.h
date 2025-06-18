#ifndef __SLIME_BALL_H__
#define __SLIME_BALL_H__

#include "Player1.h"
#include <allegro5/allegro5.h>

typedef struct slime_ball {
    float x, y;
    float vx, vy;
    int active;
    int has_hit_player;
    int anim_frame, anim_max_frames, anim_frame_width, anim_frame_height;
    int anim_frame_counter, anim_frame_delay;
    struct slime_ball *next;
} slime_ball;

slime_ball* slime_ball_create(float x, float y, float vx, float vy);
void slime_ball_update(slime_ball **head, int world_width);
void slime_ball_draw(slime_ball *head, int camera_x);
void slime_ball_destroy(slime_ball *b);

void slime_ball_load_sprite();
void slime_ball_unload_sprite();

#endif