#ifndef INICIO_H
#define INICIO_H
#include <stdio.h>
#include <stdlib.h>

struct membro {
    char nome[50];
    int uid;
    int tam_original;
    int tam_disco;
    int data;
    int ordem;
    int offset;
    struct membro *prox;
}





#endif