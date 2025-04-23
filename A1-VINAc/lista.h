#ifndef __LISTA_H__
#define __LISTA_H__

struct item_t {
  void *valor; // Alterado de int para void *
  struct item_t *ant;	
  struct item_t *prox;
};

// estrutura de uma lista
struct lista_t {
  struct item_t *prim;
  struct item_t *ult;
  int tamanho;
};

struct lista_t *lista_cria();

struct lista_t *lista_destroi(struct lista_t *lst);

int lista_insere(struct lista_t *lst, void *item, int pos);

int lista_retira(struct lista_t *lst, void **item, int pos);

int lista_consulta(struct lista_t *lst, void **item, int pos);

int lista_procura(struct lista_t *lst, void *valor);

int lista_tamanho(struct lista_t *lst);

void lista_imprime(struct lista_t *lst);

#endif
