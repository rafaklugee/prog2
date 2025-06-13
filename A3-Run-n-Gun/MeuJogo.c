//Compilação: gcc MeuJogo.c -o game $(pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_primitives-5 --libs --cflags)

#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>

#include "Player1.h"

#define X_SCREEN 800
#define Y_SCREEN 600

void update_position(player1 *p) {
    if (p->control->left) {
        player1_move(p, 1, 0, X_SCREEN, Y_SCREEN); // Mover para a esquerda
    }
    if (p->control->right) {
        player1_move(p, 1, 1, X_SCREEN, Y_SCREEN); // Mover para a direita
    }
}

#define DIR_RIGHT 1
#define DIR_LEFT  -1
int last_dir = DIR_RIGHT;

int main(){
	
	al_init();	//Faz a preparação de requisitos da biblioteca Allegro
	al_init_primitives_addon();	//Faz a inicialização dos addons das imagens básicas
    al_init_image_addon(); // Habilita o addon de imagens, que permite carregar imagens em formatos como PNG, JPEG, etc.
    al_init_font_addon(); // Habilita o addon de fontes, que permite carregar e usar fontes TrueType (TTF) e bitmap
	al_install_keyboard(); //Habilita a entrada via teclado (eventos de teclado), no programa

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);	// Cria o relógio do jogo; isso indica quantas atualizações serão realizadas por segundo (30, neste caso)
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue(); // Cria a fila de eventos; todos os eventos (programação orientada a eventos) 
	ALLEGRO_FONT* font = al_create_builtin_font(); // Carrega uma fonte padrão para escrever na tela (é bitmap, mas também suporta adicionar fontes ttf)
	ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN); // Cria uma janela para o programa, define a largura (x) e a altura (y) da tela em píxeis (800x600, neste caso)

	al_register_event_source(queue, al_get_keyboard_event_source()); // Indica que eventos de teclado serão inseridos na nossa fila de eventos
	al_register_event_source(queue, al_get_display_event_source(disp)); // Indica que eventos de tela serão inseridos na nossa fila de eventos
	al_register_event_source(queue, al_get_timer_event_source(timer)); // Indica que eventos de relógio serão inseridos na nossa fila de eventos

    // Carrega imagens do background do mapa
    ALLEGRO_BITMAP *sky, *bg_far, *bg_mid, *bg_near;
    sky = al_load_bitmap("img/PNG/Postapocalypce2/Bright/sky.png");
    bg_far = al_load_bitmap("img/PNG/Postapocalypce2/Bright/houses&trees_bg.png");
    bg_mid = al_load_bitmap("img/PNG/Postapocalypce2/Bright/houses.png");
    bg_near = al_load_bitmap("img/PNG/Postapocalypce2/Bright/road.png");

    if (!sky || !bg_far || !bg_mid || !bg_near) {
        fprintf(stderr, "Falha ao carregar um ou mais backgrounds\n");
        if (sky) al_destroy_bitmap(sky);
        if (bg_far) al_destroy_bitmap(bg_far);
        if (bg_mid) al_destroy_bitmap(bg_mid);
        if (bg_near) al_destroy_bitmap(bg_near);
        return -1;
    }

    // Carrega sprites do player
    ALLEGRO_BITMAP *player_idle = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle.png");
    ALLEGRO_BITMAP *player_idle_left = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle_Otherside.png");
    ALLEGRO_BITMAP *player_run = al_load_bitmap("sprites/gangsters/Gangsters_1/Run.png");
    if (!player_idle || !player_idle_left || !player_run) {
        fprintf(stderr, "Falha ao carregar sprites do player\n");
        return -1;
    }

    // Variáveis do background
    float scale = (float)Y_SCREEN / al_get_bitmap_height(bg_far);
    float scroll_x = 0; // Variável para controlar o deslocamento horizontal do fundo
    bool redraw = true; // Variável para controlar quando a tela deve ser redesenhada
    bool running = true; // Variável para controlar o loop principal do jogo

    player1 *p = create_player1(50, X_SCREEN / 2, Y_SCREEN - 100, X_SCREEN, Y_SCREEN); // Cria o jogador na posição central da tela
    if (!p) {
        fprintf(stderr, "Falha ao criar o jogador\n");
        return -1;
    }

    // Defina a posição fixa do player na tela (exemplo: centro)
    int player_screen_x = 40;
    int player_screen_y = Y_SCREEN - 325;

    // Adicione essas variáveis antes do while(running)
    int run_frame = 0;
    int run_max_frames = 10;
    int run_frame_width = al_get_bitmap_width(player_run) / run_max_frames;
    int run_frame_height = al_get_bitmap_height(player_run);
    int frame_counter = 0;
    int frame_delay = 3; // Ajuste para controlar a velocidade da animação

    // Defina o fator de escala desejado
    float player_scale = 2.0f; // 2x maior

    ALLEGRO_EVENT event;
    al_start_timer(timer);
    while (running) {
        al_wait_for_event(queue, &event);

        if (redraw && al_is_event_queue_empty(queue)) {
            int w_sky = al_get_bitmap_width(sky);
            int w_far = al_get_bitmap_width(bg_far);
            int w_mid = al_get_bitmap_width(bg_mid);
            int w_near = al_get_bitmap_width(bg_near);
            int scaled_w_far = w_far * scale;
            int scaled_w_mid = w_mid * scale;
            int scaled_w_near = w_near * scale;

            // Camada 1: céu (não precisa repetir)
            al_draw_scaled_bitmap(sky, 0, 0, w_sky, al_get_bitmap_height(sky), 0, 0, X_SCREEN, Y_SCREEN, 0);

            // Camada 2: casas e árvores distantes (parallax mais lento)
            float far_scroll = fmod(scroll_x * 0.3, scaled_w_far);
            al_draw_scaled_bitmap(bg_far, 0, 0, w_far, al_get_bitmap_height(bg_far), -far_scroll, 0, scaled_w_far, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_far, 0, 0, w_far, al_get_bitmap_height(bg_far), -far_scroll + scaled_w_far, 0, scaled_w_far, Y_SCREEN, 0);

            // Camada 3: casas (parallax médio)
            float mid_scroll = fmod(scroll_x * 0.6, scaled_w_mid);
            al_draw_scaled_bitmap(bg_mid, 0, 0, w_mid, al_get_bitmap_height(bg_mid), -mid_scroll, 0, scaled_w_mid, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_mid, 0, 0, w_mid, al_get_bitmap_height(bg_mid), -mid_scroll + scaled_w_mid, 0, scaled_w_mid, Y_SCREEN, 0);

            // Camada 4: chão (parallax rápido)
            float near_scroll = fmod(scroll_x, scaled_w_near);
            al_draw_scaled_bitmap(bg_near, 0, 0, w_near, al_get_bitmap_height(bg_near), -near_scroll, 0, scaled_w_near, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_near, 0, 0, w_near, al_get_bitmap_height(bg_near), -near_scroll + scaled_w_near, 0, scaled_w_near, Y_SCREEN, 0);

            ALLEGRO_BITMAP *sprite = player_idle; // sprite padrão parado
            int inicio_x = 0; // Posição inicial do sprite na ANIMAÇÃO
            int flip = 0;

            // Correndo para direita
            if (p->control->left) {
                sprite = player_run;
                flip = ALLEGRO_FLIP_HORIZONTAL;
                last_dir = DIR_LEFT; // Atualiza última direção
                frame_counter++;
                if (frame_counter >= frame_delay) {
                    run_frame = (run_frame + 1) % run_max_frames;
                    frame_counter = 0;
                }
                inicio_x = run_frame * run_frame_width;
            }
            // Correndo para esquerda
            else if (p->control->right) {
                sprite = player_run;
                flip = 0;
                last_dir = DIR_RIGHT; // Atualiza última direção
                frame_counter++;
                if (frame_counter >= frame_delay) {
                    run_frame = (run_frame + 1) % run_max_frames;
                    frame_counter = 0;
                }
                inicio_x = run_frame * run_frame_width;
            // Parado virado para esquerda ou direita
            } else {
                if (last_dir == DIR_LEFT)
                    sprite = player_idle_left;
                else
                    sprite = player_idle;
            }

            // Desenha o player correndo
            if (sprite == player_run) {
                al_draw_scaled_bitmap(
                    sprite,
                    inicio_x, 0, run_frame_width, run_frame_height,
                    player_screen_x, player_screen_y,
                    run_frame_width * player_scale,
                    run_frame_height * player_scale,
                    flip
                );
            // Desenha o player parado para direita
            } else if (sprite == player_idle) {
                int idle_frame_width = al_get_bitmap_width(player_idle) / 5;
                int idle_frame_height = al_get_bitmap_height(player_idle);
                al_draw_scaled_bitmap(
                    player_idle,
                    0, 0, idle_frame_width, idle_frame_height,
                    player_screen_x, player_screen_y,
                    idle_frame_width * player_scale,
                    idle_frame_height * player_scale,
                    0
                );
            // Desenha o player parado para esquerda
            } else if (sprite == player_idle_left) {
                int idle_left_frame_width = al_get_bitmap_width(player_idle_left) / 5;
                int idle_left_frame_height = al_get_bitmap_height(player_idle_left);
                al_draw_scaled_bitmap(
                    player_idle_left,
                    0, 0, idle_left_frame_width, idle_left_frame_height,
                    player_screen_x, player_screen_y,
                    idle_left_frame_width * player_scale,
                    idle_left_frame_height * player_scale,
                    0
                );
            }

            al_flip_display();
            redraw = false;
        }

        if (event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;

            // Atualiza o scroll_x conforme o movimento do player
            if (p->control->right) {
                scroll_x += 7; // velocidade do player para direita
            }
            if (p->control->left) {
                scroll_x -= 7; // velocidade do player para esquerda
            }
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN || event.type == ALLEGRO_EVENT_KEY_UP) {
            if (event.keyboard.keycode == ALLEGRO_KEY_A) joystick_left(p->control);
            else if (event.keyboard.keycode == ALLEGRO_KEY_D) joystick_right(p->control);
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
	}

    al_destroy_bitmap(sky);
    al_destroy_bitmap(bg_far);
    al_destroy_bitmap(bg_mid);
    al_destroy_bitmap(bg_near);
    al_destroy_font(font);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(disp);
    al_destroy_bitmap(player_idle);
    al_destroy_bitmap(player_run);
    return 0;


}