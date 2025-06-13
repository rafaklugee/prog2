#include <stdlib.h>
#include "Player1.h"

player1* create_player1(unsigned char side, unsigned short x, unsigned short y, unsigned short max_x, unsigned short max_y) {
    if ((x - side/2 < 0) || (x + side/2 > max_x) || (y - side/2 < 0) || (y + side/2 > max_y))
        return NULL;
    
    player1 *p = (player1 *)malloc(sizeof(player1));
    if (!p) {
        return NULL; // Falha ao alocar memória
    }

    p->x = x;
    p->y = y;
    p->control = joystick_create();
    p->control->right = 0;
    p->control->left = 0;
    // p->gun = pistol_create();

    return p;
}

void player1_move(player1 *p, char steps, unsigned char trajectory, unsigned short max_x, unsigned short max_y) {
    if (!trajectory) {
        if (p->x - steps*PLAYER1_STEP >= 0) {
            p->x = p->x - steps * PLAYER1_STEP; // Mover para esquerda
        }
        else if (trajectory == 1) {
            if (p->x + steps*PLAYER1_STEP <= max_x) {
                p->x = p->x + steps * PLAYER1_STEP; // Mover para direita
            }
        }
    }
}

void player1_destroy(player1 *p) {
    joystick_destroy(p->control);
    //pistol_destroy(p->gun);
    free(p);
}