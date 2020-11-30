/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM4
#define CONTROLLER 1
#include "arm_math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "MCUFRIEND_kbv.h"
#include "title.h"
#include "badminton.h"
#include "3d.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define MAXHEIGHT 240
#define MAXWIDTH 320
#define FONT 10
#define PONGSIZE 50
#define SPACE 20
#define RBALL 5
#define PONGSPEED 5
#define VYSPEED 5.0

#define LIMITUP (MAXHEIGHT - FONT)
#define LIMITL  (MAXWIDTH - FONT)

extern volatile uint32_t pulse_width;

#define TX_BUFFER_SIZE 2 * (1 + CONTROLLER)
#define RX_BUFFER_SIZE 2 * (2 - CONTROLLER)

uint8_t transferBuffer[TX_BUFFER_SIZE]; 
uint8_t receiveBuffer[RX_BUFFER_SIZE]; 

volatile bool flagTransmitted = false;
volatile bool flagReceived = false;
volatile uint32_t local_time = 0;

uint8_t score_g = 0;
uint8_t score_d = 0;
bool score_g_changed = true;
bool score_d_changed = true;

// STATE MACHINE
enum mode currentMode = Title;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
enum Tools { BALL, PONG};
enum CollisionType {UDWALL,LRWALL,PLAYER1,PLAYER2,NONE};
struct Tool
{
	float xPos;
	float yPos;
	float xSpeed;
	float ySpeed;
	enum Tools mType;
};

struct Tool mBall,mPade1,mPade2;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

void SetLCD(void)
{
	LCD_Begin();
	HAL_Delay(20);
	LCD_SetRotation(0);
//	LCD_InvertDisplay(false);
	LCD_FillScreen(BLACK);
}

void BorderGame(void)
{
	LCD_FillRect(0, 0, MAXHEIGHT, FONT, WHITE); //LEFT
	LCD_FillRect(0, 0, FONT, MAXWIDTH, WHITE);  //DOWN
	LCD_FillRect(MAXHEIGHT-FONT, 0, FONT, MAXWIDTH, WHITE); //UP
	LCD_FillRect(0, MAXWIDTH-FONT, MAXHEIGHT, FONT, WHITE); //RIGHT
}

int16_t cMax(int16_t a, int16_t b)
{
	return (a<b)?b:a;
}

int16_t cMin(int16_t a, int16_t b)
{
	return (a>b)?b:a;
}

int BallRectangle (int16_t Cx,int16_t Cy,int16_t Rx, int16_t Ry, int16_t RectWidth, int16_t RectHeight) 
{
	int16_t DeltaX = Cx - cMax(Rx, cMin(Cx, Rx + RectWidth));
	int16_t DeltaY = Cy - cMax(Ry, cMin(Cy, Ry + RectHeight));
	
	return (DeltaX * DeltaX + DeltaY * DeltaY) < (RBALL * RBALL);
}

#if CONTROLLER
enum CollisionType CollisionBall()
{
	if(BallRectangle(mBall.xPos + mBall.xSpeed,mBall.yPos + mBall.ySpeed,MAXHEIGHT-FONT, 0, FONT, MAXWIDTH) ||
		BallRectangle(mBall.xPos+ mBall.xSpeed,mBall.yPos + mBall.ySpeed,0, 0, FONT, MAXWIDTH))
			return UDWALL;
	
	if(BallRectangle(mBall.xPos + mBall.xSpeed,mBall.yPos + mBall.ySpeed,0,MAXWIDTH-FONT,MAXHEIGHT,FONT) ||
		BallRectangle(mBall.xPos + mBall.xSpeed,mBall.yPos + mBall.ySpeed,0, 0, MAXHEIGHT, FONT))
			return LRWALL;
	
	if(BallRectangle(mBall.xPos + mBall.xSpeed, mBall.yPos + mBall.ySpeed,mPade2.xPos,mPade2.yPos,PONGSIZE,FONT))
		return PLAYER2;

	if(BallRectangle(mBall.xPos + mBall.xSpeed,mBall.yPos + mBall.ySpeed,mPade1.xPos,mPade1.yPos,PONGSIZE,FONT))
		return PLAYER1;
	
	return NONE;
}
#endif
int CollisionPong(struct Tool mPade)
{
	if(((mPade.xPos + mPade.xSpeed + PONGSIZE) > LIMITUP) || 
			((mPade.xPos + mPade.xSpeed) < FONT ))
		return 1;
	return 0;
}

void InitGame()
{
	mBall.xPos = MAXHEIGHT/2;
	mBall.yPos = MAXWIDTH/2;
	mBall.xSpeed = 1;
	mBall.ySpeed = 1;
	
	mPade1.xPos = MAXHEIGHT/2-PONGSIZE/2;
	mPade1.yPos = SPACE;
	mPade1.xSpeed = 0;
	mPade1.ySpeed = 0;
	     
	mPade2.xPos = MAXHEIGHT/2-PONGSIZE/2;
	mPade2.yPos = MAXWIDTH - SPACE - FONT;
	mPade2.xSpeed = 0;
	mPade2.ySpeed = 0;	
}

#if CONTROLLER
void MoveBall()
{
	LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL+2,BLACK);
	switch (CollisionBall())
	{
		case UDWALL:
								BorderGame();
								mBall.xSpeed*=-1;
								break;
		case LRWALL:
								BorderGame();
								mBall.ySpeed*=-1;
								// Hitting the top means that left player scored.
								if (mBall.yPos < 25){
									score_g++;
									score_g_changed = true;
								}
								// The ball hitting the bottom means the right one scored.
								else if (mBall.yPos > 200){
									score_d++;
									score_d_changed = true;
								}
								// Put the ball back into the middle.
								mBall.yPos = MAXWIDTH / 2;
								break;
		case PLAYER2:
								mBall.ySpeed *= -1;
								mBall.xSpeed  = (((mPade2.xPos + PONGSIZE/2.0) - mBall.xPos)/(PONGSIZE/2.0))*-VYSPEED; 
								break;
		case PLAYER1:
								mBall.ySpeed *= -1;
								mBall.xSpeed  = (((mPade1.xPos + PONGSIZE/2.0) - mBall.xPos)/(PONGSIZE/2.0))*-VYSPEED; 
								break;
		default:
								break;
	}
	
	mBall.xPos += mBall.xSpeed;
	mBall.yPos += mBall.ySpeed;
	LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,WHITE);
}
#endif

struct Tool MovePong(struct Tool mPong)
{
	if(!CollisionPong(mPong))
	{
		LCD_FillRect(mPong.xPos,mPong.yPos,PONGSIZE,FONT,BLACK);
		mPong.xPos += mPong.xSpeed;
		mPong.yPos += mPong.ySpeed;
		LCD_FillRect(mPong.xPos,mPong.yPos,PONGSIZE,FONT,WHITE);
	}
	return mPong;
}

void drawNet(){
	LCD_FillRect(FONT, 320/2 - FONT/2, 240-(2*FONT), FONT, BLACK);
	for (int i = 0; i < 12; i += 2){
		LCD_FillRect(FONT + (i*2*FONT), 320/2 - FONT/2, FONT*2, FONT, WHITE);
	}
}
void placePongWithDistance(struct Tool* mPong, float distance){
	// Remove 2 cm because it is too close to the sensor, normalise with 30 cm
	// to define a working range and scale to screen width (240 - 20 px).
	if (distance < 2.0)
		distance = 2.0;
	float normalisedDistance = ((distance - 2.0) / 30.0) * (float)(MAXHEIGHT - (2*FONT));
	//  Check if the position is too large.
	if (normalisedDistance + PONGSIZE > MAXHEIGHT - (2*FONT))
		normalisedDistance = MAXHEIGHT - PONGSIZE - (2*FONT);
	// Place pong
	mPong->xPos = normalisedDistance + FONT;
}
void displayDefaite(uint8_t score_g, uint8_t score_d)
{
		LCD_FillScreen(LCD_Color565(140,0,0));
		LCD_SetCursor(30, 30);
		LCD_SetTextColor(BLACK,LCD_Color565(140,0,0));
		LCD_SetTextSize(4);
		LCD_Printf("X DEFAITE X\r\n");
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, 255 + FONT, FONT/2, WHITE);
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35, 120, 255, 80, LCD_Color565(0,17,114));
		LCD_FillRect(35 + 255, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35 - FONT/2, 120 + 80, 255 + FONT, FONT/2, WHITE);
		
		LCD_SetCursor(35 + FONT, 120 + FONT);
		LCD_SetTextColor(WHITE, LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf(" Score Final:\r\n");
		
		LCD_SetCursor(90 + FONT, 160 + FONT);
		LCD_SetTextColor(LCD_Color565(255,0,0), LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf("%i\r\n", score_g);

		LCD_SetCursor(190 + FONT, 160 + FONT);
		LCD_SetTextColor(LCD_Color565(255,224,71), LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf("%i\r\n", score_d);
}

void displayVictoire(uint8_t score_g, uint8_t score_d)
{
		//FONT
		LCD_FillScreen(GREENYELLOW);
		LCD_SetCursor(30, 30);
		LCD_SetTextColor(WHITE, GREENYELLOW);
		LCD_SetTextSize(4);
		LCD_Printf("!! BRAVO !!\r\n");
		LCD_FillRect(35, 120, 255, 80, LCD_Color565(0,17,114));
	
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, 255 + FONT, FONT/2, WHITE);
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35, 120, 255, 80, LCD_Color565(0,17,114));
		LCD_FillRect(35 + 255, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35 - FONT/2, 120 + 80, 255 + FONT, FONT/2, WHITE);
		
		LCD_SetCursor(35 + FONT, 120 + FONT);
		LCD_SetTextColor(WHITE, LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf(" Score Final:\r\n");
		
		LCD_SetCursor(90 + FONT, 160 + FONT);
		LCD_SetTextColor(LCD_Color565(255,0,0), LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf("%i\r\n", score_g);

		LCD_SetCursor(190 + FONT, 160 + FONT);
		LCD_SetTextColor(LCD_Color565(255,224,71), LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf("%i\r\n", score_d);
}
void displayFin(uint8_t score)
{
		//FONT
		LCD_FillScreen(LCD_Color565(0, 155, 255));
		LCD_SetCursor(30, 30);
		LCD_SetTextColor(LCD_Color565(255,0,0), LCD_Color565(0, 155, 255));
		LCD_SetTextSize(4);
	  LCD_Printf(" ): FIN :(\r\n");
		LCD_FillRect(35, 120, 255, 80, LCD_Color565(0,17,114));
	
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, 255 + FONT, FONT/2, WHITE);
		LCD_FillRect(35 - FONT/2, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35, 120, 255, 80, LCD_Color565(0,17,114));
		LCD_FillRect(35 + 255, 120 - FONT/2, FONT/2, 80 + FONT, WHITE);
		LCD_FillRect(35 - FONT/2, 120 + 80, 255 + FONT, FONT/2, WHITE);
		
		LCD_SetCursor(35 + FONT, 120 + FONT);
		LCD_SetTextColor(WHITE, LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf(" Score Final:\r\n");
		
		LCD_SetCursor(130 + FONT, 160 + FONT);
		LCD_SetTextColor(LCD_Color565(255,0,0), LCD_Color565(0,17,114));
		LCD_SetTextSize(3);
		LCD_Printf("%i\r\n", score);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(1000);
	SetLCD();
	InitGame();
	BorderGame();	
	HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);
	HAL_UART_Receive_DMA(&huart3, receiveBuffer, RX_BUFFER_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//init3D();
    		const char* intro[13] = {"ELE3312 - Project", "", "PONG-19", "", 
			"Modern pong game", "Futuristic look", "", "Developed by : ", "", "Joseph Maheshe", "Vincent Therrien",
			"", "December 3, 2020"
		};
		
		int16_t ht = 16, top = 3, line = 0, lines = 13, scroll=0;
		LCD_FillScreen(LCD_Color565(140,0,0));
		LCD_SetTextSize(2);
		LCD_SetTextColor(WHITE, LCD_Color565(140,0,0));
		LCD_SetRotation(0);
		while(line < lines)
		{
		LCD_SetCursor(0, (scroll + top) * ht);
    LCD_VertScroll(top * ht, lines * ht, (++scroll) * ht);
		LCD_Printf(intro[line]);
    HAL_Delay(1000);
    line++;
		}
		LCD_SetTextSize(3);
	initTitle();
	//drawBackPong(0.3, 25);
	//drawInnerFrameWithPong(0.65, 35);
	//HAL_Delay(10000);
	
	float distance = 0;
	//MoveBall();
	//LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,WHITE);
	//LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,WHITE);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (currentMode == DeuxContreDeux){
		// Display the net only if the ball is near it.
		if (mBall.yPos > (MAXWIDTH/2) - (2*FONT)
				&& mBall.yPos < (MAXWIDTH/2) + (2*FONT))
			drawNet();
		// Display scores  either if they changed or if they are
		// erase by the motion of the ball.
		if ((mBall.yPos > 50 && mBall.yPos < 110
				&& mBall.xPos > 20 && mBall.xPos < 100)
				|| score_d_changed){
			LCD_FillRect(50, 80, 130, 25, BLACK);
			LCD_SetCursor(50, 80);
			LCD_Printf("%i\r\n", score_d); // Player 1
		}
		if ((mBall.yPos > 210 && mBall.yPos < 270
				&& mBall.xPos > 20 && mBall.xPos < 100)
				|| score_g_changed){
			LCD_FillRect(50, 240, 130, 25, BLACK);
			LCD_SetCursor(50, 240);
			LCD_Printf("%i\r\n", score_g); // Player 2
		}
		score_g_changed = false;
		score_d_changed = false;
		// Measure distance
		distance = ((float)pulse_width)/58.0;
#if CONTROLLER
		LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,BLACK);
		placePongWithDistance(&mPade1, distance);
		LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,WHITE);
#else
		LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,BLACK);
		placePongWithDistance(&mPade2, distance);
		LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,WHITE);
		transferBuffer[0] = 253;
		transferBuffer[1] = (uint8_t) (mPade2.xPos);
		HAL_UART_Transmit_DMA(&huart3, transferBuffer, TX_BUFFER_SIZE);	
#endif
	local_time = 0;
	while (!flagReceived)
	{
		if (local_time == 50)
			break;
	}
	if (flagReceived)
	{
				
#if CONTROLLER
		if (receiveBuffer[0] == 253)
		{
			LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,BLACK);
			mPade2.xPos = receiveBuffer[1];
			LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,WHITE);
			flagReceived = 0;
		}
#else
		if (receiveBuffer[0] == 254)
		{
				LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,BLACK);
				mPade1.xPos = receiveBuffer[3];
				LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,WHITE);

				LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,BLACK);
				mBall.xPos = receiveBuffer[1];
				mBall.yPos = receiveBuffer[2]*2;
				LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,WHITE);
		}		
#endif
		flagReceived = false;
	}

#if CONTROLLER
		MoveBall();
		transferBuffer[0] = 254;
		transferBuffer[1] = (uint8_t) (mBall.xPos);
		transferBuffer[2] = (uint8_t) (mBall.yPos/2);
		transferBuffer[3] = (uint8_t)	mPade1.xPos;
		HAL_UART_Transmit_DMA(&huart3, transferBuffer, TX_BUFFER_SIZE);
#endif
		
		
		//mPade2 = MovePong(mPade2);		
		//MoveBall();
	
		// Control pong one (i.e. right):
		//LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,BLACK);
		//placePongWithDistance(&mPade1, distance);
		//LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,WHITE);
		
		//emulate player two (i.e. left):
		//mPade2.xSpeed = (rand()%5-2);
		if (score_g >= 3 || score_d >= 3){
			currentMode = Title;
			initTitle();
		}
	}

		
		distance = ((float)pulse_width)/58.0;
		// Title screen
		if (currentMode == Title){
			updateTitle(distance);
			if (HAL_GPIO_ReadPin(interrupteur_GPIO_Port, interrupteur_Pin) == RESET){
				currentMode = getMode();
				if (currentMode == DeuxContreDeux){
					SetLCD();
					BorderGame();
					InitGame();
				}
				if (currentMode == Badminton)
					initBadminton();
				if (currentMode == Mode3D)
					init3D();
			}
		}
		// Badminton
		if (currentMode == Badminton){
			updateBadminton(distance);
			if (exitB()){
				displayScoreBad();
				HAL_Delay(3000);
				currentMode = Title;
				initTitle();
			}
		}
		// 3D
		if (currentMode == Mode3D){
			update3D(distance);
			if (mode3DmustExit() == -1){
				LCD_FillScreen(RED);
				HAL_Delay(3000);
				currentMode = Title;
				initTitle();
			}
			else if (mode3DmustExit() == 1){
				LCD_FillScreen(GREEN);
				HAL_Delay(3000);
				currentMode = Title;
				initTitle();
			}
		}
		
		HAL_Delay(25);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief Retargets the C library printf function to the USART.
 * @param None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART2 and Loop until the end
	of transmission */
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		flagReceived = false;
		HAL_UART_Receive_DMA(&huart3, receiveBuffer, RX_BUFFER_SIZE);
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART3)
		flagReceived = true;
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART3)
		flagTransmitted = true;
}

void HAL_SYSTICK_Callback(void) {
	local_time++;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN){
	if (GPIO_PIN == GPIO_PIN_13){
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
		else
			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
