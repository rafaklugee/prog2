#ifndef __VINAC_H__
#define __VINAC_H__
#include <stdio.h>
#include <stdlib.h>
#include "lista.h"

struct membro {
    char nome[50];
    int id;
    int uid;
    int tam_original;
    int tam_disco;
    int data;
    int ordem;
    int offset;
    int comprimido;
    int offset_puro;
    int offset_antigo;
    struct membro *prox;
    struct membro *ant;
};

// Inserindo o membro na lista encadeada (-ip e -ic)
void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao, struct lista_t *lista_membros);

// Extraindo o membro da lista encadeada (-x)
void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros, int extrair_todos);

// Move o membro para depois do membro especificado em archive (-m membro)
void mover_membro(char *nome_archive, char *nome_membro, char *nome_target, struct lista_t *lista_membros);

// Removendo os membros da lista encadeada (-r)
void remover_membro(char *nome_archive, char *nome_membro, struct lista_t *lista_membros);

// Lista o contéudo do archive em ordem (-c)
void listar_conteudo(char *nome_archive, struct lista_t *lista_membros);

#endif