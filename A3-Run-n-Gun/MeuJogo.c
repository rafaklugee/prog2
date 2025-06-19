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
#include "Enemy.h"
#include "Camera.h"
#include "Utils.h"
#include "SlimeBall.h"
#include "Boss.h"

#define X_SCREEN 800
#define Y_SCREEN 600
#define CAMERA_LEFT_MARGIN (X_SCREEN / 4)
#define CAMERA_RIGHT_MARGIN (X_SCREEN / 3)

// Defina o tamanho total do cenário (10x o fundo)
#define BG_REPEAT 4

// Variável para a posição do personagem no mundo
int player_world_x = 50;

// Lista encadeada de inimigos
enemy *enemies = NULL;

// Mostrar hitboxes
bool show_hitboxes = true; 

// Lista encadeada de slime balls
slime_ball *slime_balls = NULL;

// Variável para o boss final
boss *final_boss = NULL;

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
    slime_ball_load_sprite(); // Carrega o sprite da slime ball, que é usado pelos inimigos

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);	// Cria o relógio do jogo; isso indica quantas atualizações serão realizadas por segundo (30, neste caso)
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue(); // Cria a fila de eventos; todos os eventos (programação orientada a eventos) 
	ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN); // Cria uma janela para o programa, define a largura (x) e a altura (y) da tela em píxeis (800x600, neste caso)
    ALLEGRO_FONT* font = al_load_ttf_font("fonts/TheGodFather.ttf", 36, 0); // Carrega a fonte TTF que será usada no menu; o tamanho da fonte é 36 pixels
    ALLEGRO_FONT* big_font = al_load_ttf_font("fonts/TheGodFather.ttf", 64, 0);
    if (!font || !big_font) return -1;


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
                "sprites/gangsters/Gangsters_1/Jump.png",
                "sprites/gangsters/Gangsters_1/Shot.png",
                "sprites/gangsters/Gangsters_1/Squat_Shot.png",
                "sprites/gangsters/Gangsters_1/Hurt.png",
                "sprites/gangsters/Gangsters_1/Dead.png",
                "sprites/gangsters/Gangsters_1/Gangster_Health.png"

            )) {
            fprintf(stderr, "Falha ao carregar sprites do player\n");
            player1_destroy(p);
            return -1;
        }

    // Criando inimigos (3)
    enemies = enemy_create(900, Y_SCREEN - 325, 1.0, "sprites/zombies/Zombie_3/Walk.png");
    enemies->next = enemy_create(1300, Y_SCREEN - 325, 0.75, "sprites/zombies/Zombie_3/Walk.png");
    enemies->next->next = enemy_create(1800, Y_SCREEN - 325, 0.5, "sprites/zombies/Zombie_3/Walk.png");

    // Criando o boss, mas inativo
    final_boss = boss_create(
        BG_REPEAT * bg->scaled_w_near - 300, // posição final do mapa
        Y_SCREEN - 325, // ajuste vertical para o chão
        3.5f,
        "sprites/boss/3_Big_Bloated/Big_bloated_idle.png"
    );

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

        // Cálculo da câmera que irá acompanhar o personagem
        current_camera_x = camera_calculate(
            player_world_x,
            current_camera_x,
            p->control->left,
            p->control->right,
            CAMERA_LEFT_MARGIN,
            CAMERA_RIGHT_MARGIN,
            world_width,
            X_SCREEN
        );

        // Cálculo da posição do player na tela
        player_screen_x = player_world_x - current_camera_x;
        if (player_screen_x < 0) player_screen_x = 0;
        if (player_screen_x > X_SCREEN) player_screen_x = X_SCREEN;


        // Atualizando o scroll para o background
        scroll_x = current_camera_x;

        // Desenhando os elementos do mundo
        if (redraw && al_is_event_queue_empty(queue)) {
            background_draw(bg, scroll_x, X_SCREEN);
            player1_draw_bullets(p, current_camera_x);

            // Desenhe todos os inimigos e hitboxes
            enemy_draw_all(enemies, current_camera_x, show_hitboxes);

            player1_draw(p, player_screen_x, player_screen_y);

            // Use a função utilitária para desenhar a HUD
            draw_hud(p, font);

            // Desenhando a hitbox do jogador
            int hx, hy, hw, hh;
            player1_get_hitbox(p, player_screen_x, player_screen_y, &hx, &hy, &hw, &hh);
            if (show_hitboxes) {
                al_draw_rectangle(hx, hy, hx + hw, hy + hh, al_map_rgb(0, 0, 255), 2);
            }

            // Desenhar todas as slime balls
            slime_ball_draw(slime_balls, current_camera_x);

            // Desenhar o boss final se ele estiver ativo
            if (final_boss && final_boss->is_active) {
                boss_draw(final_boss, current_camera_x, show_hitboxes);
            }

            al_flip_display();
            redraw = false;
        }

        // Atualizando eventos com base no relógio (timer)
        if (event.type == ALLEGRO_EVENT_TIMER) {
            player1_update(p, &player_world_x, world_width, player_screen_y);
            player1_update_bullets(p, world_width);

            // Limite para não sair do início do mapa
            if (player_world_x < 0) player_world_x = 0;

            // Limite para não sair do final do mapa
            if (player_world_x > world_width - 150) player_world_x = world_width - 150;

            // Atualizando todos os inimigos
            enemy_update_all(enemies);

            // Atualizar todas as slime balls
            slime_ball_update(&slime_balls, world_width);

            // Colisão de balas com inimigos
            enemy_check_bullet_collisions(enemies, p->gun);

            // Removendo inimigos mortos cujo cooldown acabou
            enemy_remove_dead(&enemies);

            // Se acabaram os inimigos, ativa o boss final
            if (!enemies && final_boss && !final_boss->is_active) {
                final_boss->is_active = 1;
            }

            // Calcule a hitbox do player na tela
            int player_hitbox_x, player_hitbox_y, player_hitbox_w, player_hitbox_h;
            player1_get_hitbox(p, player_screen_x, player_screen_y, &player_hitbox_x, &player_hitbox_y, &player_hitbox_w, &player_hitbox_h);

            check_slime_collision_with_player(
                p, current_camera_x,
                player_hitbox_x, player_hitbox_y, player_hitbox_w, player_hitbox_h,
                player_screen_y
            );

            // Checa colisão dos tiros com o boss
            if (final_boss && final_boss->is_active && final_boss->health > 0) {
                bullet **curr = &(p->gun->shots);
                int bx = final_boss->x + final_boss->hitbox_offset_x;
                int by = final_boss->y + final_boss->hitbox_offset_y;
                int bw = final_boss->hitbox_w;
                int bh = final_boss->hitbox_h;
                while (*curr) {
                    bullet *b = *curr;
                    if (b->x > bx && b->x < bx + bw &&
                        b->y > by && b->y < by + bh) {
                        // Só aplica dano se não está hurt, mas SEMPRE remove o tiro!
                        if (!final_boss->is_hurt) {
                            final_boss->health--;
                            final_boss->is_hurt = 1;
                            final_boss->hurt_timer = 10;
                            if (final_boss->health <= 0 && !final_boss->is_dead) {
                                final_boss->is_dead = 1;
                                final_boss->death_frame = 0;
                                final_boss->death_frame_counter = 0;
                                final_boss->death_timer = 600; // 2 segundos após animação
                            }
                        }
                        *curr = (bullet*)b->next;
                        bullet_destroy(b);
                        break; // só um tiro por frame (tirar ou não esse break)
                    } else {
                        curr = (bullet**)&((*curr)->next);
                    }
                }
            }

            // Atualizar todas as slime balls
            slime_ball_update(&slime_balls, world_width);

            // Atualiza o boss se ele estiver ativo
            if (final_boss && (final_boss->is_active || final_boss->is_dead)) {
                boss_update(final_boss);
            }

            // Após boss_update(final_boss);
            static int boss_death_end_timer = 0;
            if (final_boss && final_boss->is_dead && final_boss->death_frame >= final_boss->death_max_frames - 1 && final_boss->death_timer <= 0) {
                boss_death_end_timer++;
                if (boss_death_end_timer > 15) {
                    int menu_choice = show_victory_menu(disp, font, big_font, queue, bg);
                    if (menu_choice == 1) {
                        running = false; // Sair do jogo
                    } else {
                        // Volta ao menu principal
                        int menu_result = show_menu(disp, font, queue, bg);
                        if (menu_result == 1) {
                            running = false;
                        } else {
                            boss_death_end_timer = 0;
                            reset_game_state(&p, &enemies, &player_world_x, &current_camera_x, player_screen_y, bg, BG_REPEAT);
                        }
                    }
                }
            } else if (!(final_boss && final_boss->is_dead)) {
                boss_death_end_timer = 0;
            }

            redraw = true;
        }

        // Eventos de teclado (se aperta ou solta alguma tecla)
        if (event.type == ALLEGRO_EVENT_KEY_DOWN || event.type == ALLEGRO_EVENT_KEY_UP) {
            player1_handle_event(p, &event, player_world_x); // Atualiza os eventos de teclado do player1
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) // Encerra o jogo no "esc"
                running = false;
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_H) {
                show_hitboxes = !show_hitboxes;
            }
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) // Se fechar o display sai do jogo
            break;

            // Se o personagem morre e suas sprites de "dead" acabarem, mostra o menu de game over
        if (p->is_dead && p->dead_frame >= p->dead_max_frames - 1 && p->dead_menu_cooldown <= 0) {
            // Mostra o menu de GAME OVER
            int menu_choice = show_gameover_menu(disp, font, big_font, queue, bg);
            if (menu_choice == 1) {
                running = false; // Sair do jogo
            } else {
                reset_game_state(&p, &enemies, &player_world_x, &current_camera_x, player_screen_y, bg, BG_REPEAT); // Jogar novamente
            }
        }
    }

    // Libera memória de tudo
    background_destroy(bg);
    player1_destroy(p);
    al_destroy_font(font);
    slime_ball_unload_sprite();
    enemy_destroy_all(enemies);
    boss_destroy(final_boss);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_display(disp);
    return 0;


}