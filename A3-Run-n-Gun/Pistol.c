#include <stdlib.h>
#include "Pistol.h"

pistol* pistol_create(){																				
	pistol *new_pistol = (pistol*) malloc(sizeof(pistol));												
	if (!new_pistol) return NULL;																		
	new_pistol->timer = 0; //Inicializa o relógio de disparos em zero (pronto para atirar)
	new_pistol->shots = NULL; //Inicializa a lista de disparos com NULL; ou seja, sem disparos
	return new_pistol;																					
}

bullet* pistol_shot(unsigned short x, unsigned short y, unsigned char trajectory, pistol *gun){
	bullet *new_bullet = bullet_create(x, y, trajectory, gun->shots); //Cria uma nova instância de projétil a ser disparado
	if (!new_bullet) return NULL;																		
	return new_bullet;																					
}

void pistol_destroy(pistol *element){																	
	bullet *sentinel;														
	for (bullet *index = element->shots; index != NULL; index = sentinel){								
		sentinel = (bullet*) index->next;													
		bullet_destroy(index);																			
	}
	free(element);																			
}