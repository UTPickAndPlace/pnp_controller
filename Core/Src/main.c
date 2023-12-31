/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "command.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t rx_data;
uint8_t rx_buffer[BUFFER_SIZE][16];
uint8_t buffer_index;

uint8_t ready[6] = "ready";
char* MCUHANDSHAKE = "Handshake Complete\n";
char* ready_for_new_command = "Waiting on New Command...\n";
uint8_t PCH_flag =0;
uint8_t END_flag =0;
uint8_t t_code[16];
uint8_t t_code_index;
uint8_t start_flag =0;

uint8_t uart_buffer[BUFFER_SIZE];
volatile uint8_t read_ptr = 0;
volatile uint8_t write_ptr = 0;
char* echo_command;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//rudimentary delay
void delay() {
	for(int i = 0; i < 1000000; i++);
}

//checks if the last 6 bits in the buffer are ready?
uint8_t checkPCH(){

	if(uart_buffer[read_ptr] == 'P' && uart_buffer[read_ptr+1] == 'C' && uart_buffer[read_ptr+2] == 'H'){
		read_ptr += 3;
		read_ptr %= BUFFER_SIZE;
		PCH_flag = 1;
		return 1;
	}else{return 0;}

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);
  buffer_index = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	   // HAL_Delay(1000);
	  uint8_t t_code[24];
	  uint8_t t_code_index;

	  //wait for handshake until handshake received
	  while(!PCH_flag){
		  checkPCH();
	  }

	  //after handshake, send mcu handshake until first part of command received
	  HAL_UART_Transmit(&huart2, (uint8_t *) MCUHANDSHAKE, strlen(MCUHANDSHAKE), 10);

	  //delay();

	  //send ready_for_new_command
	  HAL_UART_Transmit(&huart2, (uint8_t *) ready_for_new_command, strlen(ready_for_new_command), 10);

	  //delay();

	  while((uart_buffer[read_ptr] == '\0'));
	  uint8_t temp_ptr = read_ptr;
	  while(!(uart_buffer[temp_ptr] == '\n' || uart_buffer[temp_ptr] == '\r')) {
		  temp_ptr++;
		  temp_ptr %= BUFFER_SIZE;
	  }

	  while (read_ptr != temp_ptr){
		  echo_command = strncat(echo_command, ((char *) &uart_buffer[read_ptr]), 1);
		  read_ptr++;
		  read_ptr %= BUFFER_SIZE;
	  }
	  echo_command = strcat(echo_command, "\n");

	  HAL_UART_Transmit(&huart2, (uint8_t *) echo_command, strlen(echo_command), 10);

	  break;

	  while(!END_flag){
		  if(read_ptr != write_ptr){
			  if(uart_buffer[read_ptr] == '\n' || uart_buffer[read_ptr] == '\r'){
				  read_ptr++;
				  read_ptr %= BUFFER_SIZE;
				  const char* t_code_str = (const char*)t_code;

				  if(start_flag){
					  uint8_t opcode = processCommand(t_code_str);

					  if( opcode == 11 ||opcode == 12){
						  END_flag = 1;
					  }
				  }else{
					  if(checkForStart(t_code_str)){
						  start_flag =1;
					  }
					  //check for startFlag
				  }




				  t_code_index = 0;
				  if(!END_flag){
					  HAL_UART_Transmit(&huart2, (uint8_t *) ready_for_new_command, 5, 10);
				  }
			  }else{

				  t_code[t_code_index] = uart_buffer[read_ptr];
				  t_code_index++;
				  read_ptr ++;
				  read_ptr %= BUFFER_SIZE;
			  }
		  }
		  //check for new byte in uart_buffer
		  //if it's not a newline, put add it to t_code
		  //if it is a newline, call doCommand on it
		  //then send ready_for_new_command

		  //check for new input
		  //if there is a new byte, form it onto


	  }
	  //wait for next command
	  PCH_flag = 0;
	  END_flag = 0;
	  start_flag =0;



	  //when command processed, send ready_for_new_command once
	  //repeat processing command and sending ready until end received
	  //when end received, go back to waiting for handshake




    /* USER CODE END WHILE */





	//  HAL_Delay(1000);
	//  HAL_UART_Transmit(&huart2, ready, 6, 10);
    /* USER CODE BEGIN 3 */
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

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) { // Adjust this based on your UART instance
        uart_buffer[write_ptr] = rx_data; // Store the received data in the buffer
        write_ptr = (write_ptr + 1) % BUFFER_SIZE; // Increment the write pointer and wrap around if needed
        HAL_UART_Receive_IT(&huart2, &rx_data, 1); // Start the next receive
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
