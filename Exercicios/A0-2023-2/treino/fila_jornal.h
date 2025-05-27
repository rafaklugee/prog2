#ifndef __FILA_JORNAL__
#define __FILA_JORNAL__

struct noticia {
    char titulo[32];
    char texto[512];
    int idade;

    struct noticia *prox;
};

struct fila_jornal {
    struct noticia *inicio;
    struct noticia *final;
};

struct noticia *criar_noticia (struct noticia *nova); // OK
void atualizar_fila (struct fila_jornal *fila); // OK
void destruir_fila (struct fila_jornal *fila);
void inserir_noticia (struct noticia *nova, struct fila_jornal *fila); // OK
void remover_noticia (struct fila_jornal *fila); // OK

#endif