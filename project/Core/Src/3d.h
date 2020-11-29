#include "MCUFRIEND_kbv.h"
#include "stdlib.h"

/**
* HOWTO
* To use this mode, call the function "init3D" first, and then function
* "update3D" repeatedly in the event loop. Add a delay of 25 milliseconds
* between calls.
*
* "init3D" takes no argument.
* "update3D" takes the argument "distance", which is a length in centimeters
* from the sensor.
*
* Example:
*
* int main(){
*     ...
*     init3D();
*     ...
*     while(1){
*         ... // Aquire distance
*         update3D(distance);
*         HAL_Delay(25);
*         ...
*     }
* }
*/

#define W 320
#define H 240
#define backgroundColour 0x4296
#define outerFrameBorder 10
#define outerFrameSize 240
#define innerFrameBorder 5
#define innerFrameSize 100
#define screenPadding 40
#define innerFrameMinX 110
#define innerFrameMinY 70
#define frontPongHeight 50
#define ballMaxRadius 18
#define ballMinRadius 5
#define maxPoint 5

void init3D();
void drawOuterFrame(void);
void drawInnerFrame(void);
void drawPerspectiveLinesBottom(void);
void drawPerspectiveLinesTop(void);
void drawBackPong(float y, int h);
void drawFrontPong(float y, int h);
void drawInnerFrameWithPong(float y, int h);
void drawBall();
void redrawBorder();
void displayScore();
float normalizeDistance3D(float d);
void moveInnerPongAuto();
void update3D(float d);
float getYAuto(void);

// Return 0: Must not exit
// Return -1: Lost
// Return 1: Won
int mode3DmustExit();
