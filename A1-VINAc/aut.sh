#!/bin/bash

# Compilar o projeto
make

# Copiar arquivos .txt do diretório rrk24/testes para rrk24
cp rrk24/testes/*.txt rrk24/

# Entrar no diretório rrk24
cd rrk24 || { echo "Erro ao entrar no diretório rrk24"; exit 1; }

# Executar o vinac com os arquivos especificados
./vina -ip arquivos.vc chama.txt soccer.txt testador.txt mouse.txt cachorro.txt

