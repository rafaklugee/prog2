#include "jukefila.h"
#include "pedido.h"
#include <stdio.h>
#include <stdlib.h>

jukefila* criar_jukefila() {
    jukefila *nova = (jukefila *)malloc(sizeof(jukefila));
    if (!nova)
        return NULL;

    nova->inicio = NULL;
    nova->final = NULL;

    return nova;
}

void inserir_jukefila(pedido* elemento, jukefila* fila) {
    if (!elemento || !fila)
        return;
    
    // Se não há nada na fila, ou a música já tem o valor maior que a primeira
    if (!fila->inicio || elemento->valor > fila->inicio->valor) {
        elemento->proximo = fila->inicio;
        fila->inicio = elemento;
        return;
    }
    // Caso contrário, vai percorrer o vetor até achar uma música com o valor menor
    pedido *aux = fila->inicio;
    while (aux->proximo && aux->proximo->valor >= elemento->valor)
      aux = aux->proximo;
    
    elemento->proximo = aux->proximo;
    aux->proximo = elemento;
}

pedido* consumir_jukefila(jukefila* fila) {
    if (!fila || !fila->inicio)
        return NULL;

    pedido *aux;

    aux = fila->inicio;
    fila->inicio = aux->proximo;
    aux->proximo = NULL;

    return aux;
}

unsigned int contar_jukefila(jukefila* fila) {
    if (!fila)
        return 0;

    unsigned int contador = 0;
    pedido *aux;
    for (aux = fila->inicio; aux != NULL; aux = aux->proximo)
        contador++;

    return contador;
}

void destruir_jukefila(jukefila *fila) {
    if (!fila)
        return;

    while(fila->inicio)
        free(consumir_jukefila(fila));

    free(fila);
}