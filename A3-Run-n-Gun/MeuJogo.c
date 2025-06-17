#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>

#include "Player1.h"
#include "Background.h"
#include "Menu.h"

#define X_SCREEN 800
#define Y_SCREEN 600
#define CAMERA_LEFT_MARGIN (X_SCREEN / 4)
#define CAMERA_RIGHT_MARGIN (X_SCREEN / 2)

// Defina o tamanho total do cenário (10x o fundo)
#define BG_REPEAT 10

// Adicione uma variável para a posição do personagem no mundo
int player_world_x = 50;

#define DIR_RIGHT 1 // Direção para a direita é 1
#define DIR_LEFT  -1 // Direção para a esquerda é -1
int last_dir = DIR_RIGHT; // Armazena a última direção do player (começa na direita)

int main(){
	
	al_init();	//Faz a preparação de requisitos da biblioteca Allegro
    al_init_ttf_addon(); // Habilita o addon que permite carregar e usar fontes TTF
	al_init_primitives_addon();	//Faz a inicialização dos addons das imagens básicas
    al_init_image_addon(); // Habilita o addon de imagens, que permite carregar imagens em formatos como PNG, JPEG, etc.
    al_init_font_addon(); // Habilita o addon de fontes, que permite carregar e usar fontes TTF e bitmap
	al_install_keyboard(); //Habilita a entrada via teclado (eventos de teclado), no programa

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);	// Cria o relógio do jogo; isso indica quantas atualizações serão realizadas por segundo (30, neste caso)
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue(); // Cria a fila de eventos; todos os eventos (programação orientada a eventos) 
	ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN); // Cria uma janela para o programa, define a largura (x) e a altura (y) da tela em píxeis (800x600, neste caso)
    ALLEGRO_FONT* font = al_load_ttf_font("fonts/TheGodFather.ttf", 36, 0); // Carrega a fonte TTF que será usada no menu; o tamanho da fonte é 36 pixels
    if (!font) return -1;


	al_register_event_source(queue, al_get_keyboard_event_source()); // Indica que eventos de teclado serão inseridos na nossa fila de eventos
	al_register_event_source(queue, al_get_display_event_source(disp)); // Indica que eventos de tela serão inseridos na nossa fila de eventos
	al_register_event_source(queue, al_get_timer_event_source(timer)); // Indica que eventos de relógio serão inseridos na nossa fila de eventos

    // Criando o background
    Background *bg = background_create(
        "img/PNG/Postapocalypce2/Bright/sky.png",
        "img/PNG/Postapocalypce2/Bright/houses&trees_bg.png",
        "img/PNG/Postapocalypce2/Bright/houses.png",
        "img/PNG/Postapocalypce2/Bright/road.png",
        Y_SCREEN, BG_REPEAT
    );
    if (!bg) {
        fprintf(stderr, "Falha ao criar o background\n");
        return -1;
    }

    // Mostrando o menu com o background
    int menu_choice = show_menu(disp, font, queue, bg);
    if (menu_choice == 1) {
        // Sair selecionado
        al_destroy_font(font);
        al_destroy_event_queue(queue);
        al_destroy_timer(timer);
        al_destroy_display(disp);
        return 0;
    }

    // Criando o player
    player1 *p = create_player1(Y_SCREEN - 325, X_SCREEN, Y_SCREEN);
        if (!p) {
            fprintf(stderr, "Falha ao criar o jogador\n");
            return -1;
        }
        if (!player1_load_sprites(
                p,
                "sprites/gangsters/Gangsters_1/Idle.png",
                "sprites/gangsters/Gangsters_1/Idle_Otherside.png",
                "sprites/gangsters/Gangsters_1/Run.png",
                "sprites/gangsters/Gangsters_1/Squat.png",
                "sprites/gangsters/Gangsters_1/Jump.png"
            )) {
            fprintf(stderr, "Falha ao carregar sprites do player\n");
            player1_destroy(p);
            return -1;
        }

    bool redraw = true; // Variável para controlar quando a tela deve ser redesenhada
    bool running = true; // Variável para controlar o loop principal do jogo

    // Defina a posição fixa do player na tela
    int player_screen_y = Y_SCREEN - 325;

    // Variável que controla a câmera
    float scroll_x = 0;

    int current_camera_x = 0;

    // Variável que controla o tamanho do mundo
    int world_width = BG_REPEAT * bg->scaled_w_near;

    ALLEGRO_EVENT event;
    al_start_timer(timer);
    while (running) {
        al_wait_for_event(queue, &event);
        int player_screen_x;

        int camera_min_x = 0;
        int camera_max_x = world_width - X_SCREEN;
        if (camera_max_x < camera_min_x) camera_max_x = camera_min_x;

        // --- CÁLCULO DA CÂMERA ---

        // Posição do jogador na tela APÓS ter se movido (com base na câmera atual)
        int player_screen_x_current_frame = player_world_x - current_camera_x;

        // 1. Lógica para movimento para a DIREITA
        if (p->control->right) {
            // Se o player está se movendo para a direita, a câmera só se move
            // se o player atingir a margem direita da tela.
            if (player_screen_x_current_frame > CAMERA_RIGHT_MARGIN) {
                // A câmera deve mover-se junto com o player para manter a distância da margem.
                current_camera_x += (player_screen_x_current_frame - CAMERA_RIGHT_MARGIN);
            }
        }
        // 2. Lógica para movimento para a ESQUERDA
        else if (p->control->left) {
            // Se o player está se movendo para a esquerda, a câmera só se move
            // se o player atingir a margem esquerda da tela.
            if (player_screen_x_current_frame < CAMERA_LEFT_MARGIN) {
                // A câmera deve mover-se junto com o player para manter a distância da margem.
                current_camera_x -= (CAMERA_LEFT_MARGIN - player_screen_x_current_frame);
            }
        }
        // Se o player está parado, a câmera não se move (current_camera_x mantém seu valor)
        // Não precisamos de uma 'else' específica aqui, pois as operações acima
        // só modificam current_camera_x se as condições de movimento forem atendidas.

        // Garante que current_camera_x esteja sempre dentro dos limites do mundo
        if (current_camera_x < camera_min_x) {
            current_camera_x = camera_min_x;
        }
        if (current_camera_x > camera_max_x) {
            current_camera_x = camera_max_x;
        }

        // --- CÁLCULO DA POSIÇÃO DO PLAYER NA TELA ---
        // O player é sempre desenhado na sua posição relativa à câmera atual.
        player_screen_x = player_world_x - current_camera_x;

        // Limita player_screen_x para garantir que ele esteja sempre dentro da tela
        // Isso é mais para evitar que o sprite saia da tela, mas a lógica da câmera já cuida disso.
        if (player_screen_x < 0) player_screen_x = 0;
        if (player_screen_x > X_SCREEN) player_screen_x = X_SCREEN;


        // Atualizando o scroll para o background
        scroll_x = current_camera_x;

        // Se há eventos pendentes na fila e redesenharemos o mapa
        if (redraw && al_is_event_queue_empty(queue)) {
            background_draw(bg, scroll_x, X_SCREEN);
            player1_draw(p, player_screen_x, player_screen_y);

            al_flip_display();
            redraw = false;
        }
        if (event.type == ALLEGRO_EVENT_TIMER) { // Atualiza os eventos do jogo
            player1_update(p, &player_world_x, world_width, player_screen_y);
            redraw = true;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN || event.type == ALLEGRO_EVENT_KEY_UP) {
            player1_handle_event(p, &event); // Atualiza os eventos de teclado do player1
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                running = false; // Encerra o jogo no "esc"
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;
    }

    background_destroy(bg);
    player1_destroy(p);
    al_destroy_font(font);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(disp);
    return 0;


}