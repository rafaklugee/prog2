#include <stdio.h>
#include <string.h>
#include "vinac.h"
#include "lista.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: vinac <opção> <arquivo.vc> [membro1 membro2 ...]\n");
        return 1;
    }
  
    char *opcao = argv[1];
    char *nome_archive = argv[2];

    // Verifica se o nome do arquivo termina com ".vc"
    if (!strstr(nome_archive, ".vc")) {
        fprintf(stderr, "Erro: O arquivo de archive deve ter a extensão .vc!\n");
        return 1;
    }

    // Criando a lista de membros
    struct lista_t *lista_membros = lista_cria();
    if (!lista_membros) {
        fprintf(stderr, "Erro ao criar a lista de membros!\n");
        return 1;
    }

    if (strcmp(opcao, "-ip") == 0 || strcmp(opcao, "-p") == 0) {
        // Inserir membros sem compressão
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            printf("Inserindo membro sem compressão: %s!\n", membro);
            inserir_membro(nome_archive, membro, 0, lista_membros); // 0 = sem compressão
        }
    } else if (strcmp(opcao, "-ic") == 0 || strcmp(opcao, "-i") == 0) {
        // Inserir membros com compressão
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            printf("Inserindo membro com compressão: %s!\n", membro);
            inserir_membro(nome_archive, membro, 1, lista_membros); // 1 = com compressão
        }
    } else if (strcmp(opcao, "-x") == 0) {
        // Extrair membros do archive
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            //printf("Extraindo membro %s do archive!\n", membro);
            extrair_membro(nome_archive, membro, lista_membros);
        }
    } else {
        fprintf(stderr, "Opção inválida: %s\n.", opcao);
        return 1;
    }

    return 0;
}