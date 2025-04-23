#include <stdio.h>
#include <stdlib.h>
#include "lista.h"

struct lista_t *lista_cria () {
    struct lista_t *l;

    l = malloc (sizeof(struct lista_t));
    if (!l)
        return NULL;

    l->prim = NULL;
    l->ult = NULL;
    l->tamanho = 0;

    return l;
}

struct lista_t *lista_destroi (struct lista_t *lst) {
    if (!lst)
        return NULL;

    void *item;

    // Remove os itens da lista.
    while (lista_tamanho(lst) > 0) {
        lista_retira(lst, &item, 0);
        free(item);
    }

    return NULL;    
}

int lista_insere(struct lista_t *lst, void *item, int pos) {
    if (!lst)
        return -1;
    
    struct item_t *novo;

    novo = malloc(sizeof(struct item_t));
    if (!novo)
        return -1;

    novo->valor = item; // Agora armazena um ponteiro genérico
    novo->ant = NULL;
    novo->prox = NULL;
    
    if (pos == -1 || pos >= lst->tamanho) {
        if (lst->tamanho == 0) {
            lst->prim = novo;
            lst->ult = novo;
        } else {
            novo->ant = lst->ult;
            lst->ult->prox = novo;
            lst->ult = novo;
        }
    } else {
        struct item_t *aux = lst->prim;

        for (int i = 0; i < pos; i++)
            aux = aux->prox;

        novo->prox = aux;      
        novo->ant = aux->ant;

        if (aux->ant) { 
            aux->ant->prox = novo;
        } else {
            lst->prim = novo;
        }

        aux->ant = novo;
    }

    lst->tamanho++;
    return lst->tamanho;
}

int lista_retira(struct lista_t *lst, void **item, int pos) {
    if (!lst || lst->tamanho == 0 || pos >= lst->tamanho)
        return -1;

    if (pos == -1) {
        pos = lst->tamanho - 1;
    }

    struct item_t *aux = lst->prim;

    for (int i = 0; i < pos; i++)
        aux = aux->prox;

    *item = aux->valor; // Retorna o ponteiro armazenado

    if (!aux->ant)
        lst->prim = aux->prox;
    else
        aux->ant->prox = aux->prox;

    if (!aux->prox)
        lst->ult = aux->ant;
    else
        aux->prox->ant = aux->ant;

    free(aux);

    lst->tamanho--;
    return lst->tamanho;
}

int lista_consulta(struct lista_t *lst, void **item, int pos) {
    if (!lst || lst->tamanho == 0 || pos >= lst->tamanho)
        return -1;

    struct item_t *aux = lst->prim;

    for (int i = 0; i < pos; i++)
        aux = aux->prox;

    *item = aux->valor; // Retorna o ponteiro armazenado

    return lst->tamanho;
}

int lista_procura (struct lista_t *lst, void *valor) {
    if (!lst || lst->tamanho == 0)
        return -1;

    int tam = lst->tamanho;

    struct item_t *aux = lst->prim;

    for (int i = 0 ; i < tam; i++) {
        if (aux->valor == valor) {
            return i;
        }
        aux = aux->prox;
    }

    return -1;
}

int lista_tamanho (struct lista_t *lst) {
    if (!lst)
        return -1;

    return (lst->tamanho);
}

void lista_imprime (struct lista_t *lst) {
    if (!lst)
        return;
    if (lista_tamanho(lst) <= 0)
        return;

    struct item_t *item;

    item = lst->prim;

    while (item) {
        if (item->prox)
            printf ("%d ", item->valor);
        else
            printf ("%d", item->valor);

        item = item->prox;
    }
}