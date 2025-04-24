// TAD lista de números inteiros
// Carlos Maziero - DINF/UFPR, Out 2024
//
// Implementação do TAD
//
// Implementação com lista encadeada dupla não-circular

#include <stdio.h>
#include <stdlib.h>
#include "lista.h"

#define TAMANHO 1000000

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

    int item;

    // Remove os itens da lista.
    while (lista_tamanho(lst) > 0)
        lista_retira(lst, &item, 0);

    // Destroi a lista em si.
    free(lst);

    return NULL;    
}

int lista_insere (struct lista_t *lst, int item, int pos) {
    if (!lst)
        return -1;
    
    struct item_t *novo;

    novo = malloc (sizeof(struct item_t));
    if (!novo)
        return -1;

    novo->valor = item;
    novo->ant = NULL;
    novo->prox = NULL;
    
    // Se o tamanho da lista for 0, o primeiro elemento é igual ao último.
    // Se a posição for -1 ou além da lista, há inserção no fim da lista.
    if (pos == -1 || pos >= lst->tamanho) {
        if (lst->tamanho == 0) {
            lst->prim = novo;
            lst->ult = novo;
        }
        else {
            novo->ant = lst->ult;
            lst->ult->prox = novo;
            lst->ult = novo;
        }
    } else {
        struct item_t *aux = lst->prim;

        // Chegando a posição para inserção.
        for (int i = 0; i < pos; i++)
            aux = aux->prox;

        // Ajustando os ponteiros de "novo".
        novo->prox = aux;      
        novo->ant = aux->ant;

        // Se o valor antigo não for o primeiro valor da lista,
        // o valor anterior a ele aponta para o próximo (novo).

        // Caso contrário, o novo valor terá que ser o primeiro da lista.
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

int lista_retira (struct lista_t *lst, int *item, int pos) {
    if (!lst || lst->tamanho == 0 || pos >= lst->tamanho)
        return -1;

    // Se posição for -1, retira o último valor.
    if (pos == -1) {
        pos = lst->tamanho - 1;
    }

    struct item_t *aux = lst->prim;

    // Chegando a posição para retirada.
    for (int i = 0; i < pos; i++)
        aux = aux->prox;

    *item = aux->valor;

    // Se não houver um valor anterior ou um próximo valor,
    // aux será o primeiro valor ou o último valor, respectivamente.
    
    // Caso contrário, há o ajuste convencional dos ponteiros.
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

int lista_consulta (struct lista_t *lst, int *item, int pos) {
    if (!lst || lst->tamanho == 0 || pos >= lst->tamanho)
        return -1;

    // Se posição for -1, retorna o último valor.
    if (pos == -1) {
        *item = lst->ult->valor;
    }
    else {
        struct item_t *aux = lst->prim;

        // Capturando a posição do valor.
        for (int i = 0; i < pos; i++)
            aux = aux->prox;

        *item = aux->valor;
    }

    return lst->tamanho;
}

int lista_procura (struct lista_t *lst, int valor) {
    if (!lst || lst->tamanho == 0)
        return -1;

    int tam = lst->tamanho;

    struct item_t *aux = lst->prim;

    // Na primeira vez que encontrar o valor passado, retorna de imediato.
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

    // Passa item por item, começando do primeiro valor, até acabar.
    while (item) {
        if (item->prox)
            printf ("%d ", item->valor);
        else
            printf ("%d", item->valor);

        item = item->prox;
    }
}