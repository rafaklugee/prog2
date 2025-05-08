#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "vinac.h"
#include "lz.h"
#include "lista.h"

struct membro* busca_membro(int uid, FILE *archive) {
    if (!archive) {
        fprintf(stderr, "Arquivo não pode ser lido em busca_membro.\n");
        return NULL;
    }

    rewind(archive); // Garante que vamos ler desde o início do arquivo

    struct membro *m = malloc(sizeof(struct membro));
    if (!m) {
        fprintf(stderr, "Erro ao alocar memória para o membro.\n");
        return NULL;
    }

    while (fread(m, sizeof(struct membro), 1, archive) == 1) {
        if (m->uid == uid) {  // Busca pelo uid
            return m; // Encontrou, retorna
        }
    }

    free(m); // Não encontrou, libera memória
    return NULL;
}


// Função para carregar os membros do arquivo para a lista global
void carregar_membros(char *nome_archive, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive para leitura!\n");
        return;
    }

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
    size_t lidos = fread(buffer_in, 1, tamanho_comprimido, arquivo_comprimido);
    if (lidos != (size_t)tamanho_comprimido) {
        fprintf(stderr, "Erro ao ler dados comprimidos do arquivo (esperado: %d, lido: %zu)\n", tamanho_comprimido, lidos);
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Descomprime os dados (sem retorno, então assumimos sucesso)
    LZ_Uncompress(buffer_in, buffer_out, tamanho_comprimido);

    // Escreve o buffer descomprimido no arquivo de saída
    size_t escritos = fwrite(buffer_out, 1, tam_original, saida);
    if (escritos != (size_t)tam_original) {
        fprintf(stderr, "Erro ao escrever dados descomprimidos no arquivo de saída\n");
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

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
    fseek(archive, 0, SEEK_END); // Move o cursor para o final do arquivo
    novo_membro->offset = ftell(archive); // Posição do novo membro no arquivo
    novo_membro->ordem = lista_membros->tamanho; // Ordem atual
    

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

        // Escreve os dados do membro no archive
        fwrite(novo_membro, sizeof(struct membro), 1, archive);

        // Escreve os dados do arquivo original no archive
        fread(buffer, 1, tam_original, entrada);
        fwrite(buffer, 1, tam_original, archive);

        free(buffer);
    }

    // Insere o novo membro na lista
    lista_insere(lista_membros, novo_membro->uid, -1); // Insere no final da lista

    limpar(entrada, NULL, archive, NULL);

    printf ("Inseri o membro com as seguintes informações:\n");
    printf ("Nome: %s, uid: %d, tam_original: %d, tam_disco: %d, data: %d, ordem: %d, offset: %d, ehComprimido: %d\n",
            novo_membro->nome, novo_membro->uid, novo_membro->tam_original,
            novo_membro->tam_disco, novo_membro->data, novo_membro->ordem,
            novo_membro->offset, novo_membro->comprimido);
}

void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros) {
    printf ("Essa eh a lista de membros dentro de extrair_membro(antes): ");
    lista_imprime(lista_membros);
    printf ("\n");

    carregar_membros(nome_archive, lista_membros);

    printf ("Essa eh a lista de membros dentro de extrair_membro(depois): ");
    lista_imprime(lista_membros);
    printf ("\n");

    struct item_t *temp = lista_membros->prim;
    struct membro *membro_atual = NULL;

    FILE *archive = fopen(nome_archive, "rb");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive!\n");
        return;
    }

    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            membro_atual = m;
            break;
        }
        if (m)
            free(m);
        temp = temp->prox;
    }

    if (!membro_atual) {
        fprintf(stderr, "Arquivo %s não encontrado no archive\n", nome_arquivo);
        fclose(archive);
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

