# PROGRAMAÇÃO 2

Repositório com os principais trabalhos desenvolvidos na disciplina de Programação 2, com destaque para um compactador de arquivos criado do zero e um jogo estilo run-n-gun em C.

<!-- 📸 Substitua o link abaixo por uma imagem ou GIF mostrando a aplicação em uso -->
<p align="center">
  <img src="./A3-Run-n-Gun/gameplay.gif" alt="Demonstração da gameplay do A3" width="800">
</p>

## Sobre o projeto

Este repositório reúne os trabalhos práticos (A0 a A3) e exercícios da disciplina de Programação 2, todos implementados em **C**. Os destaques são:

- **A1-VINAc** — compactador de arquivos criado do zero, inspirado em formatos como `.zip` e `.tgz`.
- **A3-Run-n-Gun** — jogo estilo run-n-gun desenvolvido com a biblioteca **Allegro**.

## Estrutura do projeto

```
prog2/
├── A0-Jukebox-Mercenaria/    # trabalho relacionado a fila de prioridades de uma jukebox
├── A1-VINAc/                 # compactador de arquivos estilo .zip
├── A2-Modificacao-VINAc/     # extensão/modificação do compactador
├── A3-Run-n-Gun/             # jogo run-n-gun em C com Allegro
└── Exercicios/               # exercícios da disciplina
```

## Destaques

### A1-VINAc — Compactador de arquivos

Implementação própria de um compactador/descompactador de arquivos, cobrindo os conceitos de codificação e agrupamento de dados por trás de formatos como `.zip` e `.tgz`.

```bash
cd A1-VINAc
# siga as instruções de compilação/execução específicas do diretório
```

### A3-Run-n-Gun — Jogo em C com Allegro

Jogo estilo run-n-gun, com movimentação, tiros e inimigos, desenvolvido em C utilizando a biblioteca gráfica Allegro.

```bash
cd A3-Run-n-Gun
sudo apt update
sudo apt install liballegro5-dev
sudo apt install liballegro5-dev liballegro-acodec5-dev
make
./game
```

## Tecnologias utilizadas

| Camada | Tecnologia |
|---|---|
| Linguagem | C |
| Biblioteca gráfica (A3) | Allegro |
