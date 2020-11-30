#ifndef BADMINTON_H
#define BADMINTON_H

#include "MCUFRIEND_kbv.h"
#include "stdlib.h"

#define MAXHEIGHT 240
#define MAXWIDTH 320
#define FONTB 10
#define PONGSIZEB 80
#define SPACEB 10
#define RBALLB 5
#define PONGSPEED 5
#define VYSPEED 5.0

int exitB(void);
int getScoreBadminton(void);
void initBadminton(void);
void updateBadminton(float d);
void drawFrame(void);
void moveBall(void);
void movePong(float distance);
void displayScoreBad(void); 
int RectangleBall(int16_t Cx,int16_t Cy,int16_t Rx, int16_t Ry, int16_t RectWidth, int16_t RectHeight);

#endif
