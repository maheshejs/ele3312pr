#include "title.h"

enum mode targetMode;
uint16_t backgroundColour;

void drawBorder(){
	LCD_FillRect(0, 0, borderSize, H, WHITE);
	LCD_FillRect(0, H - borderSize, W, borderSize, WHITE);
	LCD_FillRect(W - borderSize, 0, borderSize, H, WHITE);
	LCD_FillRect(0, 0, W, borderSize, WHITE);
}

void initTitle(void){
	backgroundColour = LCD_Color565(0, 17, 114);
	LCD_SetRotation(3);
	LCD_FillScreen(backgroundColour);
	drawBorder();
	// Titre
	LCD_SetCursor(50, 30);
	LCD_SetTextColor(WHITE, backgroundColour);
	LCD_Printf("P O N G - 1 9");
	LCD_FillRect(30, 60, W-60, 4, WHITE);
	// Nom des options
	LCD_SetCursor(50, (0 * 40) + 80);
	LCD_Printf("2 CONTRE 2");
	LCD_SetCursor(50, (1 * 40) + 80);
	LCD_Printf("UN JOUEUR");
	LCD_SetCursor(50, (2 * 40) + 80);
	LCD_Printf("BADMINTON");
	LCD_SetCursor(50, (3 * 40) + 80);
	LCD_Printf("3 D");
	targetMode = DeuxContreDeux;
	drawDots();
}
void drawDots(){
	for (int i = 0; i < 4; i++){
		LCD_FillCircle(30, (i * 40) + 90, 10, backgroundColour);
		if (i == targetMode){
			LCD_FillCircle(30, (i * 40) + 90, 10, LIGHTGREY);
			LCD_FillCircle(30, (i * 40) + 90, 5, BLACK);
		}
		else
			LCD_FillCircle(30, (i * 40) + 90, 5, LIGHTGREY);
	}
}
void updateTitle(float distance){
	enum mode newMode = 0;
	if (distance < 5)
		newMode = Mode3D;
	else if (distance < 15)
		newMode = Badminton;
	else if (distance < 30)
		newMode = UnJoueur;
	else
		newMode = DeuxContreDeux;
	if (newMode != targetMode){
		targetMode = newMode;
		drawDots();
	}
}

enum mode getMode(){
	return targetMode;
}
