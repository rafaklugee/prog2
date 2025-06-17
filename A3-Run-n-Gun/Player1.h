#ifndef __PLAYER1__
#define __PLAYER1__

#include <allegro5/allegro5.h>
#include "Joystick.h"
#include "Pistol.h"

#define PLAYER1_STEP 5

typedef struct {
    joystick *control;
    pistol *gun;

    // Sprites
    ALLEGRO_BITMAP *idle;
    ALLEGRO_BITMAP *idle_left;
    ALLEGRO_BITMAP *run;
    ALLEGRO_BITMAP *squat;
    ALLEGRO_BITMAP *jump;

    // Animação
    int run_frame, run_max_frames, run_frame_width, run_frame_height;
    int squat_frame, squat_max_frames, squat_frame_width, squat_frame_height;
    int squat_out_frame, squat_out_max_frames, squat_out_frame_width, squat_out_frame_height;
    int jump_frame, jump_max_frames, jump_frame_width, jump_frame_height;
    int idle_max_frames, idle_frame, idle_frame_width, idle_frame_height, idle_frame_delay;
    int frame_counter, frame_delay;
    int is_squatting, squat_anim_done, is_squatting_out;
    int is_jumping;
    float jump_velocity, gravity, jump_strength;
    float player_y;
    float player_scale;
    int last_dir;
} player1;

player1* create_player1(unsigned short initial_y, unsigned short max_x, unsigned short max_y);
int player1_load_sprites(player1 *p, const char *idle, const char *idle_left, const char *run, const char *squat, const char *jump);
void player1_update(player1 *p, int *player_world_x, int world_width, int player_screen_y);
void player1_draw(player1 *p, int player_screen_x, int player_screen_y);
void player1_handle_event(player1 *p, ALLEGRO_EVENT *event);
void player1_destroy(player1 *p);

#endif