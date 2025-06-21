#ifndef __BACKGROUND__
#define __BACKGROUND__

#include <allegro5/allegro5.h>

typedef struct {
    // Sprites
    ALLEGRO_BITMAP *sky, *bg_far, *bg_mid, *bg_near;
    // Variáveis de escala e medidas
    int w_sky, w_far, w_mid, w_near;
    float scale, scaled_w_far, scaled_w_mid, scaled_w_near;
    int bg_repeat;
    int y_screen;
} Background;

Background* background_create(int y_screen, int bg_repeat);
void background_draw(Background *bg, float scroll_x, int x_screen);
void background_destroy(Background *bg);

#endif