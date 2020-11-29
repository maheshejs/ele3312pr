#include "MCUFRIEND_kbv.h"

/**
* Title screen and default mode of the application
* 
* "init3D" takes no argument.
* "update3D" takes the argument "distance", which is a length in centimeters
* from the sensor.
*
* To go to a new mode, call function "getMode", which will return an enumeration
* that indicating the targeted mode. This function is intended to be called when
* a button is pressed.
*
* Example:
*
* int main(){
*     ...
*     initTitle();
*     ...
*     while(1){
*         ... // Aquire distance
*         updateTitle(distance);
*         HAL_Delay(25);
*         ...
*     }
* }
*/

#define W 320
#define H 240
#define borderSize 10

enum mode {
	DeuxContreDeux,
	UnJoueur,
	Badminton,
	Mode3D,
	Title};

void drawBorder();
void drawDots();
void initTitle(void);
void updateTitle(float distance);
enum mode getMode();
