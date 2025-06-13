//Compilação: gcc MeuJogo.c -o game $(pkg-config allegro-5 allegro_main-5 allegro_font-5 allegro_primitives-5 --libs --cflags)

#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define X_SCREEN 800
#define Y_SCREEN 600

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

    // Variáveis do background
    float scale = (float)Y_SCREEN / al_get_bitmap_height(bg_far);
    int bg_near_width = al_get_bitmap_width(bg_near);
    float scroll_x = 0; // Variável para controlar o deslocamento horizontal do fundo
    bool redraw = true; // Variável para controlar quando a tela deve ser redesenhada
    bool running = true; // Variável para controlar o loop principal do jogo
    bool key_right = false; // Variável para verificar se a tecla de seta direita está pressionada
    bool key_left = false; // Variável para verificar se a tecla de seta esquerda está pressionada

    // Variáveis do player
    float player_x = X_SCREEN / 2, player_y = Y_SCREEN - 325; // Poisição do jogador em relação ao chão
    float player_speed = 8;
    bool player_moving = false;

    // Carrega sprites do player
    ALLEGRO_BITMAP *player_idle = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle.png");
    ALLEGRO_BITMAP *player_idle_left = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle_Otherside.png");
    ALLEGRO_BITMAP *player_run = al_load_bitmap("sprites/gangsters/Gangsters_1/Run.png");
    if (!player_idle || !player_idle_left || !player_run) {
        fprintf(stderr, "Falha ao carregar sprites do player\n");
        return -1;
    }

    // Adicione essas variáveis para animação:
    int run_frame = 0;
    int run_max_frames = 10;
    int run_frame_width = al_get_bitmap_width(player_run) / run_max_frames;
    int run_frame_height = al_get_bitmap_height(player_run);

    int run_frame_delay = 2; // Ajuste para controlar a velocidade da animação
    int run_frame_count = 0;

    int player_direction = 1; // 1 = direita, -1 = esquerda

    int idle_frame_width = al_get_bitmap_width(player_idle) / 5;
    int idle_frame_height = al_get_bitmap_height(player_idle);

    int idle_frame_width_left = al_get_bitmap_width(player_idle_left) / 5;
    int idle_frame_height_left = al_get_bitmap_height(player_idle_left);

    ALLEGRO_EVENT event;
    al_start_timer(timer);
    while (running) {
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            player_moving = false;
            float player_x_min = X_SCREEN / 2; // Posição inicial do player

            if (key_right) {
                scroll_x -= 7;
                player_x += player_speed;
                player_moving = true;
                player_direction = 1;
            }
            if (key_left) {
                scroll_x += 7;
                player_x -= player_speed;
                player_moving = true;
                player_direction = -1;
            }

            // Limita o player_x ao mínimo permitido (posição inicial)
            if (player_x < player_x_min) player_x = player_x_min;
            //if (player_x > X_SCREEN - idle_frame_width * player_scale) player_x = X_SCREEN - idle_frame_width * player_scale;

            // Animação do player andando
            if (player_moving) {
                run_frame_count++;
                if (run_frame_count >= run_frame_delay) {
                    run_frame = (run_frame + 1) % run_max_frames;
                    run_frame_count = 0;
                }
            } else {
                run_frame = 0; // Reseta run quando parado
                run_frame_count = 0;
            }

            // Restringe o player dentro da tela
            if (player_x < 0) player_x = 0;
            if (player_x > X_SCREEN - al_get_bitmap_width(player_idle)) player_x = X_SCREEN - al_get_bitmap_width(player_idle);

            if (scroll_x <= -bg_near_width * scale) scroll_x += bg_near_width * scale;
            if (scroll_x >= bg_near_width * scale) scroll_x -= bg_near_width * scale;

            redraw = true;
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) key_right = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) key_left = true;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) key_right = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) key_left = false;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            int w_sky = al_get_bitmap_width(sky);
            int w_far = al_get_bitmap_width(bg_far);
            int w_mid = al_get_bitmap_width(bg_mid);
            int w_near = al_get_bitmap_width(bg_near);
            int scaled_w_far = w_far * scale;
            int scaled_w_mid = w_mid * scale;
            int scaled_w_near = w_near * scale;

            // Camada 1: céu
            al_draw_scaled_bitmap(sky, 0, 0, w_sky, al_get_bitmap_height(sky), 0, 0, X_SCREEN, Y_SCREEN, 0);

            // Camada 2: casas e árvores distantes
            al_draw_scaled_bitmap(bg_far, 0, 0, w_far, al_get_bitmap_height(bg_far), scroll_x, 0, scaled_w_far, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_far, 0, 0, w_far, al_get_bitmap_height(bg_far), scroll_x + scaled_w_far, 0, scaled_w_far, Y_SCREEN, 0);

            // Camada 3: casas
            al_draw_scaled_bitmap(bg_mid, 0, 0, w_mid, al_get_bitmap_height(bg_mid), scroll_x, 0, scaled_w_mid, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_mid, 0, 0, w_mid, al_get_bitmap_height(bg_mid), scroll_x + scaled_w_mid, 0, scaled_w_mid, Y_SCREEN, 0);

            // Camada 4: chão
            al_draw_scaled_bitmap(bg_near, 0, 0, w_near, al_get_bitmap_height(bg_near), scroll_x, 0, scaled_w_near, Y_SCREEN, 0);
            al_draw_scaled_bitmap(bg_near, 0, 0, w_near, al_get_bitmap_height(bg_near), scroll_x + scaled_w_near, 0, scaled_w_near, Y_SCREEN, 0);

            // Desenha o player
            float player_scale = 2.0;
            if (player_moving) {
                if (player_direction == 1) {
                    al_draw_scaled_bitmap(
                        player_run,
                        run_frame * run_frame_width, 0,
                        run_frame_width, run_frame_height,
                        player_x, player_y,
                        run_frame_width * player_scale, run_frame_height * player_scale,
                        0
                    );
                } else {
                    al_draw_scaled_bitmap(
                        player_run,
                        run_frame * run_frame_width, 0,
                        run_frame_width, run_frame_height,
                        player_x, player_y,
                        run_frame_width * player_scale, run_frame_height * player_scale,
                        ALLEGRO_FLIP_HORIZONTAL
                    );
                }
            } else {
                if (player_direction == 1) {
                    // Usa apenas o primeiro frame do idle
                    al_draw_scaled_bitmap( // Se está virando para a direita
                        player_idle,
                        0, 0,
                        idle_frame_width, idle_frame_height,
                        player_x, player_y,
                        idle_frame_width * player_scale, idle_frame_height * player_scale,
                        0
                    );
                } else {
                    al_draw_scaled_bitmap( // Se está virando para a esquerda (aqui usando o sprite de idle_left para ficar sem bug)
                        player_idle_left,
                        0, 0,
                        idle_frame_width_left, idle_frame_height_left,
                        player_x, player_y,
                        idle_frame_width_left * player_scale, idle_frame_height_left * player_scale,
                        0
                    );
                }
            }
            al_flip_display(); // Atualiza a tela para mostrar as mudanças
            redraw = false; // Reseta a variável de redesenho
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