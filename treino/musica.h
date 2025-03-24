#ifndef _MUSICA_
#define _MUSICA_

typedef struct disco {
    char nome[32];
    char artista[32];
    char preco[10];

    struct disco *prox;
} disco;

typedef struct fila_prio {
    disco *inicio;
} fila_prio;

disco *criar_musica();
void *inserir_musica (disco *novo, fila_prio *fila);
disco *tocar_remover_musica (fila_prio *fila);
void *atualizar_fila (fila_prio *fila);
void *destruir_fila (fila_prio *fila);


#endif