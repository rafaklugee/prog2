#ifndef __LISTA_H__
#define __LISTA_H__
#include <stdio.h>

// Definindo uma lista global de membros
struct lista_membros {
    struct membro *inicio;
    int quantidade;       
};

struct membro *buscar_membro(struct lista_membros *lista, char *nome_arquivo);

void liberar_lista(struct lista_membros *lista);

void adicionar_membro(struct lista_membros *lista, struct membro *novo_membro);

#endif