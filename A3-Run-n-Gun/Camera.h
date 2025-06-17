#ifndef CAMERA_H
#define CAMERA_H

int camera_calculate(
    int player_world_x,
    int current_camera_x,
    int player_control_left,
    int player_control_right,
    int camera_left_margin,
    int camera_right_margin,
    int world_width,
    int x_screen
);

#endif