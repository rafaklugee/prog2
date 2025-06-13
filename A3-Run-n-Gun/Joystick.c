#include <stdlib.h>
#include "Joystick.h"

joystick* joystick_create(){

	joystick *element = (joystick*) malloc (sizeof(joystick));
	if (!element) 
		return NULL;
	element->right = 0;
	element->left = 0;
	element->fire = 0;
	return element;
}

void joystick_destroy(joystick *element){ free(element);}

void joystick_left(joystick *element){ element->left = element->left ^ 1;}

void joystick_right(joystick *element){ element->right = element->right ^ 1;}

void joystick_fire(joystick *element){ element->fire = element->fire ^ 1;}