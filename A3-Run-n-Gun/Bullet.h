#ifndef __BULLET__ 																																
#define __BULLET__																																

// Distância que a bala se move a cada atualização
#define BULLET_MOVE 12

#define BULLET_TRAJ_LEFT   0
#define BULLET_TRAJ_RIGHT  1
#define BULLET_TRAJ_UP     2

typedef struct {																																
	unsigned short x; 
	unsigned short y; 
	unsigned short distance_traveled;
	unsigned char trajectory; // Trajetória da bala: 0 = esq, 1 = dir, 2 = cima
	struct bullet *next;
} bullet;																																		

bullet* bullet_create(unsigned short x, unsigned short y, unsigned char trajectory, bullet *next);												
void bullet_move(bullet *elements);																												
void bullet_destroy(bullet *element);																											

#endif																																			