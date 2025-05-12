Nome: Rafael Ribeiro Kluge
GRR: 20244439

# Lista de Arquivos
vinac.h: 
    - Definição da estrutura de dados das funções que manipulam o arquivador.
        - struct membro
    - Prototipação das funções que manipulam o arquivador.
        - inserir_membro
        - extrair_membro
        - mover_membro
        - remover_membro
        - listar_conteudo
vinac.c:
    - Implementação das funções que manipulam o arquivador.
        - inserir_membro
        - extrair_membro
        - mover_membro
        - remover_membro
        - listar_conteudo
        
    - Implementação de funções auxiliares para manipulação de membros do arquivador.
        - busca_membro
        - carregar_membros
        - comprimir_arquivo
        - descomprimir_arquivo
lista.h:
    - Essa lista foi criada na matéria de Programação 1
    - Definição da estrutura de dados para a lista encadeada.
        - struct lista_t
        - struct item_t
    - Prototipação das funções que manipulam a lista encadeada.
        - lista_cria
        - lista_destroi
        - lista_insere
        - lista_retira
        - lista_consulta
        - lista_procura
        - lista_tamanho
        - lista_imprime
    - Algumas funções auxiliares da lista foram utilizadas apenas para debugar o código.
lista.c:
    - Implementação das funções que manipulam a lista encadeada.
        - lista_cria
        - lista_destroi
        - lista_insere
        - lista_retira
        - lista_consulta
        - lista_procura
        - lista_tamanho
        - lista_imprime
lz.h: 
    - Prototipação das funções que manipulam um arquivo compactado.
    - Essa biblioteca foi fornecida pelo professor.
        - LZ_Compress
        - LZ_Uncompress
lz.c:
    - Implementação das funções que manipulam o arquivo compactado.
    - Essa biblioteca foi fornecida pelo professor.
        - LZ_Compress
        - LZ_Uncompress  
main.c:
    - Implementação da função principal para manipualção de um arquivador.
    - Função principal que controla o fluxo do programa.
    - Leitura dos argumentos da linha de comando.
    - Inicialização do arquivador e chamada das funções apropriadas pelo usuário.
makefile:
    - Script que compila todos os arquivos necessários para o funcionamento do arquivador.
    - Comando "make all"
         - Compila todos os arquivos.
         - Cria o executável "vina" dentro de login/.
    - Comando "make clean"
         - Remove os arquivos objeto, executável e ".vc". 
README.md:
    - Descrição do programa e instruções de uso.

# Descrição de algoritmos, estruturas de dados utilizadas e alternativas de implementação
- O programa, utilizando a biblioteca lista.h, implementa uma lista encadeada para armazenar 
    os IDs dos arquivos que estão dentro do arquivador.

- A estrutura de dados da lista (struct lista_t) é composta por tamanho e seus itens (struct item_t)
    que armazenam o valor (ID) de um membro e ponteiros para o anterior e próximo item da lista.

- A escolha por uma lista que guarda os IDs dos arquivos facilita a manipulação, pois apenas com
    ID, nós conseguimos identifcar os membros a partir da função auxiliar busca_membro, o que tira
    a necessidade de armazenar o membro inteiro na lista. Essa alternativa facilitou a implementação
    do meu código e o deixou mais eficiente.

- Propriamente, a biblioteca vinac.h possui a estrutura de dados de cada membro (struct membro) que
    armazena o ID, o nome do arquivo, tamanho do arquivo e outras informações necessárias para
    manipulação do arquivador.

- No arquivo principal do programa, chamado vinac.c, o usuário pode inserir, extrair, mover,
    remover e listar os membros do arquivador. Para cada uma dessas operações, há uma função
    correspondente que manipula a lista encadeada e a estrutura de dados dos membros.

- O programa utiliza a biblioteca lz.h para realizar a compressão e descompressão dos arquivos, garantindo
    que os dados sejam manipulados de forma comprimida. Essa biblioteca foi fornecida pelo professor e é 
    utilizada nas funções de compressão e descompressão dos arquivos.

- Ao inserirmos um ou mais arquivos no arquivador, o programa armazena como nome exatamente o que o usuário digita.
    Ou seja, se o arquivo estiver em outro diretório, como por exemplo, /testes/teste.txt, o programa armazenará o
    nome do arquivo dessa forma. Se o arquivo estiver no mesmo diretório, não há problemas.

- Ao extrairmos um ou mais membros, se os arquivos originais estiverem presentes no mesmo diretório do executável
    "vina", eles não serão sobreescritos, aparecerá que os arquivos já existem. Caso queiramos testar a extração,
    seria importante remover os arquivos originais do diretório presente logo após a inserção.

- Uma dificuldade encontrada, foi a função mover_membro. Pois para implementá-la, é necessário muitas mudanças de
    posição dentro do archive, buscar da lista posições e criar um arquivos temporários. Por ser muito complexa,
    ela possui alguns problemas, que serão citados logo em seguida.

# Bugs conhecidos
- A função de inserção consegue lidar com apenas uma linha de comando. Por exemplo, se eu insiro vários arquivos
    em um .vc no mesmo comando, a sua manipulação e outras operações sobre ele funcionam corretamente. Porém, ao
    fazer a inserção em um arquivo .vc que já existe, ele é desconfigurado e perde suas propriedades.

- Isso também vale para quando eu vou inserir um arquivo já existente dentro de um .vc. Ele substitui o arquivo corretamente,
    mas como inserimos outro membro dentro de um .vc já existente, sua manipulação fica comprometida após a ação.
 
- A função mover_membro move seus membros corretamente, podemos ver isso após movermos um membro e exibirmos com -c.
    Porém, ela deixa o archive desconfigurado, comprometendo sua manipulação. Por exemplo, se formos extrair os 
    arquivos de um archive após membros terem sido movidos, eles virão desconfigurados. Porém, se formos listar
    seu conteúdo, será feito com sucesso