#!/bin/bash

# Caminho do diretório onde está o script
DIR_SCRIPT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Copiar todos os arquivos .txt da pasta "testes" para o diretório do script
cp "$DIR_SCRIPT/testes"/*.txt "$DIR_SCRIPT"

# Executar make
make

# Executar o programa com os argumentos desejados
./vinac -ic arquivos.vc chama.txt soccer.txt testador.txt

# Remover os arquivos .txt copiados
rm -f "$DIR_SCRIPT"/*.txt
