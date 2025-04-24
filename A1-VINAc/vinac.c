#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "vinac.h"
#include "lz.h"
#include "lista.h"

// Função para carregar os membros do arquivo para a lista global
void carregar_membros(char *nome_archive, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive para leitura!\n");
        return;
    }

    lista_destroi(lista_membros); // Limpa a lista antes de carregar

    struct membro temp_membro;
    while (fread(&temp_membro, sizeof(struct membro), 1, archive) == 1) {
        struct membro *novo_membro = (struct membro *)malloc(sizeof(struct membro));
        if (!novo_membro) {
            fprintf(stderr, "Erro ao alocar memória para o membro\n");
            fclose(archive);
            return;
        }
        *novo_membro = temp_membro;

        // Insere o membro na lista
        lista_insere(lista_membros, novo_membro->uid, -1);
    }

    fclose(archive);
}

void limpar(FILE *entrada, FILE *arquivo_comprimido, FILE *archive, struct membro *novo_membro) {
    if (entrada) 
        fclose(entrada);
    if (arquivo_comprimido)
        fclose(arquivo_comprimido);
    if (archive)
        fclose(archive);
    if (novo_membro)
        free(novo_membro);
}

int comprimir_arquivo(FILE *entrada, FILE *arquivo_comprimido, int tam_original, int *tamanho_comprimido) {
    unsigned char *buffer_in = (unsigned char *)malloc(tam_original);
    if (!buffer_in) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de entrada\n");
        return -1;
    }

    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de saída\n");
        free(buffer_in);
        return -1;
    }

    // Lê o conteúdo do arquivo de entrada para o buffer
    fread(buffer_in, 1, tam_original, entrada);

    // Chama a função de compressão
    *tamanho_comprimido = LZ_Compress(buffer_in, buffer_out, tam_original);
    if (*tamanho_comprimido < 0) {
        fprintf(stderr, "Erro ao comprimir o arquivo\n");
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Escreve o buffer comprimido no arquivo de saída
    fwrite(buffer_out, 1, *tamanho_comprimido, arquivo_comprimido);

    free(buffer_in);
    free(buffer_out);

    return 0;
}

int descomprimir_arquivo(FILE *saida, FILE *arquivo_comprimido, int tam_original, int tamanho_comprimido) {
    unsigned char *buffer_in = (unsigned char *)malloc(tamanho_comprimido);
    if (!buffer_in) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de entrada\n");
        return -1;
    }

    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        fprintf(stderr, "Erro ao alocar memória para o buffer de saída\n");
        free(buffer_in);
        return -1;
    }

    // Lê o conteúdo do arquivo comprimido para o buffer
    fread(buffer_in, 1, tamanho_comprimido, arquivo_comprimido);

    // Chama a função de descompressão
    LZ_Uncompress(buffer_in, buffer_out, tamanho_comprimido);
    if (!buffer_out) {
        fprintf(stderr, "Erro ao descomprimir o arquivo\n");
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Escreve o buffer descomprimido no arquivo de saída
    fwrite(buffer_out, 1, tam_original, saida);

    free(buffer_in);
    free(buffer_out);

    return 0;
}

void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao, struct lista_t *lista_membros) {
    FILE *entrada = fopen(nome_arquivo, "rb");
    if (!entrada) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", nome_arquivo);
        return;
    }

    FILE *archive = fopen(nome_archive, "ab");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive %s\n", nome_archive);
        limpar(entrada, NULL, NULL, NULL);
        return;
    }

    struct stat st;
    if (stat(nome_arquivo, &st) == -1) {
        fprintf(stderr, "Erro ao obter informações do arquivo %s\n", nome_arquivo);
        limpar(entrada, NULL, archive, NULL);
        return;
    }

    int uid = st.st_uid;
    int tam_original = st.st_size;
    int data = st.st_mtime;

    struct membro *novo_membro = (struct membro *)malloc(sizeof(struct membro));
    if (!novo_membro) {
        fprintf(stderr, "Erro ao alocar memória para o novo membro\n");
        limpar(entrada, NULL, archive, NULL);
        return;
    }

    strcpy(novo_membro->nome, nome_arquivo);
    novo_membro->uid = uid;
    novo_membro->tam_original = tam_original;
    novo_membro->data = data;

    // Calcula o offset e a ordem
    int offset = 0, ordem = 0;
    struct item_t *temp = lista_membros->prim;
    while (temp) {
        offset += temp->membro->tam_disco; // Acessa o tamanho do membro atual
        ordem++;
        temp = temp->prox;
    }
    novo_membro->offset = offset;
    novo_membro->ordem = ordem;

    // Se houver compressão, comprime o arquivo
    if (compressao) {
        FILE *arquivo_comprimido = fopen("arquivo_comprimido.lz", "wb");
        if (!arquivo_comprimido) {
            fprintf(stderr, "Erro ao abrir o arquivo comprimido\n");
            limpar(entrada, NULL, archive, novo_membro);
            return;
        }

        int tamanho_comprimido;
        if (comprimir_arquivo(entrada, arquivo_comprimido, tam_original, &tamanho_comprimido) < 0) {
            limpar(entrada, arquivo_comprimido, archive, novo_membro);
            return;
        }

        fclose(arquivo_comprimido);

        // Reabre o arquivo comprimido para leitura
        arquivo_comprimido = fopen("arquivo_comprimido.lz", "rb");
        if (!arquivo_comprimido) {
            fprintf(stderr, "Erro ao reabrir o arquivo comprimido\n");
            limpar(entrada, NULL, archive, novo_membro);
            return;
        }

        // Aloca buffer para os dados comprimidos
        unsigned char *buffer_comprimido = (unsigned char *)malloc(tamanho_comprimido);
        if (!buffer_comprimido) {
            fprintf(stderr, "Erro ao alocar memória para o buffer comprimido\n");
            limpar(entrada, arquivo_comprimido, archive, novo_membro);
            return;
        }

        // Lê os dados comprimidos e escreve no arquivo de archive
        fread(buffer_comprimido, 1, tamanho_comprimido, arquivo_comprimido);
        fwrite(buffer_comprimido, 1, tamanho_comprimido, archive);

        free(buffer_comprimido);
        fclose(arquivo_comprimido);

        novo_membro->tam_disco = tamanho_comprimido;
        novo_membro->comprimido = 1;

        // Remove o arquivo temporário
        remove("arquivo_comprimido.lz");
    } else {
        novo_membro->tam_disco = tam_original;
        novo_membro->comprimido = 0;

        rewind(entrada);
        unsigned char *buffer = (unsigned char *)malloc(tam_original);
        if (!buffer) {
            fprintf(stderr, "Erro ao alocar memória para o buffer\n");
            limpar(entrada, NULL, archive, NULL);
            return;
        }

        fread(buffer, 1, tam_original, entrada);
        fwrite(buffer, 1, tam_original, archive);

        free(buffer);
    }

    // Escreve o membro no arquivo de archive
    fwrite(novo_membro, sizeof(struct membro), 1, archive);

    // Insere o novo membro na lista
    lista_insere(lista_membros, novo_membro->uid, -1); // Insere no final da lista

    printf("Inserindo membro: %s, offset: %d, tamanho: %d\n",
           novo_membro->nome, novo_membro->offset, novo_membro->tam_disco);

    limpar(entrada, NULL, archive, NULL);
}

void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros) {
    carregar_membros(nome_archive, lista_membros);

    struct item_t *temp = lista_membros->prim;
    struct membro *membro_atual = NULL;

    while (temp) {
        if (strcmp(temp->membro->nome, nome_arquivo) == 0) {
            membro_atual = temp->membro;
            break;
        }
        temp = temp->prox;
    }

    if (!membro_atual) {
        fprintf(stderr, "Arquivo %s não encontrado no archive\n", nome_arquivo);
        return;
    }

    FILE *archive = fopen(nome_archive, "rb");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive!\n");
        return;
    }

    FILE *saida = fopen(nome_arquivo, "wb");
    if (!saida) {
        fprintf(stderr, "Erro ao criar o arquivo de saída %s\n", nome_arquivo);
        fclose(archive);
        return;
    }

    fseek(archive, membro_atual->offset, SEEK_SET);

    if (membro_atual->comprimido) {
        if (descomprimir_arquivo(saida, archive, membro_atual->tam_original, membro_atual->tam_disco) < 0) {
            fprintf(stderr, "Erro ao descomprimir o arquivo %s\n", nome_arquivo);
        }
    } else {
        unsigned char *buffer = (unsigned char *)malloc(membro_atual->tam_original);
        if (!buffer) {
            fprintf(stderr, "Erro ao alocar memória para o buffer\n");
            fclose(saida);
            fclose(archive);
            return;
        }

        fread(buffer, 1, membro_atual->tam_original, archive);
        fwrite(buffer, 1, membro_atual->tam_original, saida);

        free(buffer);
    }

    fclose(saida);
    fclose(archive);
}
