#include <stdio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include "Menu.h"


extern ALLEGRO_SAMPLE *main_music;
extern ALLEGRO_SAMPLE_ID main_music_id;
extern float global_volume;

int show_main_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    const char* options[] = {"Iniciar Jogo", "Configuracoes", "Sair"};
    int selected = 0;
    bool in_menu = true;
    ALLEGRO_EVENT event;

    while (in_menu) {
        if (bg) background_draw(bg, 0, 800);
        else al_clear_to_color(al_map_rgb(30, 30, 30));
        for (int i = 0; i < 3; i++) {
            ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(200, 200, 200);
            al_draw_text(font, color, 400, 250 + i * 40, ALLEGRO_ALIGN_CENTRE, options[i]);
        }
        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                selected = (selected + 1) % 3;
            else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                selected = (selected + 2) % 3;
            else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
                if (selected == 0) return 0; // Iniciar Jogo
                if (selected == 1) show_settings_menu(disp, font, queue, bg); // Configurações
                if (selected == 2) return 1; // Sair
            } else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                return 1; // Sair
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            return 1; // Sair
        }
    }
    return 1;
}

void draw_pause_menu(ALLEGRO_FONT* font) {
    al_draw_filled_rectangle(200, 250, 600, 350, al_map_rgba(0,0,0,180));
    al_draw_text(font, al_map_rgb(255,255,0), 400, 280, ALLEGRO_ALIGN_CENTRE, "PAUSADO");
}

int show_gameover_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_FONT* big_font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    const char* options[] = {"Jogar novamente", "Sair"};
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

        al_draw_text(big_font, al_map_rgb(200, 200, 200), 400, 180, ALLEGRO_ALIGN_CENTRE, "GAME OVER");

        for (int i = 0; i < 2; i++) {
            ALLEGRO_COLOR color = al_map_rgb(200, 200, 200);
            if (i == selected) {
                color = al_map_rgb(255, 255, 0); // Destaca a opção selecionada
            }
            al_draw_text(font, color, 400, 270 + i * 40, ALLEGRO_ALIGN_CENTRE, options[i]);
        }
        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN || event.keyboard.keycode == ALLEGRO_KEY_UP) {
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

int show_victory_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_FONT* big_font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    const char* options[] = {"Menu principal", "Sair"};
    int selected = 0;
    bool in_menu = true;
    ALLEGRO_EVENT event;

    while (in_menu) {
        if (bg) {
            background_draw(bg, 0, 800);
        } else {
            al_clear_to_color(al_map_rgb(30, 30, 30));
        }

        al_draw_text(big_font, al_map_rgb(200, 255, 100), 400, 180, ALLEGRO_ALIGN_CENTRE, "VITORIA!");

        for (int i = 0; i < 2; i++) {
            ALLEGRO_COLOR color = al_map_rgb(200, 200, 200);
            if (i == selected) color = al_map_rgb(255, 255, 0);
            al_draw_text(font, color, 400, 270 + i * 40, ALLEGRO_ALIGN_CENTRE, options[i]);
        }
        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN || event.keyboard.keycode == ALLEGRO_KEY_UP) {
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

int show_difficulty_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    const char* options[] = {"Facil", "Medio", "Dificil"};
    int selected = 0;
    bool in_menu = true;
    ALLEGRO_EVENT event;

    while (in_menu) {
        if (bg) background_draw(bg, 0, 800);
        else al_clear_to_color(al_map_rgb(30, 30, 30));
        for (int i = 0; i < 3; i++) {
            ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(200, 200, 200);
            al_draw_text(font, color, 400, 250 + i * 40, ALLEGRO_ALIGN_CENTRE, options[i]);
        }
        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                selected = (selected + 1) % 3;
            else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                selected = (selected + 2) % 3;
            else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
                return selected;
            else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                return -1; // Voltar
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            return -1; // Voltar
        }
    }
    return -1;
}

int show_settings_menu(ALLEGRO_DISPLAY* disp, ALLEGRO_FONT* font, ALLEGRO_EVENT_QUEUE* queue, Background* bg) {
    bool in_menu = true;
    ALLEGRO_EVENT event;

    int int_volume = (int)(global_volume * 100);

    while (in_menu) {
        if (bg) background_draw(bg, 0, 800);
        else al_clear_to_color(al_map_rgb(30, 30, 30));

        // Desenha a barra de volume
        int bar_x = 250, bar_y = 300, bar_w = 300, bar_h = 30;
        al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, al_map_rgb(60,60,60));
        int filled = (int)((int_volume / 100.0f) * bar_w);
        al_draw_filled_rectangle(bar_x, bar_y, bar_x + filled, bar_y + bar_h, al_map_rgb(42, 19, 61));
        al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, al_map_rgb(255,255,255), 2);

        // Texto do volume
        char vol_str[32];
        snprintf(vol_str, sizeof(vol_str), "Volume: %d/100", int_volume);
        al_draw_text(font, al_map_rgb(200,200,200), 400, bar_y + bar_h + 10, ALLEGRO_ALIGN_CENTRE, vol_str);

        al_draw_text(font, al_map_rgb(180,180,180), 400, 400, ALLEGRO_ALIGN_CENTRE, "Use as setas para ajustar o volume");

        al_flip_display();

        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                int_volume -= 5;
                if (int_volume < 0) int_volume = 0;
                global_volume = int_volume / 100.0f;
                // Sincroniza o volume da música principal
                al_stop_sample(&main_music_id);
                al_play_sample(main_music, global_volume * 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_music_id);
            } else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                int_volume += 5;
                if (int_volume > 100) int_volume = 100;
                global_volume = int_volume / 100.0f;
                // Sincroniza o volume da música principal
                al_stop_sample(&main_music_id);
                al_play_sample(main_music, global_volume * 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_music_id);
            } else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE || event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                in_menu = false;
            }
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            in_menu = false;
        }
    }
    return 0;
}