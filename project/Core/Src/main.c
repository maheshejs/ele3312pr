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
#define MASTER_GAME 1 
#include "arm_math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "MCUFRIEND_kbv.h"
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
volatile uint32_t local_time = 0;
volatile uint8_t transmit_done = 0;
volatile uint8_t receive_done = 0;

#define TX_DATA_SIZE 2 * (1 + MASTER_GAME)
#define RX_DATA_SIZE 2 * (2 - MASTER_GAME)

uint8_t tx_data[TX_DATA_SIZE]; 
uint8_t rx_data[RX_DATA_SIZE]; 
uint8_t tab_received[1]; 
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
struct Player
{
	uint8_t score;
};

struct Tool mBall,mPade1,mPade2;
struct Player mPlayer1, mPlayer2;
void gay(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

//unsigned int conversion(unsigned int a, unsigned int b)
//{
//	unsigned int r0 = (a << 16) >> 16;
//	unsigned int r1 = a >> 16 ;
//	unsigned int r2 = (b << 16) >> 16;
//	unsigned int r3 = b >> 16 ;
//	unsigned int result = (r0 * r2) + ( (r1*r2 + r0*r3) << 16 );
//	
//	
//}

void SetLCD(void)
{
	LCD_Begin();
	HAL_Delay(20);
	LCD_SetRotation(0);
	LCD_InvertDisplay(true);
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

#if MASTER_GAME
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

#if MASTER_GAME
void MoveBall()
{

	switch (CollisionBall())
	{
		case UDWALL:
								mBall.xSpeed*=-1;
								break;
		case LRWALL:
								mBall.ySpeed*=-1;
								//TODO J'AI PERDU
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
	LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,BLACK);
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
  /* USER CODE BEGIN 2 */
		HAL_Delay(1000);
		SetLCD();
		InitGame();
		BorderGame();	
		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
		HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
		HAL_UART_Receive_DMA(&huart3, rx_data, RX_DATA_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		LCD_FillRect(100, 160, 130, 25, BLACK);
		LCD_SetCursor(100, 160);
		LCD_Printf("%i\r\n", (pulse_width)/58);
#if MASTER_GAME
		mPade1 = MovePong(mPade1);
#else
		mPade2 = MovePong(mPade2);
		tx_data[0] = 253;
		tx_data[1] = (uint8_t) (mPade2.xPos);
		HAL_UART_Transmit_DMA(&huart3, tx_data, TX_DATA_SIZE);	
#endif
	local_time = 0;
	while (receive_done == 0)
	{
		if (local_time == 50)
			break;
	}
	if (receive_done == 1)
	{
				
#if MASTER_GAME
		if (rx_data[0] == 253)
		{
			LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,BLACK);
			mPade2.xPos = rx_data[1];
			LCD_FillRect(mPade2.xPos,mPade2.yPos,PONGSIZE,FONT,WHITE);
			receive_done = 0;
		}
#else
		if (rx_data[0] == 254)
		{
				LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,BLACK);
				mPade1.xPos = rx_data[3];
				LCD_FillRect(mPade1.xPos,mPade1.yPos,PONGSIZE,FONT,WHITE);

				LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,BLACK);
				mBall.xPos = rx_data[1];
				mBall.yPos = rx_data[2]*2;
				LCD_FillCircle(mBall.xPos, mBall.yPos,RBALL,WHITE);
		}		
#endif
		receive_done = 0;
	}

#if MASTER_GAME
		MoveBall();
		tx_data[0] = 254;
		tx_data[1] = (uint8_t) (mBall.xPos);
		tx_data[2] = (uint8_t) (mBall.yPos/2);
		tx_data[3] = (uint8_t)	mPade1.xPos;
		HAL_UART_Transmit_DMA(&huart3, tx_data, TX_DATA_SIZE);
#endif
	
#if MASTER_GAME
		//emulate player one :
		mPade1.xSpeed = (rand()%5-2);
#else		
		//emulate player two :
		mPade2.xSpeed = (rand()%5-2);
#endif
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

void HAL_SYSTICK_Callback(void) {
	local_time++;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN)
{
	if (GPIO_PIN == GPIO_PIN_13)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
		{
			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);
		}
		else
		{
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
		}
			
		
	}
		
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		receive_done = 0;
		HAL_UART_Receive_DMA(&huart3, rx_data, RX_DATA_SIZE);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		receive_done = 1;
		printf("Receive done \r\n");
	}	
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		transmit_done = 1;
		printf("Transmit done \r\n");
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
