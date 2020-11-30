#include "stdio.h"
#include "3d.h"

float ball_x;
float ball_x_v;
float ball_y;
float ball_y_v;
float ball_z;
float ball_z_v;
float frontY;
float last_innerY;
float innerY;
float innerYv;
float last_ball_x, last_ball_y, last_ball_z, last_ball_size;
int last_shadow_x;
int score_back;
int score_front;

void displayScore(){
	LCD_FillRect(5, 5, 30, 25, BLACK);
	LCD_SetCursor(5, 5);
	LCD_SetTextColor(YELLOW, BLACK);
	LCD_Printf("%i\r\n", score_back);
	LCD_FillRect(5, 55, 30, 25, BLACK);
	LCD_SetCursor(5, 55);
	LCD_SetTextColor(RED, BLACK);
	LCD_Printf("%i\r\n", score_front);
}
void init3D(){
	LCD_SetRotation(3);
	ball_z = 0.5;
	ball_z_v = 0.04;
	ball_x = 0.1;
	ball_x_v = 0.018;
	ball_y = 0.5;
	ball_y_v = 0.015;
	frontY = 0.0;
	last_innerY = 0.0;
	innerY = 0.0;
	innerYv = 0.01;
	last_ball_x = 0.0;
	last_ball_y = 0.0;
	last_ball_z = 0.0;
	last_ball_size = 0.0;
	last_shadow_x = 0.0;
	score_back = 0;
	score_front = 0;
	displayScore();
	drawOuterFrame();
	drawInnerFrame();
	drawPerspectiveLinesBottom();
	drawPerspectiveLinesTop();
}
// Draw the outer region (should not be overdrawn)
void drawOuterFrame(void){
	LCD_FillScreen(backgroundColour);
	LCD_FillRect(0, 0, screenPadding, H, BLACK);
	LCD_FillRect(screenPadding + outerFrameSize, 0, screenPadding, H, BLACK);
	LCD_FillRect(screenPadding, 0, outerFrameBorder, H, WHITE); // LEFT
	LCD_FillRect(screenPadding,
		H - outerFrameBorder,
	  W - 2*screenPadding - outerFrameBorder, outerFrameSize, WHITE); // BOTTOM
	LCD_FillRect(outerFrameSize + screenPadding - outerFrameBorder, 0,
	  outerFrameBorder, H, WHITE); // RIGHT
	LCD_FillRect(screenPadding, 0,
	  W - 2*screenPadding - outerFrameBorder, outerFrameBorder, WHITE); // TOP
}
void drawInnerFrame(){
	LCD_FillRect(innerFrameMinX, innerFrameMinY,
		innerFrameBorder, innerFrameSize, WHITE); // LEFT
	LCD_FillRect(innerFrameMinX, innerFrameMinY + innerFrameSize,
	  innerFrameSize + innerFrameBorder, innerFrameBorder, WHITE); // BOTTOM
	LCD_FillRect(innerFrameMinX + innerFrameSize, innerFrameMinY,
	  innerFrameBorder, innerFrameSize, WHITE); // RIGHT
	LCD_FillRect(innerFrameMinX, innerFrameMinY,
	  innerFrameSize + innerFrameBorder, innerFrameBorder, WHITE); // TOP
}
void drawPerspectiveLinesBottom(){
	// bottom left perspective
	LCD_DrawLine(screenPadding, H,
		innerFrameMinX, innerFrameMinY + innerFrameSize + innerFrameBorder, WHITE);
	LCD_DrawLine(screenPadding + 5, H,
		innerFrameMinX, innerFrameMinY + innerFrameSize + innerFrameBorder, WHITE);
	LCD_DrawLine(screenPadding, H - 5,
		innerFrameMinX, innerFrameMinY + innerFrameSize + innerFrameBorder, WHITE);
	// bottom right perspective
	LCD_DrawLine(screenPadding + outerFrameSize, H,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameSize, WHITE);
	LCD_DrawLine(screenPadding + outerFrameSize - 5, H,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameSize, WHITE);
	LCD_DrawLine(screenPadding + outerFrameSize, H - 5,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameSize, WHITE);
}
void drawPerspectiveLinesTop(){
	// top left perspective
	LCD_DrawLine(screenPadding, 0, innerFrameMinX, innerFrameMinY, WHITE);
	LCD_DrawLine(screenPadding, 5, innerFrameMinX, innerFrameMinY + 1, WHITE);
	LCD_DrawLine(screenPadding + 5, 0, innerFrameMinX + 1, innerFrameMinY, WHITE);
	// top right perspective
	LCD_DrawLine(screenPadding + outerFrameSize, 0,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameBorder, WHITE);
	LCD_DrawLine(screenPadding + outerFrameSize - 5, 0,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameBorder, WHITE);
	LCD_DrawLine(screenPadding + outerFrameSize, 5,
		innerFrameMinX + innerFrameSize, innerFrameMinY + innerFrameBorder, WHITE);
}

void drawBackPong(float y, int h){
	uint16_t yellow5 = LCD_Color565(138, 133, 110);
	LCD_FillRect(innerFrameMinX + innerFrameBorder,
		(int) (last_innerY * innerFrameSize) + innerFrameMinY + innerFrameBorder,
		innerFrameSize - innerFrameBorder, h, backgroundColour);
	LCD_FillRect(innerFrameMinX + innerFrameBorder,
		(int) (y * innerFrameSize) + innerFrameMinY + innerFrameBorder,
		innerFrameSize - innerFrameBorder, h, yellow5);
}
void drawFrontPong(float y, int h){
	uint16_t red5 = LCD_Color565(153, 43, 95);
	LCD_FillRect(screenPadding + outerFrameBorder,
		(int) (frontY * outerFrameSize) + outerFrameBorder,
		outerFrameSize - (2*outerFrameBorder), h, red5);
}
void drawInnerFrameWithPong(float y, int h){
	uint16_t white_red = LCD_Color565(255, 136, 136);
	drawInnerFrame();
	drawFrontPong(frontY, h);
	int ymin = (int) (frontY * outerFrameSize) + outerFrameBorder;
	int ymax = ymin + h;
	int x1 = innerFrameMinX; int y1 = innerFrameMinY; // left
	int x2 = innerFrameMinX; int y2 = innerFrameMinY + innerFrameSize; // bottom
	int x3 = innerFrameMinX + innerFrameSize; int y3 = innerFrameMinY; // right
	int x4 = innerFrameMinX; int y4 = innerFrameMinY; // top
	// Redraw bottom
	if (y2 > ymin && y2 < ymax)
		LCD_FillRect(x2, y2, innerFrameSize + innerFrameBorder, innerFrameBorder, white_red);
	// Redraw top
	if (y4 > ymin && y4 < ymax)
		LCD_FillRect(x4, y4, innerFrameSize + innerFrameBorder, innerFrameBorder, white_red);
	// Redraw hidden pices of left and right sides
	if (ymax > innerFrameMinY && ymin < innerFrameMinY + innerFrameSize){
		int rectMinY = ymin < innerFrameMinY ? innerFrameMinY : ymin;
		int rectMaxY = ymax > innerFrameMinY + innerFrameSize ? innerFrameMinY + innerFrameSize : ymax;
		LCD_FillRect(innerFrameMinX, rectMinY, innerFrameBorder, rectMaxY - rectMinY, white_red); 
		LCD_FillRect(innerFrameMinX + innerFrameSize, rectMinY, innerFrameBorder, rectMaxY - rectMinY, white_red); 
	}
}
void drawBall(){
	uint16_t shadowColor = LCD_Color565(37, 46, 102);
	static int shadowDepth = 50;
	int minShadowY = 175;
	// erase
	LCD_FillCircle(last_ball_x, last_ball_y, last_ball_size, backgroundColour);
	LCD_FillCircle(last_shadow_x, minShadowY + (shadowDepth * last_ball_z), last_ball_size/1.5, backgroundColour);
	// draw the ball
	uint16_t hidden_green = LCD_Color565(114, 141, 18);
	int ballSize = (ballMaxRadius - ballMinRadius) * ball_z;
	ballSize = ballSize < ballMinRadius ? ballMinRadius : ballSize;
	int apparentFrameSize = innerFrameSize + (outerFrameSize - innerFrameSize) * ball_z;
	int relativeX = ball_x * apparentFrameSize;
	int relativeY = ball_y * apparentFrameSize;
	int paddingX = ((outerFrameSize - apparentFrameSize)/2) + screenPadding;
	int paddingY = ((outerFrameSize - apparentFrameSize)/2);
	int apparentX = (relativeX + paddingX)*0.9 + 5;
	int apparentY = (relativeY + paddingY)*0.9 + 5;
	// draw shadow before the ball
	LCD_FillCircle(apparentX, minShadowY + (shadowDepth * ball_z), ballSize/2, shadowColor);
	if (ball_y > frontY && ball_y < frontY + 0.35)
		LCD_FillCircle(apparentX, apparentY, ballSize, hidden_green);
	else
		LCD_FillCircle(apparentX, apparentY, ballSize, GREEN);
	// register current position
	last_ball_x = apparentX;
	last_ball_y = apparentY;
	last_ball_z = ball_z;
	last_ball_size = ballSize;
	last_shadow_x = apparentX;
}
void redrawBorder(){
	if (ball_x < 0.2){
		LCD_FillRect(0, 0, screenPadding, H, BLACK);
		LCD_FillRect(screenPadding, 0, outerFrameBorder, H, WHITE);
		displayScore();
	}
	else if (ball_x > 0.8){
		LCD_FillRect(screenPadding + outerFrameSize, 0, screenPadding, H, BLACK);
		LCD_FillRect(outerFrameSize + screenPadding - outerFrameBorder, 0, outerFrameBorder, H, WHITE);
	}
	if (ball_y < 0.2)
		LCD_FillRect(screenPadding, 0, W - 2*screenPadding - outerFrameBorder, outerFrameBorder, WHITE);
	else if (ball_y > 0.8 || ball_z > 0.8)
		LCD_FillRect(screenPadding, H - outerFrameBorder, W - 2*screenPadding - outerFrameBorder, outerFrameSize, WHITE);
}
float normalizeDistance3D(float d){
	if (d < 2.0)
		d = 2.0;
	if (d > 30.0)
		d = 30.0;
	return ((d - 2.0) / 30.0);
}
void moveInnerPongAuto(){
	last_innerY = innerY;
	float rng = ((float)rand() / (float)RAND_MAX);
	if (rng > 0.95){
		rng = ((float)rand() / (float)RAND_MAX);
		if (rng > 0.55){
			innerYv = -0.02;
			if (innerY < 0.2)
				innerYv = 0.02;
		}
		else if (rng > 0.1){
			innerYv = 0.02;
			if (innerY > 0.65)
				innerYv = -0.02;
		}
		else
			innerYv = 0;
	}
	innerY += innerYv;
	if (innerY < 0.0){
		innerY = 0.0;
		innerYv *= -1;
	}
	else if (innerY > 0.75){
		innerY = 0.75;
		innerYv *= -1;
	}
}
// BBRAINZ  - Cyber Imagination
void update3D(float d){
	float normalisedD = normalizeDistance3D(d);
	// LOGIC
	ball_z += ball_z_v;
	// score
	if (ball_z > 1 && !(ball_y > frontY && ball_y < frontY + 0.35)){
		score_back++;
		displayScore();
	}
	if (ball_z < 0 && !(ball_y > innerY && ball_y < innerY + 0.35)){
		score_front++;
		displayScore();
	}
	if (ball_z > 1 || ball_z < 0)
		ball_z_v *= -1;
	ball_x += ball_x_v;
	if (ball_x > 1 || ball_x < 0)
		ball_x_v *= -1;
	ball_y += ball_y_v;
	if (ball_y > 1 || ball_y < 0)
		ball_y_v *= -1;
	moveInnerPongAuto();
	// DISPLAY
	drawBackPong(innerY, 25);
	drawPerspectiveLinesBottom();
	drawPerspectiveLinesTop();
	// measurements
	float oldY = frontY;
	frontY = normalisedD;
	int ypx = frontY * (float)outerFrameSize;
	if (ypx + frontPongHeight > outerFrameSize + outerFrameBorder)
		ypx = outerFrameSize - frontPongHeight - 2*outerFrameBorder;
	frontY = (float)ypx/(float)outerFrameSize;
	// Erase last frame
	LCD_FillRect(screenPadding + outerFrameBorder, oldY*outerFrameSize + outerFrameBorder - 1,
		outerFrameSize - (2*outerFrameBorder), frontPongHeight + 1, backgroundColour);
	// Draw new frame
	drawInnerFrameWithPong(frontY, frontPongHeight);
	// ball
	drawBall();
	redrawBorder();
}
float getYAuto(void){
	static float pongRelativeHeight = (float)frontPongHeight/(float)outerFrameSize;
	static float borderRelativeHeight = (float)outerFrameBorder/(float)outerFrameSize;
	static float d = 0.0;
	static int descend = 1;
	if (descend)
		d += 0.01;
	else
		d -= 0.01;
	if (d >= 1 - pongRelativeHeight - borderRelativeHeight*2)
		descend = 0;
	if (d <= 0)
		descend = 1;
	return d;
}
int mode3DmustExit(){
	if (score_back < maxPoint && score_front < maxPoint)
		return 0;
	if (score_back >= maxPoint)
		return -1;
	if (score_front >= maxPoint)
		return 1;
}
int getPlayerScore3D(){
	return score_front;
}
int getComputerScore3D(){
	return score_back;
}
