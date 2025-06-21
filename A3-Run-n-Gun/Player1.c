#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "Player1.h"

#define DIR_RIGHT 1
#define DIR_LEFT -1

extern float global_volume;

// Inicialização do Player 1 com atributos iniciais
player1* create_player1(unsigned short initial_y, unsigned short max_x, unsigned short max_y) {
    player1 *p = (player1 *)malloc(sizeof(player1));
    if (!p) return NULL;

    p->control = joystick_create();
    p->gun = pistol_create();
    p->player_scale = 2.0f;
    p->gravity = 1.2f;
    p->jump_strength = -18.0f;
    p->is_jumping = 0;
    p->player_y = initial_y;
    p->last_dir = DIR_RIGHT;
    p->frame_delay = 3;
    p->frame_counter = 0;
    p->is_squatting = 0;
    p->squat_anim_done = 0;
    p->is_squatting_out = 0;
    p->is_squat_shooting = 0;
    p->is_shooting_pressed = 0;
    p->health = 3;
    return p;
}

// Carrega e inicializa todas as sprites necessárias para o Player1
int player1_load_sprites(player1 *p) {
    p->idle = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle.png");
    p->idle_left = al_load_bitmap("sprites/gangsters/Gangsters_1/Idle_Otherside.png");
    p->run = al_load_bitmap("sprites/gangsters/Gangsters_1/Run.png");
    p->squat = al_load_bitmap("sprites/gangsters/Gangsters_1/Squat.png");
    p->jump = al_load_bitmap("sprites/gangsters/Gangsters_1/Jump.png");
    p->shot = al_load_bitmap("sprites/gangsters/Gangsters_1/Shot.png");
    p->squat_shot = al_load_bitmap("sprites/gangsters/Gangsters_1/Squat_Shot.png");
    p->hurt = al_load_bitmap("sprites/gangsters/Gangsters_1/Hurt.png");
    p->dead = al_load_bitmap("sprites/gangsters/Gangsters_1/Dead.png");
    p->hud_health = al_load_bitmap("sprites/gangsters/Gangsters_1/Gangster_Health.png");
    p->recharge = al_load_bitmap("sprites/gangsters/Gangsters_1/Recharge.png");
    p->hud_ammo = al_load_bitmap("sprites/gangsters/Gangsters_1/Hud_Ammo.png");

    // Carrega som
    p->gun_shot_sound = al_load_sample("sounds/Gun_Shot.wav");
    p->gun_reload_sound = al_load_sample("sounds/Gun_Reload.wav");



    if (!p->idle || !p->idle_left || !p->run || !p->squat || !p->jump || !p->shot 
        || !p->hurt || !p->hud_health || !p->recharge || !p->hud_ammo || !p->gun_shot_sound || !p->gun_reload_sound)
        return 0;

    // Frames e dimensões
    p->run_max_frames = 10;
    p->run_frame = 0;
    p->run_frame_width = al_get_bitmap_width(p->run) / p->run_max_frames;
    p->run_frame_height = al_get_bitmap_height(p->run);

    p->squat_max_frames = 4;
    p->squat_frame = 0;
    p->squat_frame_width = al_get_bitmap_width(p->squat) / p->squat_max_frames;
    p->squat_frame_height = al_get_bitmap_height(p->squat);

    p->jump_max_frames = 10;
    p->jump_frame = 0;
    p->jump_frame_width = al_get_bitmap_width(p->jump) / p->jump_max_frames;
    p->jump_frame_height = al_get_bitmap_height(p->jump);

    p->idle_max_frames = 6;
    p->idle_frame = 0;
    p->idle_frame_width = al_get_bitmap_width(p->idle) / p->idle_max_frames;
    p->idle_frame_height = al_get_bitmap_height(p->idle);
    p->idle_frame_delay = 7;

    p->shot_max_frames = 4;
    p->shot_frame = 0;
    p->shot_frame_width = al_get_bitmap_width(p->shot) / p->shot_max_frames;
    p->shot_frame_height = al_get_bitmap_height(p->shot);
    p->is_shooting = 0;
    p->shooting_frames = 0;

    p->hurt_max_frames = 5;
    p->hurt_frame = 0;
    p->hurt_frame_width = al_get_bitmap_width(p->hurt) / p->hurt_max_frames;
    p->hurt_frame_height = al_get_bitmap_height(p->hurt);
    p->hurt_frame_delay = 3;
    p->hurt_frame_counter = 0;
    p->is_hurt = 0;

    p->dead_max_frames = 5;
    p->dead_frame = 0;
    p->dead_frame_width = al_get_bitmap_width(p->dead) / p->dead_max_frames;
    p->dead_frame_height = al_get_bitmap_height(p->dead);
    p->dead_frame_delay = 7;
    p->dead_frame_counter = 0;
    p->is_dead = 0;

    p->reload_max_frames = 17;
    p->reload_frame = 0;
    p->reload_frame_width = al_get_bitmap_width(p->recharge) / p->reload_max_frames;
    p->reload_frame_height = al_get_bitmap_height(p->recharge);
    p->reload_frame_delay = 3;
    p->reload_frame_counter = 0;
    p->is_reloading = 0;
    p->reload_cooldown = 0;
    p->ammo = 30;
    p->max_ammo = 90;

    return 1;
}

// Lógicas de update (flags, estados, etc)
void player1_update(player1 *p, int *player_world_x, int world_width, int player_screen_y) {
    // Se o player morreu, executa seus frames de morte e volta ao menu principal
    if (p->is_dead) {
        // Interrompe o reload se morrer
        p->is_reloading = 0;
        p->reload_frame = 0;
        p->reload_frame_counter = 0;
        p->dead_frame_counter++;
        if (p->dead_frame_counter >= p->dead_frame_delay) {
            p->dead_frame++;
            p->dead_frame_counter = 0;
        }
        if (p->dead_frame >= p->dead_max_frames) {
            p->dead_frame = p->dead_max_frames - 1;
        }
        if (p->dead_menu_cooldown > 0) {
            p->dead_menu_cooldown--;
        }
        // Durante a morte, não processa o resto do update (bloqueia)
        return;
    }

    // Se o player tomou hit (hurt), bloqueia seus ataques
    if (p->is_hurt) {
        // Interrompe o reload ao tomar dano
        p->is_reloading = 0;
        p->reload_frame = 0;
        p->reload_frame_counter = 0;
        p->hurt_frame_counter++;
        if (p->hurt_frame_counter >= p->hurt_frame_delay) {
            p->hurt_frame++;
            p->hurt_frame_counter = 0;
        }
        if (p->hurt_frame >= p->hurt_max_frames) {
            p->is_hurt = 0;
            p->hurt_frame = 0;
        }
        // Durante o dano, não processa o resto do update (bloqueia)
        return;
    }

    // Se está recarregando, só anima o reload
    if (p->is_reloading) {
        p->reload_frame_counter++;
        if (p->reload_frame_counter >= p->reload_frame_delay) {
            p->reload_frame++;
            p->reload_frame_counter = 0;
        }
        if (p->reload_frame >= p->reload_max_frames) {
            p->is_reloading = 0;
            p->reload_frame = 0;
            p->reload_cooldown = 0;

            int needed = 30 - p->ammo;
            if (p->max_ammo >= needed) {
                p->max_ammo -= needed;
                p->ammo = 30;
            } else {
                p->ammo += p->max_ammo;
                p->max_ammo = 0;
                if (p->ammo > 30) p->ammo = 30;
            }
        }
        return; // Bloqueia tudo enquanto recarrega
    }

    // Se o player não está agachando e não está indo para direita ou esquerda
    if (!p->is_squatting && !(p->control->left && p->control->right)) {
        // Anda para esquerda
        if (p->control->left && *player_world_x - PLAYER1_STEP >= 0) {
            *player_world_x -= PLAYER1_STEP;
        }
        // Anda para direita
        if (p->control->right) {
            *player_world_x += PLAYER1_STEP;
        }
    }

    // Atualiza salto
    if (p->is_jumping) {
        p->jump_velocity += p->gravity;
        p->player_y += p->jump_velocity;
        // Avanço de frame do pulo
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->jump_frame = (p->jump_frame + 1) % p->jump_max_frames;
            p->frame_counter = 0;
        }
        // Garante que o jogador fique no chão
        if (p->player_y >= player_screen_y) {
            p->player_y = player_screen_y;
            p->is_jumping = 0;
            p->jump_frame = 0;
            p->jump_velocity = 0;
        }
    } else {
        // Se ele não estiver pulando, vão ficar no chão
        p->player_y = player_screen_y;
    }

    // Avanço de frame para agachar
    if (p->control->down) {
        if (!p->squat_anim_done) {
            p->frame_counter++;
            if (p->frame_counter >= p->frame_delay) {
                p->squat_frame++;
                p->frame_counter = 0;
            }
            if (p->squat_frame >= p->squat_max_frames - 1) {
                p->squat_frame = p->squat_max_frames - 1;
                p->squat_anim_done = 1;
                p->is_squatting = 1;
            }
        } else {
            p->is_squatting = 1;
        }
    } else {
        p->squat_frame = 0;
        p->squat_anim_done = 0;
        p->is_squatting = 0;
    }

    // Avanço de frame para correr
    if (p->control->left && !p->control->right) {
        p->last_dir = DIR_LEFT;
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->run_frame = (p->run_frame + 1) % p->run_max_frames;
            p->frame_counter = 0;
        }
    } else if (p->control->right && !p->control->left) {
        p->last_dir = DIR_RIGHT;
        p->frame_counter++;
        if (p->frame_counter >= p->frame_delay) {
            p->run_frame = (p->run_frame + 1) % p->run_max_frames;
            p->frame_counter = 0;
        }
    } else if (!p->is_jumping && !p->control->down) {
        // Idle
        p->frame_counter++;
        if (p->frame_counter >= p->idle_frame_delay) {
            p->idle_frame = (p->idle_frame + 1) % p->idle_max_frames;
            p->frame_counter = 0;
        }
    }

    // Se a arma não estiver recarregando...
    if (p->gun && p->gun->timer > 0)
        p->gun->timer--;

    // Atualiza o estado de tiro do player
    if (p->is_shooting) {
        p->shooting_frames--;
        if (p->shooting_frames > 0) {
            // Avança para o próximo frame da animação de tiro
            p->shot_frame = (p->shot_frame + 1) % p->shot_max_frames;
        }
        // Se acabou o tempo do tiro, reseta todos os estados relacionados ao tiro
        if (p->shooting_frames <= 0) {
            p->is_shooting = 0;
            p->shooting_frames = 0;
            p->shot_frame = 0;
            p->is_squat_shooting = 0;
        }
    }

    // Atualiza enquanto o player atira
    if (p->is_shooting_pressed && p->gun->timer == 0 && !p->is_reloading) {
        // Bloqueia tiro se estiver agachando ou correndo
        if (p->control->left || p->control->right || (p->control->down && !p->squat_anim_done)) {
            return;
        }
        // Bloqueia tiro se não tiver munição
        if (p->ammo <= 0) {
            return;
        }
        unsigned char traj;
        int bullet_offset_x, bullet_offset_y;

        // Se eu estou pressionando up (Q + SPACE), atira para cima
        if (p->control->up) {
            traj = BULLET_TRAJ_UP;
            if (p->control->down) {
                // Ajuste o offset_x para tiro agachado para cima
                if (p->last_dir == DIR_LEFT)
                    bullet_offset_x = 80; 
                else
                    bullet_offset_x = 175; 
                bullet_offset_y = 220;    
            } else {
                if (p->last_dir == DIR_LEFT)
                    bullet_offset_x = 75;
                else
                    bullet_offset_x = 175;
                bullet_offset_y = 180;
            }
        // Senão, a bala vai para direita ou esquerda
        } else {
            if (p->last_dir == DIR_LEFT)
                traj = BULLET_TRAJ_LEFT;
            else
                traj = BULLET_TRAJ_RIGHT;

            // Controla o eixo X de onde sai a bala
            if (p->last_dir == DIR_LEFT)
                bullet_offset_x = 85;
            else
                bullet_offset_x = 175;

            // Lógica para onde a bala vai se o personagem está pulando
            if (p->is_jumping) {
                static const int jump_shot_offset_y[10] = {195, 185, 175, 165, 155, 165, 175, 185, 195, 200};
                int frame = p->jump_frame;
                if (frame < 0) frame = 0;
                if (frame >= 10) frame = 9;
                bullet_offset_y = jump_shot_offset_y[frame];
            // Lógica para onde a bala vai se o personagem está agachado
            } else if (p->control->down) {
                bullet_offset_y = 210;
            // Se não está pulando nem agachado, esse é o eixo Y correto
            } else {
                bullet_offset_y = 175;
            }
        }

        // Cria uma nova bala com bullet e pistol, com as dimensões do player e offsets passados
        bullet *new_bullet = pistol_shot(
            *player_world_x + bullet_offset_x,
            p->player_y + bullet_offset_y,
            traj,
            p->gun
        );
        // Se uma nova bala foi criada, adiciona ela a lista de balas (tiros)
        if (new_bullet) {
            new_bullet->next = (struct bullet*)p->gun->shots;
            p->gun->shots = new_bullet;

            // Toca o som do tiro
            if (p->gun_shot_sound) {
                al_play_sample(p->gun_shot_sound, global_volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }

            // Tempo de cooldown para o tiro
            if (p->is_jumping)
                p->gun->timer = PISTOL_COOLDOWN * 2;
            else
                p->gun->timer = PISTOL_COOLDOWN; 
            
            // Controla as flags de estar atirando
            p->is_shooting = 1;
            p->shooting_frames = 4;
            p->shot_frame = 0;
            // Controla a flag de estar atirando agachado
            if (p->control->down) {
                p->is_squat_shooting = 1;
            } else {
                p->is_squat_shooting = 0;
            }
            // Gasta uma bala
            p->ammo--;
        }
    }
}

// Lógicas de desenho
void player1_draw(player1 *p, int player_screen_x, int player_screen_y) {
    // Sprite de recarregamento (recharge)
    if (p->is_reloading) {
        int frame_x = p->reload_frame * p->reload_frame_width;
        int flip = (p->last_dir == DIR_LEFT) ? ALLEGRO_FLIP_HORIZONTAL : 0;
        al_draw_scaled_bitmap(
            p->recharge,
            frame_x, 0, p->reload_frame_width, p->reload_frame_height,
            player_screen_x, p->player_y,
            p->reload_frame_width * p->player_scale,
            p->reload_frame_height * p->player_scale,
            flip
        );
        return;
    }

    // Sprite de morte (dead)
    if (p->is_dead) {
        int frame_x = p->dead_frame * p->dead_frame_width;
        int dead_offset_x = + 40;
        al_draw_scaled_bitmap(
            p->dead,
            frame_x, 0, p->dead_frame_width, p->dead_frame_height,
            player_screen_x + dead_offset_x, p->player_y,
            p->dead_frame_width * p->player_scale,
            p->dead_frame_height * p->player_scale,
            0
        );
        return;
    }

    // Sprite de dano (hurt)
    if (p->is_hurt) {
        int frame_x = p->hurt_frame * p->hurt_frame_width;
        int flip;
        // Animação espelhada se ele estiver virado para o outro lado
        if (p->last_dir == DIR_LEFT)
            flip = ALLEGRO_FLIP_HORIZONTAL;
        else
            flip = 0;
        al_draw_scaled_bitmap(
            p->hurt,
            frame_x, 0, p->hurt_frame_width, p->hurt_frame_height,
            player_screen_x, p->player_y,
            p->hurt_frame_width * p->player_scale,
            p->hurt_frame_height * p->player_scale,
            flip
        );
        return;
    }

    ALLEGRO_BITMAP *sprite = p->idle;
    int inicio_x = 0;
    int flip = 0;
    int idle_frame_height = al_get_bitmap_height(p->idle);
    int squat_y_offset = (idle_frame_height - p->squat_frame_height) * p->player_scale;

    // Sprite para quando o player atira (shot)
    if (p->is_shooting) {
        if (p->is_squat_shooting) {
            // Sprite para quando o player atira agachado (squat shot)
            ALLEGRO_BITMAP *sprite = p->squat_shot;
            int squat_shot_max_frames = 2;
            int squat_shot_frame_width = al_get_bitmap_width(sprite) / squat_shot_max_frames;
            int squat_shot_frame_height = al_get_bitmap_height(sprite);
            int shot_frame_x = p->shot_frame % squat_shot_max_frames * squat_shot_frame_width;
            int shot_offset_x = 20;
            int flip = 0;
            if (p->last_dir == DIR_LEFT) {
                flip = ALLEGRO_FLIP_HORIZONTAL;
                shot_offset_x = -20;
            }
            al_draw_scaled_bitmap(
                sprite,
                shot_frame_x, 0, squat_shot_frame_width, squat_shot_frame_height,
                player_screen_x + shot_offset_x, p->player_y,
                squat_shot_frame_width * p->player_scale,
                squat_shot_frame_height * p->player_scale,
                flip
            );
        // Sprite para tiro normal (em pé)
        } else {
            ALLEGRO_BITMAP *sprite = p->shot;
            int shot_frame_x = p->shot_frame * p->shot_frame_width;
            int shot_offset_x = 16;
            int flip = 0;
            if (p->last_dir == DIR_LEFT) {
                flip = ALLEGRO_FLIP_HORIZONTAL;
                shot_offset_x = -16;
            }
            al_draw_scaled_bitmap(
                sprite,
                shot_frame_x, 0, p->shot_frame_width, p->shot_frame_height,
                player_screen_x + shot_offset_x, p->player_y,
                p->shot_frame_width * p->player_scale,
                p->shot_frame_height * p->player_scale,
                flip
            );
        }
        return;
    }

    // Sprite para pulo (jump)
    if (p->is_jumping) {
        sprite = p->jump;
        if (p->last_dir == DIR_LEFT) {
            flip = ALLEGRO_FLIP_HORIZONTAL;
        } else {
            flip = 0;
        }
        inicio_x = p->jump_frame * p->jump_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->jump_frame_width, p->jump_frame_height,
            player_screen_x, p->player_y,
            p->jump_frame_width * p->player_scale,
            p->jump_frame_height * p->player_scale,
            flip
        );
        return;
    }

    // Sprite para agachar (squat)
    else if (p->control->down) {
        if (!p->squat_anim_done) {
            sprite = p->squat;
            if (p->last_dir == DIR_LEFT)
                flip = ALLEGRO_FLIP_HORIZONTAL;
            else
                flip = 0;
            inicio_x = p->squat_frame * p->squat_frame_width;
        } else {
            sprite = p->squat;
            if (p->last_dir == DIR_LEFT)
                flip = ALLEGRO_FLIP_HORIZONTAL;
            else
                flip = 0;
            inicio_x = (p->squat_max_frames - 1) * p->squat_frame_width;
        }
        float squat_scale = p->player_scale;
        float x_offset = 0;
        if (flip == ALLEGRO_FLIP_HORIZONTAL) {
            x_offset = -40;
        }
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->squat_frame_width, p->squat_frame_height,
            player_screen_x + 20 + x_offset, p->player_y + squat_y_offset,
            p->squat_frame_width * squat_scale,
            p->squat_frame_height * squat_scale,
            flip
        );
        return;
    }

    // Correndo para esquerda
    else if (p->control->left && !p->control->right) {
        sprite = p->run;
        flip = ALLEGRO_FLIP_HORIZONTAL;
        inicio_x = p->run_frame * p->run_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->run_frame_width, p->run_frame_height,
            player_screen_x, p->player_y,
            p->run_frame_width * p->player_scale,
            p->run_frame_height * p->player_scale,
            flip
        );
        return;
    }

    // Correndo para direita
    else if (p->control->right && !p->control->left) {
        sprite = p->run;
        flip = 0;
        inicio_x = p->run_frame * p->run_frame_width;
        al_draw_scaled_bitmap(
            sprite,
            inicio_x, 0, p->run_frame_width, p->run_frame_height,
            player_screen_x, p->player_y,
            p->run_frame_width * p->player_scale,
            p->run_frame_height * p->player_scale,
            flip
        );
        return;
    }

    // Idle
    if (p->last_dir == DIR_LEFT) {
        sprite = p->idle_left;
    } else {
        sprite = p->idle;
    }
    int idle_frame_x = p->idle_frame * p->idle_frame_width;
    al_draw_scaled_bitmap(
        sprite,
        idle_frame_x, 0, p->idle_frame_width, p->idle_frame_height,
        player_screen_x, p->player_y,
        p->idle_frame_width * p->player_scale,
        p->idle_frame_height * p->player_scale,
        0
    );

    if (p->is_shooting) {
        ALLEGRO_BITMAP *sprite = p->shot;
        int shot_frame_x = p->shot_frame * p->shot_frame_width;
        int shot_offset_x;
        if (p->last_dir == DIR_LEFT) {
            shot_offset_x = -32;
        } else {
            shot_offset_x = 16;
        }
        al_draw_scaled_bitmap(
            sprite,
            shot_frame_x, 0, p->shot_frame_width, p->shot_frame_height,
            player_screen_x + shot_offset_x, p->player_y,
            p->shot_frame_width * p->player_scale,
            p->shot_frame_height * p->player_scale,
            0
        );
        return;
    }
}

// Eventos de teclado do Player 1
void player1_handle_event(player1 *p, ALLEGRO_EVENT *event, int player_world_x) {
    // Bloqueia qualquer atualização de teclado se o player morreu
    if (p->is_dead)
        return;

    // Se uma tecla for pressionada, faça isso
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        // Esquerda
        if (event->keyboard.keycode == ALLEGRO_KEY_A) {
            if (p->is_squatting) { p->control->down = 0; p->is_squatting = 0; p->squat_anim_done = 0; p->squat_frame = 0; }
            p->control->left = 1;
        }
        // Direita
        else if (event->keyboard.keycode == ALLEGRO_KEY_D) {
            if (p->is_squatting) { p->control->down = 0; p->is_squatting = 0; p->squat_anim_done = 0; p->squat_frame = 0; }
            p->control->right = 1;
        }
        // Salto
        else if (event->keyboard.keycode == ALLEGRO_KEY_W) {
            if (!p->is_jumping && !p->is_squatting) {
                p->is_jumping = 1;
                p->jump_velocity = p->jump_strength;
            }
        }
        // Agachar
        else if (event->keyboard.keycode == ALLEGRO_KEY_S) {
            // Só permite agachar se não estiver pulando
            if (!p->is_jumping)
                p->control->down = 1;
        }
        // Tiro
        else if (event->keyboard.keycode == ALLEGRO_KEY_SPACE) {
            p->is_shooting_pressed = 1;
        }
        // Tiro para cima
        else if (event->keyboard.keycode == ALLEGRO_KEY_Q) {
            p->control->up = 1;
        }
        // Recarregar
        else if (event->keyboard.keycode == ALLEGRO_KEY_R) {
            if (!p->is_reloading && p->ammo < 30 && p->max_ammo > 0 && !p->is_jumping && !p->is_hurt && !p->is_dead) {
                p->is_reloading = 1;
                p->reload_frame = 0;
                p->reload_frame_counter = 0;
                // Toca o som de recarregar
                if (p->gun_reload_sound) {
                    al_play_sample(p->gun_reload_sound, global_volume * 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                }
            }
        }
    }
    // Se desapertou a tecla, reseta tudo
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        // Tiro
        if (event->keyboard.keycode == ALLEGRO_KEY_SPACE) {
            p->is_shooting_pressed = 0;
        }
        // Esquerda
        else if (event->keyboard.keycode == ALLEGRO_KEY_A)
            p->control->left = 0;
        // Direita
        else if (event->keyboard.keycode == ALLEGRO_KEY_D)
            p->control->right = 0;
        // Agachar
        else if (event->keyboard.keycode == ALLEGRO_KEY_S) {
            p->control->down = 0;
            if (p->is_squatting) {
                p->is_squatting = 0;
                p->squat_anim_done = 0;
                p->squat_frame = 0;
                p->is_squatting_out = 1;
                p->frame_counter = 0;
            }
        }
        // Tiro para cima
        else if (event->keyboard.keycode == ALLEGRO_KEY_Q) {
            p->control->up = 0;
        }

        // Salto não precisa, pois executa uma vez e retorna
    }
}

void player1_update_bullets(player1 *p, int world_width) {
    // Referencia o ponteiro para a lista de balas (shots) da arma do player
    bullet **curr = &(p->gun->shots);
    while (*curr) {
        // Determina a direção da bala atual
        if ((*curr)->trajectory == BULLET_TRAJ_LEFT)
            (*curr)->x -= BULLET_MOVE;
        else if ((*curr)->trajectory == BULLET_TRAJ_RIGHT)
            (*curr)->x += BULLET_MOVE;
        else if ((*curr)->trajectory == BULLET_TRAJ_UP)
            (*curr)->y -= BULLET_MOVE;

        // Atualize a distância percorrida
        (*curr)->distance_traveled += BULLET_MOVE;

        // Remove a bala se sair da tela do mundo ou se andou 600 pixels
        if ((*curr)->x < 0 || (*curr)->x > world_width || (*curr)->y < 0 || (*curr)->distance_traveled >= 600) {
            bullet *to_remove = *curr;
            *curr = (bullet*)(*curr)->next;
            bullet_destroy(to_remove);
        } else {
            curr = (bullet**)&((*curr)->next);
        }
    }
}

// Desenha os tiros (bullets)
void player1_draw_bullets(player1 *p, int camera_x) {
    for (bullet *b = p->gun->shots; b != NULL; b = (bullet*)b->next) {
        int screen_x = b->x - camera_x;
        int screen_y = b->y;
        // Desenho da bala propriamente
        al_draw_filled_rectangle(
            screen_x, screen_y,
            screen_x + 4, screen_y + 4,
            al_map_rgb(255, 255, 255)
        );
    }
}

// Captura as hitbox do player
void player1_get_hitbox(player1 *p, int player_screen_x, int player_screen_y, int *hitbox_x, int *hitbox_y, int *hitbox_w, int *hitbox_h) {
    int sprite_w, sprite_h, draw_x, draw_y;

    // Hitbox para pulo
    if (p->is_jumping) {
        sprite_w = p->jump_frame_width * p->player_scale;
        sprite_h = p->jump_frame_height * p->player_scale;
        draw_x = player_screen_x;
        draw_y = p->player_y;
        *hitbox_w = sprite_w * 0.25;
        *hitbox_h = sprite_h * 0.5;
        *hitbox_x = draw_x + (sprite_w - *hitbox_w) / 2;
        *hitbox_y = draw_y + (sprite_h - *hitbox_h) / 2 + 50;
    // Hitbox para agachado
    } else if (p->is_squatting) {
        sprite_w = p->squat_frame_width * p->player_scale;
        sprite_h = p->squat_frame_height * p->player_scale;
        draw_x = player_screen_x + 20;
        draw_y = p->player_y + ((p->idle_frame_height - p->squat_frame_height) * p->player_scale);
        *hitbox_w = sprite_w * 0.25;
        *hitbox_h = sprite_h * 0.35;
        *hitbox_x = draw_x + (sprite_w - *hitbox_w) / 2 - 25;
        *hitbox_y = draw_y + (sprite_h - *hitbox_h) / 2 + 80;
    // Hitbox para em pé/parado/correndo
    } else {
        sprite_w = p->idle_frame_width * p->player_scale;
        sprite_h = p->idle_frame_height * p->player_scale;
        draw_x = player_screen_x;
        draw_y = p->player_y;
        *hitbox_w = sprite_w * 0.25;
        *hitbox_h = sprite_h * 0.45;
        *hitbox_x = draw_x + (sprite_w - *hitbox_w) / 2;
        *hitbox_y = draw_y + (sprite_h - *hitbox_h) / 2 + 60;
    }
}

// Liberação de memória
void player1_destroy(player1 *p) {
    joystick_destroy(p->control);
    if (p->idle) al_destroy_bitmap(p->idle);
    if (p->idle_left) al_destroy_bitmap(p->idle_left);
    if (p->run) al_destroy_bitmap(p->run);
    if (p->squat) al_destroy_bitmap(p->squat);
    if (p->jump) al_destroy_bitmap(p->jump);
    if (p->hurt) al_destroy_bitmap(p->hurt);
    if (p->gun) pistol_destroy(p->gun);
    if (p->dead) al_destroy_bitmap(p->dead);
    if (p->recharge) al_destroy_bitmap(p->recharge);
    if (p->hud_health) al_destroy_bitmap(p->hud_health);
    if (p->hud_ammo) al_destroy_bitmap(p->hud_ammo);
    if (p->gun_shot_sound) al_destroy_sample(p->gun_shot_sound);
    if (p->gun_reload_sound) al_destroy_sample(p->gun_reload_sound);
    free(p);
}