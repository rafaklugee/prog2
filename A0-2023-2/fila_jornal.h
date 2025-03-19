#ifndef __FILA_JORNAL__
#define __FILA_JORNAL__

typedef struct noticia {
	char titulo[33];
	char texto[513];
	char idade;

	struct noticia *prox;
} noticia;

typedef struct fila_jornal {
	noticia *inicio;
	noticia *final;
} fila_jornal;

#endif
