#include <stdio.h>
#include "vinac.h"
#include "lista.h"

struct membro *buscar_membro(struct lista_membros *lista, char *nome_arquivo) {
    struct membro *atual = lista->inicio;
    while (atual) {
        if (strcmp(atual->nome, nome_arquivo) == 0) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

void liberar_lista(struct lista_membros *lista) {
    struct membro *atual = lista->inicio;
    while (atual) {
        struct membro *prox = atual->prox;
        free(atual);
        atual = prox;
    }
    lista->inicio = NULL;
    lista->quantidade = 0;
}

void adicionar_membro(struct lista_membros *lista, struct membro *novo_membro) {
    if (!lista->inicio) {
        lista->inicio = novo_membro;
    } else {
        struct membro *ultimo = lista->inicio;
        while (ultimo->prox) {
            ultimo = ultimo->prox;
        }
        ultimo->prox = novo_membro;
    }
    lista->quantidade++;
}