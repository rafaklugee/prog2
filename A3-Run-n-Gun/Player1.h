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
    ALLEGRO_BITMAP *shot;
    ALLEGRO_BITMAP *squat_shot;
    ALLEGRO_BITMAP *hurt;
    ALLEGRO_BITMAP *dead;
    ALLEGRO_BITMAP *hud_health;

    // Variáveis para animação das sprites
    int run_frame, run_max_frames, run_frame_width, run_frame_height;
    int squat_frame, squat_max_frames, squat_frame_width, squat_frame_height,
        squat_out_frame, squat_out_max_frames, squat_out_frame_width, squat_out_frame_height,
        is_squatting, squat_anim_done, is_squatting_out, is_squat_shooting;
    int jump_frame, jump_max_frames, jump_frame_width, jump_frame_height, is_jumping;
    int idle_max_frames, idle_frame, idle_frame_width, idle_frame_height, idle_frame_delay;
    int shot_max_frames, shot_frame, shot_frame_width, shot_frame_height, is_shooting, shooting_frames, is_shooting_pressed;
    int is_hurt, hurt_frame, hurt_max_frames, hurt_frame_width, hurt_frame_height, hurt_frame_delay, hurt_frame_counter;
    int is_dead, dead_frame, dead_max_frames, dead_frame_width, dead_frame_height, 
        dead_frame_delay, dead_frame_counter, dead_menu_cooldown;
    float jump_velocity, gravity, jump_strength;

    // Variáveis de controle geral
    int frame_counter, frame_delay;
    int last_dir;
    int health; 
    float player_y;
    float player_scale;
    
} player1;

player1* create_player1(unsigned short initial_y, unsigned short max_x, unsigned short max_y);
int player1_load_sprites(player1 *p, const char *idle, const char *idle_left, const char *run,
                        const char *squat, const char *jump, const char *shot, const char *squat_shot, 
                        const char *hurt, const char *dead, const char *hud_health);
void player1_get_hitbox(player1 *p, int player_screen_x, int player_screen_y, int *hx, int *hy, int *hw, int *hh);
void player1_update(player1 *p, int *player_world_x, int world_width, int player_screen_y);
void player1_draw(player1 *p, int player_screen_x, int player_screen_y);
void player1_handle_event(player1 *p, ALLEGRO_EVENT *event, int player_world_x);
void player1_update_bullets(player1 *p, int world_width);
void player1_draw_bullets(player1 *p, int camera_x);
void player1_destroy(player1 *p);

#endif