#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "vinac.h"
#include "lz.h"

// Definindo a uma lista global de membros
struct membro *lista_membros = NULL;

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

void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao) {
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
    struct membro *temp = lista_membros;
    while (temp) {
        offset += temp->tam_disco;
        ordem++;
        temp = temp->prox;
    }
    novo_membro->offset = offset;
    novo_membro->ordem = ordem;
    novo_membro->prox = NULL;

    // Insere o novo membro na lista
    if (!lista_membros) {
        lista_membros = novo_membro;
    } else {
        struct membro *ultimo = lista_membros;
        while (ultimo->prox) {
            ultimo = ultimo->prox;
        }
        ultimo->prox = novo_membro;
    }

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

        novo_membro->tam_disco = tamanho_comprimido;
        fclose(arquivo_comprimido);
    } else {
        novo_membro->tam_disco = tam_original;
    }

    // Escreve o membro no arquivo de archive
    fwrite(novo_membro, sizeof(struct membro), 1, archive);

    limpar(entrada, NULL, archive, NULL);
}