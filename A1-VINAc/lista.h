#ifndef LISTA
#define LISTA

struct item_t {
  int valor;
  struct item_t *ant;
  struct item_t *prox;
};

struct lista_t {
  struct item_t *prim;
  struct item_t *ult;
  int tamanho;
};

struct lista_t *lista_cria ();

struct lista_t *lista_destroi (struct lista_t *lst);

int lista_insere (struct lista_t *lst, int item, int pos);

int lista_retira (struct lista_t *lst, int *item, int pos);

int lista_consulta (struct lista_t *lst, int *item, int pos);

int lista_procura (struct lista_t *lst, int valor);

int lista_tamanho (struct lista_t *lst);

void lista_imprime (struct lista_t *lst);

#endif
