// ...

/* USER CODE BEGIN PV */
// STATE MACHINE
enum mode currentMode = Title;
/* USER CODE END PV */

// ...

int main(){
    // ...

    /* USER CODE BEGIN WHILE */
    initTitle();
    while (1)
    {
        /* USER CODE END WHILE */

        // Transmissions UART...

        /* USER CODE BEGIN 3 */
        distance = ((float)pulse_width)/58.0;
		// Title screen
		if (currentMode == Title){
			updateTitle(distance);
			if (HAL_GPIO_ReadPin(interrupteur_GPIO_Port, interrupteur_Pin) == RESET){
				currentMode = getMode();
				if (currentMode == Mode3D)
					init3D();
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
        /* USER END BEGIN 3 */
        
        // ...
}

// ...
