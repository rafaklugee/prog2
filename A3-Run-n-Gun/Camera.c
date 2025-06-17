#include "Camera.h"

int camera_calculate(
    int player_world_x,
    int current_camera_x,
    int player_control_left,
    int player_control_right,
    int camera_left_margin,
    int camera_right_margin,
    int world_width,
    int x_screen
) {
    int camera_min_x = 0;
    int camera_max_x = world_width - x_screen;
    if (camera_max_x < camera_min_x) camera_max_x = camera_min_x;

    int player_screen_x_current_frame = player_world_x - current_camera_x;

    // Movimento para a direita
    if (player_control_right) {
        if (player_screen_x_current_frame > camera_right_margin) {
            current_camera_x += (player_screen_x_current_frame - camera_right_margin);
        }
    }
    // Movimento para a esquerda
    else if (player_control_left) {
        if (player_screen_x_current_frame < camera_left_margin) {
            current_camera_x -= (camera_left_margin - player_screen_x_current_frame);
        }
    }

    // Limites do mundo
    if (current_camera_x < camera_min_x) current_camera_x = camera_min_x;
    if (current_camera_x > camera_max_x) current_camera_x = camera_max_x;

    return current_camera_x;
}