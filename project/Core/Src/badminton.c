#include "badminton.h"
#include "ball.h"

float ball_x_bad;
float ball_y_bad;
float pade_x;
float pade_y;
float speed_x;
float speed_y;
float pade_speed;
int score;
uint8_t score_changed;


void initBadminton(void)
{
    ball_x_bad = MAXWIDTH/2;
    ball_y_bad = MAXHEIGHT/2;
    speed_x = 0;
    speed_y = 1;
    pade_x = MAXWIDTH/2-PONGSIZEB/2;
    pade_y = MAXHEIGHT - SPACEB - FONTB;
    pade_speed = 0;
		score = 0;
    
    drawFrame();
	displayScoreBad();
}

void updateBadminton(float d)
{
    LCD_FillRect(pade_x,pade_y, PONGSIZEB, FONTB, LCD_Color565(0,17,114));
    movePong(d);
    LCD_FillRect(pade_x,pade_y, PONGSIZEB, FONTB, LCD_Color565(255,0,0));
		if (RectangleBall(ball_x_bad, ball_y_bad, FONTB+5, FONTB+5, MAXWIDTH - 2*FONTB - 5, 25) || score_changed == 1)
		{
			displayScoreBad();
			score_changed = 0;
		}
    LCD_FillCircle(ball_x_bad, ball_y_bad,RBALLB + 2, LCD_Color565(0,17,114));
    moveBall();
	  LCD_FillCircle(ball_x_bad, ball_y_bad,RBALLB, LCD_Color565(0,255,33));
//    struct ball_s ball = {ball_x_bad, ball_y_bad, RBALLB, LCD_Color565(0,255,33)};
//    draw_ball_3D(&ball);
}

void drawFrame(void)
{
		LCD_FillScreen(LCD_Color565(0,17,114));
    LCD_FillRect(MAXWIDTH - FONTB, 0, FONTB,  MAXHEIGHT, WHITE); //RIGHT
    LCD_FillRect(0, 0, MAXWIDTH, FONTB, WHITE); //UP
    LCD_FillRect(0, MAXHEIGHT-FONTB, MAXWIDTH, FONTB,  WHITE); //DOWN
    LCD_FillRect(0, 0, FONTB, MAXHEIGHT, WHITE); //LEFT
}

void displayScoreBad(void)
{
    LCD_FillRect(FONTB+5, FONTB+5, MAXWIDTH - 2*FONTB -5, 25, LCD_Color565(0,17,114));
    LCD_SetCursor(FONTB+5, FONTB+5);
    LCD_SetTextColor(WHITE, LCD_Color565(0,17,114));
    LCD_Printf("Score : %i\r\n", score);
}

void movePong(float distance)
{
    if (distance < 2.0) distance = 2.0;
    float normalisedDistance = ((distance - 2.0) / 30.0) * (float)(MAXWIDTH - (2*FONTB));
    if (normalisedDistance + PONGSIZEB > MAXWIDTH - (2*FONTB)) normalisedDistance = MAXWIDTH - PONGSIZEB - (2*FONTB);
    pade_x = normalisedDistance + FONTB;
}

void moveBall(void)
{
        if (RectangleBall(ball_x_bad + speed_x,ball_y_bad + speed_y, 0, 0, MAXWIDTH, FONTB))
        {
						speed_y*=-1;
        }
        else if (RectangleBall(ball_x_bad + speed_x,ball_y_bad + speed_y, 0, 0, FONTB, MAXHEIGHT) ||
        RectangleBall(ball_x_bad+ speed_x,ball_y_bad + speed_y, MAXWIDTH - FONTB, 0, FONTB,  MAXHEIGHT))
        {
					 speed_x *=-1;
        }
        else if (RectangleBall(ball_x_bad + speed_x,ball_y_bad + speed_y, pade_x, pade_y,  PONGSIZEB, FONTB))
        {
            score++;
						score_changed = 1;
						speed_y *= -1;
            speed_x  = (((pade_x + PONGSIZEB/2.0) - ball_x_bad)/(PONGSIZEB/2.0))*-VYSPEED;
        }
        ball_x_bad += speed_x;
        ball_y_bad += speed_y;
}

int16_t gMax(int16_t a, int16_t b)
{
    return (a<b)?b:a;
}

int16_t gMin(int16_t a, int16_t b)
{
    return (a>b)?b:a;
}

int RectangleBall(int16_t Cx,int16_t Cy,int16_t Rx, int16_t Ry, int16_t RectWidth, int16_t RectHeight) 
{
    int16_t DeltaX = Cx - gMax(Rx, gMin(Cx, Rx + RectWidth));
    int16_t DeltaY = Cy - gMax(Ry, gMin(Cy, Ry + RectHeight));
    return (DeltaX * DeltaX + DeltaY * DeltaY) < ((RBALLB + 2) * (RBALLB + 2));
}

int exitB(void)
{
	return RectangleBall(ball_x_bad+ speed_x,ball_y_bad + speed_y,0, MAXHEIGHT-FONTB, MAXWIDTH, FONTB) ; 
}
int getScoreBadminton(void)
{
	return score;
}

