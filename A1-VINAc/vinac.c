#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "vinac.h"
#include "lz.h"
#include "lista.h"

// Essa função busca algum membro e o retorna. A função que utiliza que verá se é o membro que ela quer
// Enquanto ela faz essa busca, ela atualiza os ponteiros de quem ela está buscando -> Isso é preciso ? Não!
struct membro* busca_membro(int id, FILE *archive) {
    if (!archive)
        return NULL;

    // Vai ler o archive do começo
    rewind(archive);

    // Pula o número de membros, que está no início do archive
    int membros;
    if (fread(&membros, sizeof(int), 1, archive) != 1) {
        return NULL;
    }

    // Lê os membros e busca pelo id
    for (int i = 0; i < membros; i++) {
        struct membro *atual = malloc(sizeof(struct membro));
        if (!atual) {
            return NULL;
        }

        // Lê os membros a cada struct membro
        if (fread(atual, sizeof(struct membro), 1, archive) != 1) {
            free(atual);
            return NULL;
        }

        // Se o id for igual, retorna o membro
        if (atual->id == id) {
            return atual;
        }

        // Pula os dados do membro que já foi lido no arquivo
        if (fseek(archive, atual->tam_disco, SEEK_CUR) != 0) {
            free(atual);
            return NULL;
        }

        free(atual);
    }

    return NULL;
}


// Função para carregar os membros do arquivo (popular a lista)
struct lista_t *carregar_membros(FILE *arquivo, struct lista_t *lista_membros, struct membro **primeiro_membro_out) {
    if (!arquivo || !lista_membros) {
        return NULL;
    }

    // Vai ler o archive do começo
    rewind(arquivo);

    // Lê a quantidade de membros que tem no archive
    int qtd_membros;
    if (fread(&qtd_membros, sizeof(int), 1, arquivo) != 1 || qtd_membros <= 0) {
        return NULL;
    }

    struct membro *primeiro_membro = NULL;
    struct membro *ultimo_membro = NULL;

    for (int i = 0; i < qtd_membros; i++) {
        struct membro *m = malloc(sizeof(struct membro));
        if (!m) 
            return NULL;

        if (fread(m, sizeof(struct membro), 1, arquivo) != 1) {
            free(m);
            return NULL;
        }

        m->ant = ultimo_membro;
        m->prox = NULL;

        if (ultimo_membro)
            ultimo_membro->prox = m;
        else
            primeiro_membro = m;

        ultimo_membro = m;

        if (!lista_insere(lista_membros, m->id, -1)) {
            free(m);
            return NULL;
        }

        if (fseek(arquivo, m->tam_disco, SEEK_CUR) != 0) {
            free(m);
            return NULL;
        }
    }

    // Retorna a lista de membros com o id de cada um e os membros com seus ponteiros ant e prox corretos
    *primeiro_membro_out = primeiro_membro;
    return lista_membros;
}

int comprimir_arquivo(FILE *entrada, FILE *arquivo_comprimido, int tam_original, int *tamanho_comprimido) {
    // Para que eu preciso de buffers ? Não posso apenas passar meus arquivos originais ?
    // Isso deve ter haver com as funções do lz, de comprimir e descomprimir -> Sim! Essas funções trabalham com blocos de memória, não arquivos

    // Alocando meu buffer de entrada, ele tem o tamanho original do meu arquivo
    unsigned char *buffer_in = (unsigned char *)malloc(tam_original);
    if (!buffer_in) 
        return -1;

    // Alocando meu buffer de saída, ele tem o tamanho original do meu arquivo
    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        free(buffer_in);
        return -1;
    }

    // Coloca o conteúdo do arquivo de entrada no buffer
    fread(buffer_in, 1, tam_original, entrada);

    // Chama a função de compressão, colocando os buffers de entrada e saída, e passando o tamanho original do arquivo
    *tamanho_comprimido = LZ_Compress(buffer_in, buffer_out, tam_original);
    if (*tamanho_comprimido < 0) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Escreve o buffer out, agora comprimido no próprio buffer out (precisa disso ?)
    fwrite(buffer_out, 1, *tamanho_comprimido, arquivo_comprimido);

    // Libera os buffers
    free(buffer_in);
    free(buffer_out);

    return 0;
}

int descomprimir_arquivo(FILE *saida, FILE *arquivo_comprimido, int tam_original, int tamanho_comprimido) {
    // Alocando meu buffer de entrada, ele tem o tamanho do meu arquivo comprimido
    unsigned char *buffer_in = (unsigned char *)malloc(tamanho_comprimido);
    if (!buffer_in) 
        return -1;

    // Alocando meu buffer de saída, ele tem o tamanho do original do meu arquivo (tamanho não comprimido)
    unsigned char *buffer_out = (unsigned char *)malloc(tam_original);
    if (!buffer_out) {
        free(buffer_in);
        return -1;
    }

    // Coloca o conteúdo do arquivo comprimido de entrada no buffer
    size_t lidos = fread(buffer_in, 1, tamanho_comprimido, arquivo_comprimido);
    if (lidos != (size_t)tamanho_comprimido) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Chama a função de descompressão, colocando os buffers de entrada e saída, e passando o tamanho do arquivo comprimido
    LZ_Uncompress(buffer_in, buffer_out, tamanho_comprimido);

    // Escreve o buffer out, agora descomprimido no próprio buffer out (precisa disso ?)
    size_t escritos = fwrite(buffer_out, 1, tam_original, saida);
    if (escritos != (size_t)tam_original) {
        free(buffer_in);
        free(buffer_out);
        return -1;
    }

    // Libera os buffers
    free(buffer_in);
    free(buffer_out);

    return 0;
}

void inserir_membro(char *nome_archive, char *nome_arquivo, int compressao, struct lista_t *lista_membros) {
    // Abrindo meu membro que será inserido
    FILE *entrada = fopen(nome_arquivo, "rb");
    if (!entrada)
        return;

    struct membro *primeiro = NULL;

    // Verifica se o membro já existe no archive
    // Criando uma lista temporária
    struct lista_t *temp_lista = lista_cria();
    // Abrindo temporariamente meu archive
    FILE *archive_temp = fopen(nome_archive, "rb");
    if (archive_temp) {
        // Carregando meus membros do archive e inserindo eles na lista
        carregar_membros(archive_temp, temp_lista, &primeiro);
        struct item_t *temp = temp_lista->prim;
        while (temp) {
            // Buscando um membro qualquer e verificando se ele já existe no archive
            struct membro *m = busca_membro(temp->valor, archive_temp);
            if (m && strcmp(m->nome, nome_arquivo) == 0) {
                printf("Membro %s já existe no archive. Substituindo...\n", nome_arquivo);
                // Se ele já existir no archive, ele é removido, pois será substituído
                remover_membro(nome_archive, nome_arquivo, lista_membros);
                free(m);
                break;
            }
            free(m);
            temp = temp->prox;
        }
        // Destroi o carregamento do meu archive temporário
        fclose(archive_temp);
    }
    // Destroi minha lista temporária
    lista_destroi(temp_lista);

    // Abrindo meu archive com o nome passado pela linha de comando se ele já existe
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive) {
        // Se ele não existir, eu vou criar um archive com o nome passado pela linha de comando
        // Será que por isso que depois que tenho abrir ele novamente para inserir, dá errado ?
        archive = fopen(nome_archive, "wb");
        if (!archive) {
            fclose(entrada);
            return;
        }

        // Vou deixar um espaço inicial de int no meu archive para escrever o número de membros (inicializando com 0)
        int membros = 0;
        fwrite(&membros, sizeof(int), 1, archive);
    }

    // O stat serve para pegar as estatísticas do meu archive (criado ou aberto)
    struct stat st;

    // Aqui estou pegando o nome do arquivo
    if (stat(nome_arquivo, &st) == -1) {
        fclose(entrada);
        fclose(archive);
        return;
    }

    // Aqui estou pegando seu uid, tamanho original e data
    int uid = st.st_uid;
    int tam_original = st.st_size;
    int data = st.st_mtime;

    // Agora, vou alocar um espaço para escrever meu novo membro
    struct membro *novo_membro = (struct membro *)malloc(sizeof(struct membro));
    if (!novo_membro) {
        fclose(entrada);
        fclose(archive);
        return;
    }

    // Vou atribuir valores ao meu novo membro antes de inseri-lo, como:
    // nome, uid, tamanho original e data
    strcpy(novo_membro->nome, nome_arquivo);
    novo_membro->uid = uid;
    novo_membro->tam_original = tam_original;
    novo_membro->data = data;

    // Aqui, estou movendo o cursor para o final do arquivo e com isso, consigo calcular sua posição atual (ftell indica onde está o cursor)
    // O offset do meu membro será a sua posição mais o tamanho de sua estrutura
    // A ordem será simplesmente o tamanho da lista, visto que é sequencial
    fseek(archive, 0, SEEK_END);
    long posicao_atual = ftell(archive);
    novo_membro->offset = posicao_atual + sizeof(struct membro);
    novo_membro->ordem = lista_tamanho(lista_membros);

    // Se a opção de compressão estiver ligada
    if (compressao) {
        // Crio/abro um arquivo .lz temporário
        FILE *arquivo_comprimido = fopen("arquivo_comprimido.lz", "wb");
        if (!arquivo_comprimido) {
            fclose(entrada);
            fclose(archive);
            free(novo_membro);
            return;
        }

        // Aqui, eu vou comprimir meu arquivo, passando sua entrada, arquivo temporário .lz e tamanho, e ele me devolverá o tamanho comprimido
        int tamanho_comprimido;
        if (comprimir_arquivo(entrada, arquivo_comprimido, tam_original, &tamanho_comprimido) < 0) {
            fclose(entrada);
            fclose(arquivo_comprimido);
            fclose(archive);
            free(novo_membro);
            return;
        }

        // Posso fechar meu arquivo temporário comprimido
        fclose(arquivo_comprimido);

        // Verifico se meu arquivo original é maior do que o comprimido. Se for, então ele vai escrever o atual
        if (tamanho_comprimido >= tam_original) {
            printf("Arquivo comprimido maior ou igual ao original. Armazenando o arquivo original.\n");
            // O tamanho será o original e ele não ficará como "comprimido"
            novo_membro->tam_disco = tam_original;
            novo_membro->comprimido = 0;

            // Volto o cursor para o começo do arquivo
            rewind(entrada);

            // Aloco um buffer do tamanho original do meu arquivo
            unsigned char *buffer = (unsigned char *)malloc(tam_original);
            if (!buffer) {
                fclose(entrada);
                fclose(archive);
                free(novo_membro);
                return;
            }

            // Aqui, já atribuio o seu id, que será a posição que ele está na lista
            // Escrevo o a estrutura do membro com fwrite no archive
            // Depois eu coloco no buffer os dados lidos do meu membro
            // Logo em seguida eu escrevo os dados do meu membro no archive com fwrite
            novo_membro->id = lista_tamanho(lista_membros);
            fwrite(novo_membro, sizeof(struct membro), 1, archive);
            size_t lidos = fread(buffer, 1, tam_original, entrada);
            fwrite(buffer, 1, lidos, archive);

            // Libero o buffer, que não será mais utilizado
            free(buffer);
        } else {
            // Se o tamanho do arquivo comprimido for de fato menor do que o original, eu faço exatamente a mesma coisa acima,
            // porém com o arquivo comprimido, utilizando o temporário .lz que criei
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
        // Se a inserção não for de um arquivo comprimido
        // Já consigo atribuir seu tamanho de disco e o status de compressão
        novo_membro->tam_disco = tam_original;
        novo_membro->comprimido = 0;

        // Volto para o começo do meu arquivo
        rewind(entrada);

        // Aloco um buffer do tamanho original do meu arquivo
        unsigned char *buffer = (unsigned char *)malloc(tam_original);
        if (!buffer) {
            fclose(entrada);
            fclose(archive);
            free(novo_membro);
            return;
        }

        // Aqui, já atribuio o seu id, que será a posição que ele está na lista
        // Escrevo o a estrutura do membro com fwrite no archive
        // Depois eu coloco no buffer os dados lidos do meu membro
        // Logo em seguida eu escrevo os dados do meu membro no archive com fwrite
        novo_membro->id = lista_tamanho(lista_membros);
        fwrite(novo_membro, sizeof(struct membro), 1, archive);
        size_t lidos = fread(buffer, 1, tam_original, entrada);
        fwrite(buffer, 1, lidos, archive);
        
        // Libero o buffer, que não será mais utilizado
        free(buffer);
    }

    // Depois da inserção do membro e os seus dados no meu archive, eu atualizo minha lista de membros
    // Insiro o id do membro inserido na última posição da lista, como uma fila
    lista_insere(lista_membros, novo_membro->id, -1);
    // Posso liberar o espaço que foi alocado pelo meu membro inserido
    free(novo_membro);

    // Volto para o começo do arquivo e atualizo o tamanho da lista, inserindo esse dado no começo do meu archive
    // Portanto, no começo do meu archive terá o número de membros e o tamanho da lista (é pra ter isso apenas ? é, é feito na func de inserção)
    rewind(archive);
    int tamanho_lista = lista_tamanho(lista_membros);
    fwrite(&tamanho_lista, sizeof(int), 1, archive);

    // Se houve compressão, eu removo o arquivo comprimido temporário
    if (compressao)
        remove("arquivo_comprimido.lz");

    // Fecho meu arquivo (que foi inserido) e meu archive
    fclose(entrada);
    fclose(archive);
}

void extrair_membro(char *nome_archive, char *nome_arquivo, struct lista_t *lista_membros, int extrair_todos) {
    // Abrindo meu archive
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive)
        return;

    struct membro *primeiro = NULL;

    // Verifica se a lista já foi carregada, caso contrário, carrega a lista de membros
    if (lista_membros->prim == NULL) {
        lista_membros = carregar_membros(archive, lista_membros, &primeiro);
        if (!lista_membros) {
            fclose(archive);
            return;
        }
    }

    struct membro *temp = primeiro;
    while (temp) {
        // Eu vou percorrer todos os membros da minha lista
            // Se não for para extrair todos, e se o nome não corresponder, vai para o próximo membro, ignorando com continue
            if (!extrair_todos && strcmp(temp->nome, nome_arquivo) != 0) {
                temp = temp->prox;
                continue;
            }

            // Caso contrário, cria o arquivo de saída com o nome do membro
            FILE *saida = fopen(temp->nome, "wb");
            if (!saida) {
                free(temp);
                fclose(archive);
                return;
            }

            // Move o cursor para onde está o membro achado
            fseek(archive, temp->offset, SEEK_SET);

            // Se ele for comprimido, eu simplesmente preciso descomprimimi-lo
            if (temp->comprimido) {
                if (descomprimir_arquivo(saida, archive, temp->tam_original, temp->tam_disco) < 0)  {
                    return;
                }
            // Caso ele não for comprimido
            } else {
                // Aloco um buffer do tamanho do arquivo que quero extrair
                unsigned char *buffer = (unsigned char *)malloc(temp->tam_original);
                if (!buffer) {
                    fclose(saida);
                    fclose(archive);
                    free(temp);
                    return;
                }

                // Lê de archive os dados (limitado pelo seu tamanho) do arquivo e armazena no buffer
                size_t lidos = fread(buffer, 1, temp->tam_original, archive);
                if (!lidos) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(temp);
                    return;
                }

                // Escreve na saida o conteúdo do buffer, que são os dados do meu arquivo
                size_t escritos = fwrite(buffer, 1, temp->tam_original, saida);
                if (!escritos) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(temp);
                    return;
                }  

                // Libero o buffer
                free(buffer);
            }

            // Libero o membro e fecho o arquivo de saída
            fclose(saida);

            // Se eu não vou extrair todos, acabo minha função aqui. Se não, eu continuo a extrair os arquivos
            if (!extrair_todos) 
                break;
        // Continuando a extrair os arquivos
        temp = temp->prox;
    }

    // Fecho meu archive
    fclose(archive);
}

int calcula_offset(struct membro *membro) {
    // Se o membro não tiver ponteiro anterior, o offset é o tamanho do membro
    if (!membro->ant) {
        return membro->offset;
    } else {
        // Se o membro tiver ponteiro anterior, o offset é a diferença entre os offsets
        return membro->offset - membro->ant->offset;
    }
}

void remover_membro(char *nome_archive, char *nome_membro, struct lista_t *lista_membros) {
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive) 
        return;

    struct membro *primeiro = NULL;

    lista_membros = carregar_membros(archive, lista_membros, &primeiro);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    struct membro *membro_a_remover = NULL;
    struct membro *tmpo = primeiro;

    while (tmpo) {
        if (strcmp(tmpo->nome, nome_membro) == 0) {
            membro_a_remover = tmpo;
            break;
        }
        tmpo = tmpo->prox;
    }

    if (!membro_a_remover) {
        fclose(archive);
        return;
    }

    rewind(archive);
    
    // Agora eu tenho uma função que calcula o offset de cada membro, vou armazenar eles em "offset_puro"
    tmpo = primeiro;
    while (tmpo) {
        tmpo->offset_puro = calcula_offset(tmpo);
        tmpo = tmpo->prox;
    }   

    // Vou ajeitar primeiramente apenas os ponteiros
    tmpo = primeiro;
    while (tmpo) {
        if (strcmp(tmpo->nome, membro_a_remover->nome) == 0) {    
            if (membro_a_remover->ant && membro_a_remover->prox) {
                // Não é nenhum dos membros extremos
                membro_a_remover->ant->prox = membro_a_remover->prox;
                membro_a_remover->prox->ant = membro_a_remover->ant;
            }
            else if (membro_a_remover->prox && !membro_a_remover->ant) {
                // É o primeiro membro
                primeiro = membro_a_remover->prox;
                primeiro->ant = NULL;
            }
            else if (membro_a_remover->ant && !membro_a_remover->prox) {
                // É o último membro
                membro_a_remover->ant->prox = NULL;
            }
            break;
        }
        tmpo = tmpo->prox;
    }

    // Liberando os ponteiros do membro a remover
    membro_a_remover->prox = NULL;
    membro_a_remover->ant = NULL;

    // Agora vou guardar os offsets originais dos membros
    tmpo = primeiro;
    while (tmpo) {
        tmpo->offset_antigo = tmpo->offset;
        tmpo = tmpo->prox;
    }

    // Volto para o começo do archive
    rewind(archive);

    // Retirando membro removido da lista encadeada
    lista_retira(lista_membros, &membro_a_remover->id, lista_procura(lista_membros, membro_a_remover->id));

    // Escreve o número de membros no início do arquivo
    int tamanho_lista = lista_tamanho(lista_membros);
    fwrite(&tamanho_lista, sizeof(int), 1, archive);

    //int posicao_atual = ftell(archive);

    // Armazeno a posição atual do cursor depois de ter armazenado o tamanho da lista de membros
    long posicao_atual = ftell(archive);
    
    // Sobreescreve todos os membros no archive, menos o removido
    tmpo = primeiro;
    while (tmpo) {
        if (tmpo == membro_a_remover) {
            // Ignora o membro removido
            tmpo = tmpo->prox;
            continue;
        }

        // Atualiza o offset do membro atual
        tmpo->offset = posicao_atual + sizeof(struct membro);

        // Escreve a estrutura do membro
        fwrite(tmpo, sizeof(struct membro), 1, archive);

        // Aloca o buffer do tamanho original do membro
        unsigned char *buffer = (unsigned char *)malloc(tmpo->tam_original);
        if (!buffer) {
            fclose(archive);
            return;
        }

        // Vai até os dados do membro no arquivo original
        fseek(archive, tmpo->offset_antigo, SEEK_SET);

        // Armazena os dados no buffer
        size_t lidos = fread(buffer, 1, tmpo->tam_original, archive);
        if (lidos != (size_t)tmpo->tam_original) {
            free(buffer);
            fclose(archive);
            return;
        }

        // Volta para a posição correta no arquivo para sobrescrever
        fseek(archive, tmpo->offset, SEEK_SET);

        // Escreve os dados do membro no archive
        size_t escritos = fwrite(buffer, 1, lidos, archive);
        if (escritos != lidos) {
            free(buffer);
            fclose(archive);
            return;
        }

        // Atualiza a posição atual no arquivo
        posicao_atual = ftell(archive);

        free(buffer);
        tmpo = tmpo->prox;
    }

    // Trunca o arquivo para remover os resíduos do membro excluído
    ftruncate(fileno(archive), posicao_atual);

    fclose(archive);
}

void desloca_membro(FILE *archive, long inicio, long fim, long deslocamento) {
    // Se o deslocamento for 0, não há nada a fazer
    if (deslocamento == 0) 
        return;

    // Determina a direção do deslocamento
    if (deslocamento > 0) {
        // Deslocamento para frente (aumenta o espaço)
        for (long i = fim; i >= inicio; i--) {
            fseek(archive, i, SEEK_SET);
            int byte = fgetc(archive);
            fseek(archive, i + deslocamento, SEEK_SET);
            fputc(byte, archive);
        }
    } else {
        // Deslocamento para trás (remove espaço)
        for (long i = inicio; i <= fim; i++) {
            fseek(archive, i, SEEK_SET);
            int byte = fgetc(archive);
            fseek(archive, i + deslocamento, SEEK_SET);
            fputc(byte, archive);
        }
    }
}

void mover_membro(char *nome_archive, char *nome_membro, char *nome_target, struct lista_t *lista_membros) {
    // Abro meu archive
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive) 
        return;

    struct membro *primeiro = NULL;

    // Carrego minha lista de membros, assim como os ponteiros entre membros
    lista_membros = carregar_membros(archive, lista_membros, &primeiro);
    if (!lista_membros || lista_membros->tamanho <= 1) {
        fclose(archive);
        return;
    }

    
    // Declaro as variáveis que vou utilizar
    struct membro *tmp = primeiro;
    struct membro *a_mover_membro = NULL;
    struct membro *target_membro = NULL;

    // Enquanto eu tenho membros na lista, vou buscá-los e identificar o membro a mover e o target
    while (tmp) {
        if (strcmp(tmp->nome, nome_membro) == 0) 
            a_mover_membro = tmp;
        if (strcmp(tmp->nome, nome_target) == 0) 
            target_membro = tmp;

        tmp = tmp->prox;
    }

    // Se eu não achar algum dos dois, retorno
    if (!a_mover_membro || !target_membro) {
        fclose(archive);
        return;
    }

    // Se o membro a mover for igual ao target, retorno
    if (a_mover_membro == target_membro) {
        fclose(archive);
        return;
    }

    // Vou calcular os offsets puros dos membros
    tmp = primeiro;
    while (tmp) {
        tmp->offset_puro = calcula_offset(tmp);
        tmp = tmp->prox;
    }

    // Vou guardar os offsets antigos dos membros
    tmp = primeiro;
    while (tmp) {
        tmp->offset_antigo = tmp->offset;
        tmp = tmp->prox;
    }

    // Primeiro vou ajustar os ponteiros dos membros
    tmp = primeiro;
    struct membro *a_mover_membro_ant = a_mover_membro->ant;
    struct membro *a_mover_membro_prox= a_mover_membro->prox; 
    while (tmp) {
         if (tmp == target_membro) {
            // Se o membro for o target, atualizo o ponteiro dele para o membro a mover
            struct membro *antigo_prox = tmp->prox;
            tmp->prox = a_mover_membro;
            a_mover_membro->ant = tmp;
            if (antigo_prox) {
                a_mover_membro->prox = antigo_prox;
                antigo_prox->ant = a_mover_membro;
            }
        }
        // Atualizo o anterior e o próximo antigos do membro a mover
        else if (tmp == a_mover_membro_ant) {
            tmp->prox = a_mover_membro_prox;
        }
        else if (tmp == a_mover_membro_prox) {
            tmp->ant = a_mover_membro_ant;
        }
        tmp = tmp->prox;
    }

    //printf ("Debug para ver como estão os ponteiros:\n");
    //tmp = primeiro;
    //while (tmp) {
    //    printf("Membro: %s, Ant: %s, Prox: %s\n", tmp->nome, tmp->ant ? tmp->ant->nome : "NULL", tmp->prox ? tmp->prox->nome : "NULL");
    //    tmp = tmp->prox;
    //}

    // Agora que eu já tenho os ponteiros ajustados, preciso deixar um espaço no meu archive para colocar meu novo membro
    rewind(archive);


    


    fclose(archive);
}

void listar_conteudo(char *nome_archive, struct lista_t *lista_membros) {
    // Abro meu archive
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive)
        return;

    struct membro *primeiro = NULL;

    // Carrego minha lista de membros, assim como os ponteiros entre membros
    lista_membros = carregar_membros(archive, lista_membros, &primeiro);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    struct membro *temp = primeiro;
    
    // Vou buscar membro a membro. Todo membro que achar, vou printar suas informações
    while (temp) {
        printf("Nome: %s, UID: %d, Tamanho Original: %d, Tamanho Disco: %d, Data: %d, Ordem: %d, Offset: %d\n",
               temp->nome, temp->uid, temp->tam_original, temp->tam_disco, temp->data, temp->ordem, temp->offset);
        // Depois de printar, vou para o próximo membro
        temp = temp->prox;
    }

    // Fecho meu archive
    fclose(archive);
}