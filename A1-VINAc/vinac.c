#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
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
struct lista_t *carregar_membros(FILE *arquivo, struct lista_t *lista_membros) {
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

    struct membro *anterior = NULL;

    for (int i = 0; i < qtd_membros; i++) {
        struct membro *m = malloc(sizeof(struct membro));
        if (!m) 
            return NULL;

        // Lê os membros a cada struct membro
        if (fread(m, sizeof(struct membro), 1, arquivo) != 1) {
            free(m);
            return NULL;
        }

        // A cada leitura, atualiza os ponteiros dos membros que são lidos
        // anterior <- m -> NULL
        m->ant = anterior;
        m->prox = NULL;

        // Se há algum membro anterior, atualiza o ponteiro do membro anterior
        if (anterior)
            // anterior <-> m -> NULL
            anterior->prox = m;

        // Insere na lista_membros o id de cada membro
        if (!lista_insere(lista_membros, m->id, -1)) {
            free(m);
            return NULL;
        }

        // Pula os dados do membro que já foi lido no arquivo
        if (fseek(arquivo, m->tam_disco, SEEK_CUR) != 0) {
            free(m);
            return NULL;
        }

        // Agora sim, o membro atual virou o anterior (não é mais a primeira iteração)
        anterior = m;
        free(m);
    }

    // Retorna a lista de membros com o id de cada um e os membros com seus ponteiros ant e prox corretos
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

    // Verifica se o membro já existe no archive
    // Criando uma lista temporária
    struct lista_t *temp_lista = lista_cria();
    // Abrindo temporariamente meu archive
    FILE *archive_temp = fopen(nome_archive, "rb");
    if (archive_temp) {
        // Carregando meus membros do archive e inserindo eles na lista
        carregar_membros(archive_temp, temp_lista);
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
        // Eu vou percorrer todos os membros da minha lista
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            // Se não for para extrair todos, e se o nome não corresponder, vai para o próximo membro, ignorando com continue
            if (!extrair_todos && strcmp(m->nome, nome_arquivo) != 0) {
                free(m);
                temp = temp->prox;
                continue;
            }

            // Caso contrário, cria o arquivo de saída com o nome do membro
            FILE *saida = fopen(m->nome, "wb");
            if (!saida) {
                free(m);
                fclose(archive);
                return;
            }

            // Move o cursor para onde o membro achado começa
            fseek(archive, m->offset, SEEK_SET);

            // Se ele for comprimido, eu simplesmente preciso descomprimimi-lo
            if (m->comprimido) {
                if (descomprimir_arquivo(saida, archive, m->tam_original, m->tam_disco) < 0) 
                    return;
            // Caso ele não for comprimido
            } else {
                // Aloco um buffer do tamanho do arquivo que quero extrair
                unsigned char *buffer = (unsigned char *)malloc(m->tam_original);
                if (!buffer) {
                    fclose(saida);
                    fclose(archive);
                    free(m);
                    return;
                }

                // Lê de archive os dados (limitado pelo seu tamanho) do arquivo e armazena no buffer
                size_t lidos = fread(buffer, 1, m->tam_original, archive);
                if (lidos != (size_t)m->tam_original) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(m);
                    return;
                }

                // Escreve na saida o conteúdo do buffer, que são os dados do meu arquivo
                size_t escritos = fwrite(buffer, 1, m->tam_original, saida);
                if (escritos != (size_t)m->tam_original) {
                    fclose(archive);
                    fclose(saida);
                    free(buffer);
                    free(m);
                    return;
                }  

                // Libero o buffer
                free(buffer);
            }

            // Libero o membro e fecho o arquivo de saída
            free(m);
            fclose(saida);

            // Se eu não vou extrair todos, acabo minha função aqui. Se não, eu continuo a extrair os arquivos
            if (!extrair_todos) 
                break;
        }
        // Continuando a extrair os arquivos
        temp = temp->prox;
    }

    // Fecho meu archive
    fclose(archive);
}

void remover_membro(char *nome_archive, char *nome_membro, struct lista_t *lista_membros) {
    // Abrindo meu archive
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive)
        return;

    // Carrego minha lista de membros, assim como os ponteiros entre membros
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    // Localiza o membro a ser removido
    struct membro *membro_a_remover = NULL;
    struct item_t *temp = lista_membros->prim;
    while (temp) {
        // Vou buscar o membro a ser removido, se ele corresponder, será armazenado na variável membro_a_remover
        // Se encontrar, sai do laço while, caso contrário, continua até achar
        struct membro *m = busca_membro(temp->valor, archive);
        if (m && strcmp(m->nome, nome_membro) == 0) {
            membro_a_remover = m;
            break;
        }
        free(m);
        temp = temp->prox;
    }

    // Se não achou o membro a remover, retorna
    if (!membro_a_remover) {
        fclose(archive);
        return;
    }

    // Cria um binário temporário
    FILE *temp_archive = fopen("temp_archive.bin", "wb");
    if (!temp_archive) {
        free(membro_a_remover);
        fclose(archive);
        return;
    }

    // Volta no começo de archive
    rewind(archive);

    // Armazeno em membros a quantidade de membros que tenho
    int membros;
    fread(&membros, sizeof(int), 1, archive);
    // Armazeno em novos_membro a quantidade que eu terei após a remoção
    int novos_membros = membros - 1;
    fwrite(&novos_membros, sizeof(int), 1, temp_archive);

    // Aloco um espaço de memória para um membro
    struct membro *m = (struct membro *)malloc(sizeof(struct membro));
    if (!m) {
        fclose(temp_archive);
        fclose(archive);
        free(membro_a_remover);
        return;
    }

    for (int i = 0; i < membros; i++) {
        // Armazeno em m um membro de archive e vou lendo struct a struct
        fread(m, sizeof(struct membro), 1, archive);

        // Quando eu achar o membro que quero remover no archive, eu pulo para o próximo
        // membro com continue
        if (strcmp(m->nome, membro_a_remover->nome) == 0) {
            // Pula os dados do membro a ser removido
            fseek(archive, m->tam_disco, SEEK_CUR);
            continue;
        }

        // Se não for o membro que quero remover
        // O novo offset do membro vai ser a posição do cursor + o tamanho da struct membro
        m->offset = ftell(temp_archive) + sizeof(struct membro);
        // Vou escrever em temp_archive meu membro (struct membro), agora com um novo offset
        fwrite(m, sizeof(struct membro), 1, temp_archive);

        // Alocando um buffer do tamanho do membro atual
        unsigned char *buffer = (unsigned char *)malloc(m->tam_disco);
        if (!buffer) {
            fclose(temp_archive);
            fclose(archive);
            free(m);
            free(membro_a_remover);
            return;
        }

        // Vou armazenar em um buffer o tamanho do membro lido
        size_t lidos = fread(buffer, 1, m->tam_disco, archive);
        // Vou escrever em archive os dados do meu novo membro
        fwrite(buffer, 1, lidos, temp_archive);

        free(buffer);
    }

    // Libero minha struct utilizada para iterar os membros
    free(m);

    // Fecho o archive e meu binário temporário
    fclose(archive);
    fclose(temp_archive);

    // Removo o meu archive e renomeio meu binário modificado com seu nome
    // Pode estar aqui o erro será ?
    remove(nome_archive);
    rename("temp_archive.bin", nome_archive);

    // Retiro o id do meu membro removido da lista de membros
    lista_retira(lista_membros, &membro_a_remover->id, lista_procura(lista_membros, membro_a_remover->id));
    // Não tenho que atualizar a ordem aqui também ?

    // Libero meu membro removido
    free(membro_a_remover);
}

void mover_membro(char *nome_archive, char *nome_membro, char *nome_target, struct lista_t *lista_membros) {
    // Abro meu archive
    FILE *archive = fopen(nome_archive, "r+b");
    if (!archive) 
        return;

    // Carrego minha lista de membros, assim como os ponteiros entre membros
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros || lista_membros->tamanho <= 1) {
        fclose(archive);
        return;
    }

    // Declaro as variáveis que vou utilizar
    struct item_t *item = lista_membros->prim;
    struct item_t *a_mover_item = NULL;
    struct item_t *target_item = NULL;
    struct membro *a_mover_membro = NULL;
    struct membro *target_membro = NULL;

    // Enquanto eu tenho membros na lista, vou buscá-los e identificar o membro a mover e o target
    while (item) {
        struct membro *m = busca_membro(item->valor, archive);
        if (strcmp(m->nome, nome_membro) == 0)
            a_mover_membro = m;
            a_mover_item = item;
        if (strcmp(m->nome, nome_target) == 0)
            target_membro = m;
            target_item = item;
        free(m);
        item = item->prox;
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

    // Agora, eu vou localizar meus membros intermediários, que ficam entre o membro a mover e o target e criar uma lista temporária com eles
    // Basta eu começar minha busca com o offset do target e ir até o offset do membro a mover
    struct item_t *temp = target_item;
    struct lista_t *temp_lista = lista_cria();
    while (temp && temp != a_mover_item) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            // Insiro o id do membro na lista temporária
            lista_insere(temp_lista, m->id, -1);
            free(m);
        }
        temp = temp->prox;
    }

    // Debug
    printf ("Esses são os membros intermediários:\n");
    // Imprimo os membros intermediários
    struct item_t *temp2 = temp_lista->prim;
    while (temp2) {
        struct membro *m = busca_membro(temp2->valor, archive);
        if (m) {
            printf("[INTERMEDIARIO]Nome: %s, UID: %d, Tamanho Original: %d, Tamanho Disco: %d, Data: %d, Ordem: %d, Offset: %d\n",
                   m->nome, m->uid, m->tam_original, m->tam_disco, m->data, m->ordem, m->offset);
            free(m);
        }
        temp2 = temp2->prox;
    }
    
    // Agora, vou armazenar meu membro a mover em uma variável temporária
    struct membro *membro_a_mover_copia = a_mover_membro;
    // Mais importante, vou armazenar o tamanho do meu membro que será movido
    int tam_membro_a_mover = a_mover_membro->tam_disco;

    // Vou remover o membro da dinâmica de ponteiros que ele está inserido
    // A função remove já ajeita os ponteiros... (vamos supor)
    remover_membro(nome_archive, a_mover_membro->nome, lista_membros);



    
}

void listar_conteudo(char *nome_archive, struct lista_t *lista_membros) {
    // Abro meu archive
    FILE *archive = fopen(nome_archive, "rb");
    if (!archive)
        return;

    // Carrego minha lista de membros, assim como os ponteiros entre membros
    lista_membros = carregar_membros(archive, lista_membros);
    if (!lista_membros) {
        fclose(archive);
        return;
    }

    struct item_t *temp = lista_membros->prim;
    
    // Vou buscar membro a membro. Todo membro que achar, vou printar suas informações
    while (temp) {
        struct membro *m = busca_membro(temp->valor, archive);
        if (m) {
            printf("Nome: %s, UID: %d, Tamanho Original: %d, Tamanho Disco: %d, Data: %d, Ordem: %d, Offset: %d\n",
                   m->nome, m->uid, m->tam_original, m->tam_disco, m->data, m->ordem, m->offset);
            free(m);
        }
        // Depois de printar, vou para o próximo membro
        temp = temp->prox;
    }

    // Fecho meu archive
    fclose(archive);
}