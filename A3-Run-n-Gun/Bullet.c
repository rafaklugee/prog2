#include <stdlib.h>
#include "Bullet.h"

bullet* bullet_create(unsigned short x, unsigned short y, unsigned char trajectory, bullet *next){		

	if ((trajectory < 0) || (trajectory > 2)) return NULL;													

	bullet *new_bullet = (bullet*) malloc(sizeof(bullet));													
	if (!new_bullet) return NULL;																			
	new_bullet->x = x;																						
	new_bullet->y = y;																						
	new_bullet->trajectory = trajectory;																	
	new_bullet->distance_traveled = 0;
	new_bullet->next = (struct bullet*) next;																
	return new_bullet;																						
}

void bullet_move(bullet *elements){																			

	for (bullet *index = elements; index != NULL; index = (bullet*) index->next){	
		if (index->trajectory == BULLET_TRAJ_LEFT) index->x = index->x - BULLET_MOVE; // Se a trajetória for para a esquerda, movimenta um passo à esquerda
		else if (index->trajectory == BULLET_TRAJ_RIGHT) index->x = index->x + BULLET_MOVE;	// Se a trajetória for para a direita, movimenta um passo à direita
		else if (index->trajectory == BULLET_TRAJ_UP) index->y = index->y - BULLET_MOVE; // Se a trajetória for para a direita, movimenta um passo à cima
	}
}

void bullet_destroy(bullet *element){
	free(element);
}