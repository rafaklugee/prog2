#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "vinac.h"
#include "lz.h"
#include "lista.h"

struct membro* busca_membro(int id, FILE *archive) {
    if (!archive) {
        return NULL;
    }

    // Garantindo que o arquivo será lido do início
    rewind(archive);

    // Pula o número de membros, que está no início do archive
    int membros;
    if (fread(&membros, sizeof(int), 1, archive) != 1)
        return NULL;

    struct membro *m = malloc(sizeof(struct membro));
    if (!m)
        return NULL;

    // Lê os membros e busca pelo id
    while (fread(m, sizeof(struct membro), 1, archive) == 1) {
        // Se o id for igual, achou o membro
        if (m->id == id) {
            return m;
        }

        // Pula os dados do membro achado no arquivo
        if (fseek(archive, m->tam_disco, SEEK_CUR) != 0) {
            free(m);
            return NULL;
        }
    }

    free(m);
    return NULL;
}


// Função para carregar os membros do arquivo (popular a lista)
struct lista_t *carregar_membros(FILE *arquivo, struct lista_t *lista_membros) {
    if (!arquivo)
        return NULL;

    int membros;
    if (fread(&membros, sizeof(int), 1, arquivo) != 1 || membros <= 0)
        return NULL;

    for (int i = 0; i < membros; i++) {
        // Não vou usar alocação dinâmica pois só uso o id de membro
        struct membro m;

        // Lê os metadados diretamente em uma variável da pilha
        if (fread(&m, sizeof(struct membro), 1, arquivo) != 1)
            return NULL;

        // Insere apenas o ID na lista
        if (!lista_insere(lista_membros, m.id, -1))
            return NULL;

        // Pula os dados do arquivo
        if (fseek(arquivo, m.tam_disco, SEEK_CUR) != 0)
            return NULL;
    }

    return lista_membros;
}

int comprimir_arquivo(FILE *entrada, FILE *arquivo_comprimido, int tam_original, int *tamanho_comprimido) {
    unsigned char *buffer_in = (unsigned char *)malloc(tam_original);
    if (!buffer_in) 
        return -1;

    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        free(buffer_in);
        return -1;
    }

    // Lê o conteúdo do arquivo de entrada para o buffer
    fread(buffer_in, 1, tam_original, entrada);

    // Chama a função de compressão
    *tamanho_comprimido = LZ_Compress(buffer_in, buffer_out, tam_original);
    if (*tamanho_comprimido < 0) {
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
    if (!buffer_in) 
        return -1;

    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        free(buffer_in);
        return -1;
    }

    // Lê o conteúdo do arquivo comprimido para o buffer
    size_t lidos = fread(buffer_in, 1, tamanho_comprimido, arquivo_comprimido);
    if (lidos != (size_t)tamanho_comprimido) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Descomprime os dados
    LZ_Uncompress(buffer_in, buffer_out, tamanho_comprimido);

    // Escreve o buffer descomprimido no arquivo de saída
    size_t escritos = fwrite(buffer_out, 1, tam_original, saida);
    if (escritos != (size_t)tam_original) {
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
    if (!entrada)
        return;

    // Verifica se o membro já existe no archive
    struct lista_t *temp_lista = lista_cria();
    FILE *archive_temp = fopen(nome_archive, "rb");
    if (archive_temp) {
        carregar_membros(archive_temp, temp_lista);
        struct item_t *temp = temp_lista->prim;
        while (temp) {
            struct membro *m = busca_membro(temp->valor, archive_temp);
            if (m && strcmp(m->nome, nome_arquivo) == 0) {
                printf("Membro %s já existe no archive. Substituindo...\n", nome_arquivo);
                remover_membro(nome_archive, nome_arquivo, lista_membros);
                free(m);
                break;
            }
            free(m);
            temp = temp->prox;
        }
        fclose(archive_temp);
    }
    lista_destroi(temp_lista);

    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive) {
        archive = fopen(nome_archive, "wb");
        if (!archive) {
            fclose(entrada);
            return;
        }

        int membros = 0;
        fwrite(&membros, sizeof(int), 1, archive);
    }

    struct stat st;
    if (stat(nome_arquivo, &st) == -1) {
        fclose(entrada);
        fclose(archive);
        return;
    }

    int uid = st.st_uid;
    int tam_original = st.st_size;
    int data = st.st_mtime;

    struct membro *novo_membro = (struct membro *)malloc(sizeof(struct membro));
    if (!novo_membro) {
        fclose(entrada);
        fclose(archive);
        return;
    }

    strcpy(novo_membro->nome, nome_arquivo);
    novo_membro->uid = uid;
    novo_membro->tam_original = tam_original;
    novo_membro->data = data;

    fseek(archive, 0, SEEK_END);
    long posicao_atual = ftell(archive);
    novo_membro->offset = posicao_atual + sizeof(struct membro);
    novo_membro->ordem = lista_tamanho(lista_membros);

    if (compressao) {
        FILE *arquivo_comprimido = fopen("arquivo_comprimido.lz", "wb");
        if (!arquivo_comprimido) {
            fclose(entrada);
            fclose(archive);
            free(novo_membro);
            return;
        }

        int tamanho_comprimido;
        if (comprimir_arquivo(entrada, arquivo_comprimido, tam_original, &tamanho_comprimido) < 0) {
            fclose(entrada);
            fclose(arquivo_comprimido);
            fclose(archive);
            free(novo_membro);
            return;
        }

        fclose(arquivo_comprimido);

        if (tamanho_comprimido >= tam_original) {
            printf("Arquivo comprimido maior ou igual ao original. Armazenando o arquivo original.\n");
            novo_membro->tam_disco = tam_original;
            novo_membro->comprimido = 0;

            rewind(entrada);

            unsigned char *buffer = (unsigned char *)malloc(tam_original);
            if (!buffer) {
                fclose(entrada);
                fclose(archive);
                free(novo_membro);
                return;
            }

            novo_membro->id = lista_tamanho(lista_membros);
            fwrite(novo_membro, sizeof(struct membro), 1, archive);
            size_t lidos = fread(buffer, 1, tam_original, entrada);
            fwrite(buffer, 1, lidos, archive);

            free(buffer);
        } else {
            arquivo_comprimido = fopen("arquivo_comprimido.lz", "rb");
            if (!arquivo_comprimido) {
                fclose(entrada);
                fclose(archive);
                free(novo_membro);
                return;
            }

            unsigned char *buffer_comprimido = (unsigned char *)malloc(tamanho_comprimido);
            if (!buffer_comprimido) {
                fclose(entrada);
                fclose(arquivo_comprimido);
                fclose(archive);
                free(novo_membro);
                return;
            }

            fread(buffer_comprimido, 1, tamanho_comprimido, arquivo_comprimido);

            novo_membro->id = lista_tamanho(lista_membros);
            novo_membro->tam_disco = tamanho_comprimido;
            novo_membro->comprimido = 1;

            fwrite(novo_membro, sizeof(struct membro), 1, archive);
            fwrite(buffer_comprimido, 1, tamanho_comprimido, archive);

            free(buffer_comprimido);
            fclose(arquivo_comprimido);
        }
    } else {
        novo_membro->tam_disco = tam_original;
        novo_membro->comprimido = 0;

        rewind(entrada);

        unsigned char *buffer = (unsigned char *)malloc(tam_original);
        if (!buffer) {
            fclose(entrada);
            fclose(archive);
            free(novo_membro);
            return;
        }

        novo_membro->id = lista_tamanho(lista_membros);
        fwrite(novo_membro, sizeof(struct membro), 1, archive);
        size_t lidos = fread(buffer, 1, tam_original, entrada);
        fwrite(buffer, 1, lidos, archive);

        free(buffer);
    }

    lista_insere(lista_membros, novo_membro->id, -1);
    free(novo_membro);

    rewind(archive);
    int tamanho_lista = lista_tamanho(lista_membros);
    fwrite(&tamanho_lista, sizeof(int), 1, archive);

    // Remover o arquivo comprimido
    if (compressao)
        remove("arquivo_comprimido.lz");

    fclose(entrada);
    fclose(archive);
}

void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros, int extrair_todos) {
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive)
        return;

    // Verifica se a lista já foi carregada, caso contrário, carrega a lista de membros
    if (lista_membros->prim == NULL) {
        lista_membros = carregar_membros(archive, lista_membros);
        if (!lista_membros) {
            fclose(archive);
            return;
        }
    }

    struct item_t *temp = lista_membros->prim;
    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            // Se não for para extrair todos, verifica se o nome corresponde
            if (!extrair_todos && strcmp(m->nome, nome_arquivo) != 0) {
                free(m);
                temp = temp->prox;
                continue;
            }

            // Cria o arquivo de saída
            FILE *saida = fopen(m->nome, "wb");
            if (!saida) {
                free(m);
                fclose(archive);
                return;
            }

            // Move o cursor do arquivo de archive para o offset do membro
            fseek(archive, m->offset, SEEK_SET);

            if (m->comprimido) {
                if (descomprimir_arquivo(saida, archive, m->tam_original, m->tam_disco) < 0) 
                    return;
            } else {
                unsigned char *buffer = (unsigned char *)malloc(m->tam_original);
                if (!buffer) {
                    fclose(saida);
                    fclose(archive);
                    free(m);
                    return;
                }

                size_t lidos = fread(buffer, 1, m->tam_original, archive);
                if (lidos != (size_t)m->tam_original) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(m);
                    return;
                }

                size_t escritos = fwrite(buffer, 1, m->tam_original, saida);
                if (escritos != (size_t)m->tam_original) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(m);
                    return;
                }

                free(buffer);
            }

            free(m);
            fclose(saida);

            if (!extrair_todos) 
                break;
        }
        temp = temp->prox;
    }

    fclose(archive);
}

void mover_membro(char *nome_archive, char *nome_membro, char *nome_target, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive)
        return;

    // Carregando os membros do archive para a lista
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    // Localizando o membro a ser movido
    struct membro *membro_a_mover = NULL;
    struct item_t *temp = lista_membros->prim;
    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m && strcmp(m->nome, nome_membro) == 0) {
            membro_a_mover = m;
            break;
        }
        temp = temp->prox;
    }

    if (!membro_a_mover) {
        fclose(archive);
        return;
    }

    // Localizar o membro target
    struct membro *membro_target = NULL;
    if (nome_target) {
        temp = lista_membros->prim;
        while (temp) {
            struct membro *m = busca_membro(temp->valor, archive);
            if (m && strcmp(m->nome, nome_target) == 0) {
                membro_target = m;
                break;
            }
            temp = temp->prox;
        }

        if (!membro_target) {
            free(membro_a_mover);
            fclose(archive);
            return;
        }
    }


    // Não implementada ainda
    // Implementar utilizando remover_membro e inserir_membro

    fclose(archive);
}

void remover_membro(char *nome_archive, char *nome_membro, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive)
        return;

    // Carregando os membros 
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    // Localizando o membro a ser removido
    struct membro *membro_a_remover = NULL;
    struct item_t *temp = lista_membros->prim;
    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m && strcmp(m->nome, nome_membro) == 0) {
            membro_a_remover = m;
            break;
        }
        free(m);
        temp = temp->prox;
    }

    if (!membro_a_remover) {
        fclose(archive);
        return;
    }

    // Criando um arquivo temporário para reorganizar os dados
    FILE *temp_archive = fopen("temp_archive.bin", "wb");
    if (!temp_archive) {
        free(membro_a_remover);
        fclose(archive);
        return;
    }

    rewind(archive);

    // Reescrevendo os membros no novo arquivo, tirando o que será removido
    int membros;
    fread(&membros, sizeof(int), 1, archive);
    int novos_membros = membros - 1;
    fwrite(&novos_membros, sizeof(int), 1, temp_archive);

    struct membro *m = (struct membro *)malloc(sizeof(struct membro));
    if (!m) {
        fclose(temp_archive);
        fclose(archive);
        free(membro_a_remover);
        return;
    }

    for (int i = 0; i < membros; i++) {
        fread(m, sizeof(struct membro), 1, archive);

        if (strcmp(m->nome, membro_a_remover->nome) == 0) {
            // Pula os dados do membro a ser removido
            fseek(archive, m->tam_disco, SEEK_CUR);
            continue;
        }

        // Atualizando o offset no novo arquivo
        m->offset = ftell(temp_archive) + sizeof(struct membro);
        fwrite(m, sizeof(struct membro), 1, temp_archive);

        // Copiando os dados do membro
        unsigned char *buffer = (unsigned char *)malloc(m->tam_disco);
        if (!buffer) {
            fclose(temp_archive);
            fclose(archive);
            free(m);
            free(membro_a_remover);
            return;
        }

        fread(buffer, 1, m->tam_disco, archive);
        fwrite(buffer, 1, m->tam_disco, temp_archive);
        free(buffer);
    }

    free(m);

    fclose(archive);
    fclose(temp_archive);

    // Substituindo o arquivo original pelo temporário
    remove(nome_archive);
    rename("temp_archive.bin", nome_archive);

    // Atualizando a lista de membros
    lista_retira(lista_membros, &membro_a_remover->id, lista_procura(lista_membros, membro_a_remover->id));

    free(membro_a_remover);
}

void listar_conteudo(char *nome_archive, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive)
        return;

    // Carregando os membros do arquivo para a lista
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    struct item_t *temp = lista_membros->prim;
    
    // Buscando e imprimindo os membros do archive
    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            printf("Nome: %s, UID: %d, Tamanho Original: %d, Tamanho Disco: %d, Data: %d, Ordem: %d, Offset: %d\n",
                   m->nome, m->uid, m->tam_original, m->tam_disco, m->data, m->ordem, m->offset);
            free(m);
        }
        temp = temp->prox;
    }

    fclose(archive);
}