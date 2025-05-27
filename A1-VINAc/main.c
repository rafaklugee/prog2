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
        fprintf(stderr, "Erro: O archive deve ter a extensão .vc!\n");
        return 1;
    }

    // Criando a lista de membros
    struct lista_t *lista_membros = lista_cria();
    if (!lista_membros)
        return 1;

    // Inserir membros sem compressão
    if (strcmp(opcao, "-ip") == 0 || strcmp(opcao, "-p") == 0) {
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            printf("Inserindo membro sem compressão: %s!\n", membro);
            inserir_membro(nome_archive, membro, 0, lista_membros);
        }
    }

    // Inserir membros com compressão
    else if (strcmp(opcao, "-ic") == 0 || strcmp(opcao, "-i") == 0) {    
        for (int i = 3; i < argc; i++) {
            char *membro = argv[i];
            printf("Inserindo membro com compressão: %s!\n", membro);
            inserir_membro(nome_archive, membro, 1, lista_membros);
        }
    } 

    // Extrair membros
    else if (strcmp(opcao, "-x") == 0) {
        if (argc == 3) {
            // Nenhum membro especificado, extrair todos
            printf("Extraindo todos os membros!\n");
            extrair_membro(nome_archive, NULL, lista_membros, 1);
        } else {
            // Extrair membros específicos
            for (int i = 3; i < argc; i++) {
                printf ("Extraindo membro: %s!\n", argv[i]);
                char *membro = argv[i];
                extrair_membro(nome_archive, membro, lista_membros, 0);
            }
        }
    } 

    // Mover membros
    else if (strcmp(opcao, "-m") == 0) {
        if (argc < 5)
            return 1;

        printf ("Movendo membro: %s!\n", argv[3]);
        char *membro = argv[3];
        char *membro_alvo = argv[4];
        mover_membro(nome_archive, membro, membro_alvo, lista_membros);
    } 

    // Remover membros
    else if (strcmp(opcao, "-r") == 0) {
        if (argc < 4)
            return 1;

        printf ("Removendo membro: %s!\n", argv[3]);
        char *membro = argv[3];
        remover_membro(nome_archive, membro, lista_membros);
    }

    // Listar membros
    else if (strcmp(opcao, "-c") == 0) {
        printf("Conteúdo do archive '%s':\n", nome_archive);
        listar_conteudo(nome_archive, lista_membros);
    }

    // Derivar membros
    else if (strcmp(opcao, "-z") == 0) {
        char **membros = &argv[3];
        int qtd_membros = argc - 3; // Pois (0)programa, (1)opção, 2(archive)
        derivar_archive(nome_archive, membros, qtd_membros, lista_membros);
    }

    // Opção inválida
    else {
        fprintf(stderr, "Opção inválida: %s\n.", opcao);
        return 1;
    }

    lista_destroi(lista_membros);

    return 0;
}