#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "Background.h"

Background* background_create(const char *sky_path, const char *far_path, const char *mid_path, const char *near_path, int y_screen, int bg_repeat) {
    Background *bg = malloc(sizeof(Background));
    if (!bg) 
        return NULL;

    // Sprites
    bg->sky = al_load_bitmap(sky_path);
    bg->bg_far = al_load_bitmap(far_path);
    bg->bg_mid = al_load_bitmap(mid_path);
    bg->bg_near = al_load_bitmap(near_path);
    bg->y_screen = y_screen;
    bg->bg_repeat = bg_repeat;

    if (!bg->sky || !bg->bg_far || !bg->bg_mid || !bg->bg_near) {
        fprintf(stderr, "Falha ao carregar um ou mais backgrounds\n");
        if (bg->sky) al_destroy_bitmap(bg->sky);
        if (bg->bg_far) al_destroy_bitmap(bg->bg_far);
        if (bg->bg_mid) al_destroy_bitmap(bg->bg_mid);
        if (bg->bg_near) al_destroy_bitmap(bg->bg_near);
        free(bg);
        return NULL;
    }

    // Captura as medidas do background
    bg->w_sky = al_get_bitmap_width(bg->sky);
    bg->w_far = al_get_bitmap_width(bg->bg_far);
    bg->w_mid = al_get_bitmap_width(bg->bg_mid);
    bg->w_near = al_get_bitmap_width(bg->bg_near);

    // Inicializa as escalas
    bg->scale = (float)y_screen / al_get_bitmap_height(bg->bg_far);
    bg->scaled_w_far = bg->w_far * bg->scale;
    bg->scaled_w_mid = bg->w_mid * bg->scale;
    bg->scaled_w_near = bg->w_near * bg->scale;

    return bg;
}

// Desenha o background com base no scroll horizontal
void background_draw(Background *bg, float scroll_x, int x_screen) {
    // Camada 1: chão (parallax rápido)
    for (int i = 0; i < bg->bg_repeat; i++) {
        al_draw_scaled_bitmap(bg->bg_near, 0, 0, bg->w_near, al_get_bitmap_height(bg->bg_near),
            i * bg->scaled_w_near - scroll_x, 0, bg->scaled_w_near, bg->y_screen, 0);
    }

    // Camada 2: céu
    al_draw_scaled_bitmap(bg->sky, 0, 0, bg->w_sky, al_get_bitmap_height(bg->sky), 0, 0, x_screen, bg->y_screen, 0);

    // Camada 3: casas e árvores distantes (parallax mais lento)
    float far_scroll = fmod(scroll_x * 0.3, bg->scaled_w_far);
    al_draw_scaled_bitmap(bg->bg_far, 0, 0, bg->w_far, al_get_bitmap_height(bg->bg_far), -far_scroll, 0, bg->scaled_w_far, bg->y_screen, 0);
    al_draw_scaled_bitmap(bg->bg_far, 0, 0, bg->w_far, al_get_bitmap_height(bg->bg_far), -far_scroll + bg->scaled_w_far, 0, bg->scaled_w_far, bg->y_screen, 0);

    // Camada 4: casas (parallax médio)
    float mid_scroll = fmod(scroll_x * 0.6, bg->scaled_w_mid);
    al_draw_scaled_bitmap(bg->bg_mid, 0, 0, bg->w_mid, al_get_bitmap_height(bg->bg_mid), -mid_scroll, 0, bg->scaled_w_mid, bg->y_screen, 0);
    al_draw_scaled_bitmap(bg->bg_mid, 0, 0, bg->w_mid, al_get_bitmap_height(bg->bg_mid), -mid_scroll + bg->scaled_w_mid, 0, bg->scaled_w_mid, bg->y_screen, 0);

    // Camada 5: chão (parallax rápido)
    float near_scroll = fmod(scroll_x, bg->scaled_w_near);
    al_draw_scaled_bitmap(bg->bg_near, 0, 0, bg->w_near, al_get_bitmap_height(bg->bg_near), -near_scroll, 0, bg->scaled_w_near, bg->y_screen, 0);
    al_draw_scaled_bitmap(bg->bg_near, 0, 0, bg->w_near, al_get_bitmap_height(bg->bg_near), -near_scroll + bg->scaled_w_near, 0, bg->scaled_w_near, bg->y_screen, 0);
}

// Destrói o background
void background_destroy(Background *bg) {
    if (!bg) return;
    if (bg->sky) al_destroy_bitmap(bg->sky);
    if (bg->bg_far) al_destroy_bitmap(bg->bg_far);
    if (bg->bg_mid) al_destroy_bitmap(bg->bg_mid);
    if (bg->bg_near) al_destroy_bitmap(bg->bg_near);
    free(bg);
}