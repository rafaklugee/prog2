#ifndef __PISTOL__ 																												
#define __PISTOL__																												

#include "Bullet.h"

// Tempo entre um tiro e outro
#define PISTOL_COOLDOWN 7

typedef struct {																												
	unsigned char timer; //Relógio de intervalo entre tiros
	bullet *shots; //Lista de balas ativas no campo disparadas pelas arma	
} pistol;																														

pistol* pistol_create();																										
bullet* pistol_shot(unsigned short x, unsigned short y, unsigned char trajectory, pistol *gun);									
void pistol_destroy(pistol *element);																							

#endif																															