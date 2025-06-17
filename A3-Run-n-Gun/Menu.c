#include "Menu.h"

int show_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    const char* options[] = {"Iniciar Jogo", "Sair"};
    int selected = 0;
    bool in_menu = true;
    ALLEGRO_EVENT event;

    while (in_menu) {
        // Desenha o background do menu
        if (bg) {
            background_draw(bg, 0, 800);
        } else {
            al_clear_to_color(al_map_rgb(30, 30, 30));
        }
        for (int i = 0; i < 2; i++) {
            ALLEGRO_COLOR color = al_map_rgb(200, 200, 200);
            if (i == selected) {
                color = al_map_rgb(255, 255, 0); // Destaca a opção selecionada
            }
            al_draw_text(font, color, 400, 250 + i * 40, ALLEGRO_ALIGN_CENTRE, options[i]);
        }
        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                selected = (selected + 1) % 2;
            } else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
                selected = (selected + 1) % 2;
            } else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
                return selected;
            } else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                return 1; // Sair
            }
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            return 1; // Sair
        }
    }
    return 1;
}