#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "vinac.h"
#include "lz.h"
#include "lista.h"

struct membro* busca_membro(int id, FILE *archive) {
    if (!archive) {
        fprintf(stderr, "Arquivo não pode ser lido em busca_membro.\n");
        return NULL;
    }

    rewind(archive); // Garante que vamos ler desde o início do arquivo

    // Pula o cabeçalho (número de membros)
    int membros;
    if (fread(&membros, sizeof(int), 1, archive) != 1) {
        fprintf(stderr, "Erro ao ler o número de membros no arquivo.\n");
        return NULL;
    }

    struct membro *m = malloc(sizeof(struct membro));
    if (!m) {
        fprintf(stderr, "Erro ao alocar memória para o membro.\n");
        return NULL;
    }

    // Lê os membros e busca pelo id
    while (fread(m, sizeof(struct membro), 1, archive) == 1) {
        // Verifica se o membro já foi extraído
        struct stat st;
        if (stat(m->nome, &st) == 0) {
            printf("Arquivo %s já foi extraído (função busca), pulando...\n", m->nome);
            // Pula os dados do membro no arquivo
            if (fseek(archive, m->tam_disco, SEEK_CUR) != 0) {
                fprintf(stderr, "Erro ao pular os dados do membro %s no arquivo.\n", m->nome);
                free(m);
                return NULL;
            }
            continue; // Pula para o próximo membro
        }

        // Verifica se o ID corresponde ao membro desejado
        if (m->id == id) {
            return m; // Encontrou o membro desejado
        }

        // Pula os dados do membro no arquivo
        if (fseek(archive, m->tam_disco, SEEK_CUR) != 0) {
            fprintf(stderr, "Erro ao pular os dados do membro %s no arquivo.\n", m->nome);
            free(m);
            return NULL;
        }
    }

    free(m); // Não encontrou, libera memória
    return NULL;
}


// Função para carregar os membros do arquivo para a lista global
struct lista_t *carregar_membros(FILE *arquivo, struct lista_t *lista_membros) {
    if (!arquivo) {
        fprintf(stderr, "Arquivo não pode ser lido em carregar_membros.\n");
        return NULL;
    }

    int membros;
    fread(&membros, sizeof(int), 1, arquivo);

    if (membros <= 0) {
        fprintf(stderr, "Nenhum membro encontrado no arquivo.\n");
        return NULL;
    }

    printf("Número de membros no arquivo: %d\n", membros);

    for (int i = 0; i < membros; i++) {
        struct membro *m = malloc(sizeof(struct membro));
        if (!m) {
            fprintf(stderr, "Erro ao alocar memória para o membro.\n");
            return NULL;
        }

        if (fread(m, sizeof(struct membro), 1, arquivo) != 1) {
            fprintf(stderr, "Erro ao ler os dados do membro.\n");
            free(m);
            return NULL;
        }
        printf("Membro lido: %s, id: %d, Offset: %d, Tamanho: %d\n", m->nome, m->id, m->offset, m->tam_disco);
        lista_insere(lista_membros, m->id, -1);

        // Pula os dados do arquivo associado ao membro
        if (fseek(arquivo, m->tam_disco, SEEK_CUR) != 0) {
            fprintf(stderr, "Erro ao pular os dados do membro no arquivo.\n");
            free(m);
            return NULL;
        }
    }

    return lista_membros;
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

    FILE *archive = fopen(nome_archive, "r+b"); // Abre para leitura e escrita
    if (!archive) {
        // Se o arquivo não existir, cria um novo
        archive = fopen(nome_archive, "wb");
        if (!archive) {
            fprintf(stderr, "Erro ao criar o arquivo de archive %s\n", nome_archive);
            fclose(entrada);
            return;
        }

        // Inicializa o número de membros como 0
        int membros = 0; // Mantém para inicializar o arquivo
        fwrite(&membros, sizeof(int), 1, archive);
    }

    // Move o cursor para o final do arquivo para adicionar o novo membro
    fseek(archive, 0, SEEK_END);

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
    long posicao_atual = ftell(archive); // Posição atual no arquivo
    novo_membro->offset = posicao_atual + sizeof(struct membro); // Offset dos dados do membro
    novo_membro->ordem = lista_tamanho(lista_membros); // Ordem atual

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

        // Atualiza o ID do membro antes de escrever no arquivo
        novo_membro->id = lista_tamanho(lista_membros); // Atualiza o ID do membro

        // Escreve os metadados do membro no archive
        fwrite(novo_membro, sizeof(struct membro), 1, archive);

        // Escreve o conteúdo do arquivo original no archive
        fread(buffer, 1, tam_original, entrada);
        fwrite(buffer, 1, tam_original, archive);

        free(buffer);
    }


    // Insere o novo membro na lista
    lista_insere(lista_membros, novo_membro->id, -1); // Insere no final da lista

    // Atualiza o número de membros no arquivo
    rewind(archive);
    int tamanho_lista = lista_tamanho(lista_membros);
    
    printf ("Tamanho da lista: %d\n", tamanho_lista);
    fwrite(&tamanho_lista, sizeof(int), 1, archive);

    printf ("Membro=%s id=%d offset=%d tamanho=%d foi inserido no archive %s!\n", novo_membro->nome, novo_membro->id, novo_membro->offset, novo_membro->tam_disco, nome_archive);
    limpar(entrada, NULL, archive, NULL);
}

void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive) {
        fprintf(stderr, "Erro ao abrir o arquivo de archive!\n");
        return;
    }
    
    printf("\n\nIniciando extracao do membro %s\n\n", nome_arquivo);

    printf("Essa eh a lista de membros (antes) de extrair_membro: ");
    lista_imprime(lista_membros);
    printf("\n");

    // Verifica se a lista já foi carregada
    if (lista_membros->prim == NULL) {
        printf("Carregando membros do arquivo...\n");
        lista_membros = carregar_membros(archive, lista_membros);
    } else {
        printf("Lista de membros já carregada, pulando recarregamento.\n");
    }

    printf("Essa eh a lista de membros (depois) de extrair_membro: ");
    lista_imprime(lista_membros);
    printf("\n");

    struct item_t *temp = lista_membros->prim;
    struct membro *membro_atual = NULL;

    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            printf("Nome_arquivo: %s, Nome_encontrado: %s\n", nome_arquivo, m->nome);

            // Verifica se o nome do membro corresponde ao nome do arquivo desejado
            if (strcmp(m->nome, nome_arquivo) == 0) {
                // Verifica se o arquivo já foi extraído
                struct stat st;
                if (stat(nome_arquivo, &st) == 0) {
                    printf("Arquivo %s já foi extraído, pulando...\n", nome_arquivo);
                    free(m); // Libera a memória do membro
                    temp = temp->prox;
                    continue; // Pula para o próximo membro
                }

                membro_atual = m;
                break;
            }
            free(m); // Libera a memória se o membro não for o desejado
        }
        temp = temp->prox;
    }

    if (!membro_atual) {
        fprintf(stderr, "Arquivo %s não encontrado no archive ou já foi extraído.\n", nome_arquivo);
        fclose(archive);
        return;
    }

    FILE *saida = fopen(nome_arquivo, "wb");
    if (!saida) {
        fprintf(stderr, "Erro ao criar o arquivo de saída %s\n", nome_arquivo);
        free(membro_atual); // Libera a memória do membro encontrado
        fclose(archive);
        return;
    }

    fseek(archive, membro_atual->offset, SEEK_SET); // Posiciona no início dos dados do membro

    if (membro_atual->comprimido) {
        if (descomprimir_arquivo(saida, archive, membro_atual->tam_original, membro_atual->tam_disco) < 0) {
            fprintf(stderr, "Erro ao descomprimir o arquivo %s\n", nome_arquivo);
        }
    } else {
        unsigned char *buffer = (unsigned char *)malloc(membro_atual->tam_original);
        if (!buffer) {
            fprintf(stderr, "Erro ao alocar memória para o buffer\n");
            fclose(saida);
            free(membro_atual); // Libera a memória do membro encontrado
            fclose(archive);
            return;
        }

        size_t lidos = fread(buffer, 1, membro_atual->tam_original, archive);
        if (lidos != (size_t)membro_atual->tam_original) {
            fprintf(stderr, "Erro ao ler dados do arquivo (esperado: %d, lido: %zu)\n", membro_atual->tam_original, lidos);
            free(buffer);
            fclose(saida);
            free(membro_atual); // Libera a memória do membro encontrado
            fclose(archive);
            return;
        }

        size_t escritos = fwrite(buffer, 1, membro_atual->tam_original, saida);
        if (escritos != (size_t)membro_atual->tam_original) {
            fprintf(stderr, "Erro ao escrever dados no arquivo de saída (esperado: %d, escrito: %zu)\n", membro_atual->tam_original, escritos);
            free(buffer);
            fclose(saida);
            free(membro_atual); // Libera a memória do membro encontrado
            fclose(archive);
            return;
        }

        free(buffer);
    }

    printf("\n\nMembro %s extraído com sucesso!\n\n", membro_atual->nome);

    free(membro_atual); // Libera a memória do membro encontrado
    fclose(saida);
    fclose(archive);
}

