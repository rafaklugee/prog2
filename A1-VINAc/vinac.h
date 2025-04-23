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

// Vou inserir o membro na lista, o valor compressão pode ser 0 (sem compressão) ou 1 (com compressão)
void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao);

void extrair_membro (char *nome_arquivo, int compressao);

void listar_membros (struct membro *arquivo);

struct membro* ler_diretorio (FILE *arquivo);

#endif