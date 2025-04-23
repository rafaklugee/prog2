#ifndef __VINAC_H__
#define __VINAC_H__
#include <stdio.h>
#include <stdlib.h>

struct membro {
    char nome[50];
    int uid;
    int tam_original;
    int tam_disco;
    int data;
    int ordem;
    int offset;
    struct membro *prox;
};

// Inserindo o membro na lista encadeada (-ip e -ic)
void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao);

// Extraindo o membro da lista encadeada (-x)
void extrair_membro (char *nome_arquivo, int compressao);

// Removendo os membros da lista encadeada (-r)
void remove_membro (char *nome_arquivo, int compressao);

// Lista o contéudo do archive em ordem (-c)
void listar_conteudo (char *nome_arquivo);

// Move o membro para depois do membro especificado em archive (-m membro)
void mover_membro (char *nome_arquivo, struct membro *membro);

#endif