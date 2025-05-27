#include "fila_jornal.h"
#include <stdio.h>

void requisita(char *titulo, char *texto){
    getchar();
    printf("\nDigite o título: ");
    fgets(titulo, 33, stdin);
    printf("Digite o texto: ");
    fgets(texto, 513, stdin);
}

struct noticia *criar_noticia (struct noticia *nova) {
    if (!nova)
        return NULL;
    
    requisita(nova->titulo, nova->texto);
    nova->idade = 0;
    nova->prox = NULL;

    return nova;
}

void inserir_noticia (struct noticia *nova, struct fila_jornal *fila) {
    if (!nova || !fila)
        return;

    if (fila->final) {
        fila->final->prox = nova;
        nova->prox = NULL;
        fila->final = nova;
    } else {
        fila->inicio = nova;
        fila->final = nova;
    }

    return;
}

void remover_noticia (struct fila_jornal *fila) {
    if (!fila || !fila->inicio)
        return;

    struct noticia *aux;    

    if (fila->inicio) {
        aux = fila->inicio;
        fila->inicio = aux->prox;
        aux->prox = NULL;
    
    } else {
        return;
    }

    free(aux);
    return;
}

void atualizar_fila (struct fila_jornal *fila) {
    struct noticia *anterior = NULL;
    struct noticia *atual;

    for (atual = fila->inicio; atual != NULL; atual = atual->prox) {
        if (atual->idade == 3) {
            // Ver isso depois
        }

        else
            atual->idade++;
        
        anterior = atual;
    } 
}

void destruir_fila (struct fila_jornal *fila) {
    if (!fila)
        return;

    while(fila->inicio)
        remover_noticia(fila);

    free(fila);
}