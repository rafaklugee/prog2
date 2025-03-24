#include "musica.h"
#include <stdio.h>

void requisitar(char *titulo, char *artista, char *preco){
	getchar();
	printf("\nDigite o título: ");
	fgets(titulo, 33, stdin);
	printf("Digite o artista: ");
	fgets(artista, 33, stdin);
    printf("Digite o preço: ");
	fgets(preco, 10, stdin);
}

disco *criar_musica() {
    disco *novo = (disco*)malloc(sizeof(disco));
    requisitar(novo->nome, novo->artista, novo->preco);
    novo->prox = NULL;

    return novo;
}

void *inserir_musica (disco *novo, fila_prio *fila) {
    if (!fila->inicio || novo->preco > fila->inicio->preco) {
        novo->prox = fila->inicio;
        fila->inicio = novo;
    }

    disco *atual;
    for (atual = fila->inicio; atual->prox->preco > novo->preco; atual = atual->prox) {
        novo->prox = atual->prox;
        atual->prox = novo;
    }

    return;
}

//disco *tocar_remover_musica (fila_prio *fila);
//void *atualizar_fila (fila_prio *fila);
//void *destruir_fila (fila_prio *fila);