#ifndef __PLAYER1__
#define __PLAYER1__

#include "Joystick.h"
#include "Pistol.h"

#define PLAYER1_STEP 5

typedef struct {
    unsigned short x;
    unsigned short y;
    joystick *control;
    pistol *gun;
} player1;

player1* create_player1(unsigned char side, unsigned short x, unsigned short y, unsigned short max_x, unsigned short max_y);
void player1_move(player1 *p, char steps, unsigned char trajectory, unsigned short max_x, unsigned short max_y);
void player1_fire(player1 *p);
void player1_destroy(player1 *p);

#endif