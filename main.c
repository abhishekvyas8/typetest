/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

int count = 0;
int value = 1;

int count2 = 0; //DC monitor
uint32_t compval = 0;



static char data[100]; //game commands data
//static char user_entry[100]; //what the user enters during game
static char input[2];
static char input2[2];
static char index;
static int flag = 0; //putty input flag
static int state = 200;
//static int mode;
static int tim_flag = 0;
//static int input_length = 0; //given string length
//static int user_length = 0; //user entered length
static int game_time = 0; //timer to enter string
static int countdown = 4; //start check
//static int complete_flag = 0; //user completes entering string
//static char data2[100]; //entered data
//static char index2;
//static int update_flag = 0;
static int level = 1;
static int practice_time = 0;
static char practice[100];
static int practice_score = 0;
//static int countdown2 = 4;

//static int pwmflag = 0;

//static int tim3_flag = 0;

//score
//+5 for every right letter
//-2 for every wrong letter
//0 if time runs out
static int score = 0;

//thresholds
static int threshold1 = 15;
static int threshold2 = 35;
static int threshold3 = 60;
static int threshold4 = 70;
static int threshold5 = 80;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void App_reg_init();
//extern void TIM2_IRQHandler()
uint32_t adcRead(void);
void adcEnableChannel(int channel);




void displayString(char* output){
    int index = 0;
    while(index < strlen(output)){
        HAL_Delay(50);
        HAL_SPI_Transmit(&hspi1, (uint8_t*)(output + index), 1, 150);
        index++;
    }
}

void clearDisplay(){
    char ch1 = 0xFE;
    char ch2 = 0x51;
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&ch1, 1, 150);
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&ch2, 1, 150);
}

void blinkCursor(){
    char ch1 = 0xFE;
    char ch2 = 0x4B;
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, &ch1, 1, 150);
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, &ch2, 1, 150);
}

void cursorHome(){
    char ch3 = 0xFE;
    char ch4 = 0x46;
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, &ch3, 1, 150);
    HAL_Delay(50);
    HAL_SPI_Transmit(&hspi1, &ch4, 1, 150);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
    uint8_t i;
    uint8_t j;
    if(huart -> Instance == USART1){
        if(index == 0){
            for(i = 0; i < 100; i++){
                data[i] = 0;
            }
        }

       /*if(index2 == 0){
            for(j = 0; j < 100; j++){
                data2[j] = 0;
            }
        }

        if(user_length != input_length ){
            data2[index2] = input2[0];
            user_length++;
            index2++;
            update_flag = 1;
        }
        else if(user_length == input_length){
            //index2 = 0;
            complete_flag = 1;
        }*/

        if(input[0] != 13 && input[0] != 10){
            data[index] = input[0];
            index++;

        }

        else if(input[0] == 13 || input[0] == 10){
            index = 0;
            flag = 1;

        }
    }
    HAL_UART_Receive_IT(&huart1, input, 1);
    HAL_UART_Receive_IT(&huart1, input2, 1);
}


void transmitString(UART_HandleTypeDef * huart, char * output){
    int length = strlen(output);
    HAL_UART_Transmit(huart, output, length, 1000);
}

void transmiti2c(char* output){
    int index = 0;
        while(index < strlen(output)){
            HAL_Delay(50);
            HAL_I2C_Master_Transmit(&hi2c1, 0x42<<1, (uint8_t*)(output + index), 1, 150);
            index++;
        }
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    App_reg_init();
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, input, 1);
  HAL_UART_Receive_IT(&huart1, input2, 1);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_I2C_MspInit(&hi2c1);
  //blinkCursor();

  //transmitString(&huart1, "Modes: \n");
  HAL_Delay(500);

  //displayString("Enter Mode");

  //displayString("Modes:");
  //transmitString(&huart1, "Welcome: \n");




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

                  //count2  = 0;
  /*                adcEnableChannel(1);
                  compval  = adcRead();
                  value = compval;
char val[100];
           sprintf(val, "Value: %d\n", value);
            transmiti2c("&");
            transmiti2c(&val);*/

      //transmiti2c("1");
      //transmiti2c("0");
      //game state machines
      //initial state

      if(state == 200){
          transmiti2c("&");
          //HAL_Delay(500);
          transmiti2c("Welcome To\n");
          HAL_Delay(500);
          transmiti2c("Team 09's\n");
          HAL_Delay(500);
          transmiti2c("Project");
          HAL_Delay(500);
          /*transmitString(&huart1, "Welcomexg: \n");*/

          HAL_Delay(500);
          clearDisplay();
          HAL_Delay(500);
          cursorHome();

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("\n  RULES!");
          HAL_Delay(1000);

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("$");
          HAL_Delay(500);
          transmiti2c("  +5 for\n   every\n   right\n character");
          HAL_Delay(1500);
          transmiti2c("$");
          HAL_Delay(500);

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("%");
          HAL_Delay(500);
          transmiti2c("  -2 for\n   every\n   wrong\n character");
          HAL_Delay(1500);
          transmiti2c("%");
          HAL_Delay(500);
          state = 0;
      }

      else if(state == 0){
          clearDisplay();
          cursorHome();
          displayString("Enter Mode");
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("   Ready\n   Player\n     1\n");
          HAL_Delay(2000);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("\nPress x to  start \n");
          while(flag == 0){
          }
          flag = 0;
          if(strcmp(data, "x") == 0){
              state = 1;
          }
          else{
              state = 0;
          }
      }

      //display mode state
      else if(state == 1){
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("Enter Mode\n");
          HAL_Delay(500);
          transmiti2c("1.Game\n");
          HAL_Delay(500);
          transmiti2c("2.Practice");
          state = 2;
      }

      //enter mode state
      else if(state == 2){
          while(flag == 0){
          }
          flag = 0;

          int mode = atoi(data);

          if(mode == 1){
              state = 30;
          }
          else if(mode == 2){
              state = 3;
          }
          else{
              state = 2;
          }

      }

      //practice mode
      else if(state == 3){
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("Enter 30  characters\n");
          while(flag == 0){
          }
          flag = 0;

          //int practice_length = 30;

          for(int i = 0; i < 30; i++){
              practice[i] = data[i];
          }

          HAL_Delay(500);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("Enter the time limit\n");
          while(flag == 0){
          }
          flag = 0;

          practice_time = atoi(data);

          state = 91;
      }

      //countdown to start
      else if(state == 4){
          countdown = 4;
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c(" Type when   buzzer \n   beeps!");
          //HAL_Delay(500);
          //transmiti2c("   time \n  starts\n");
          HAL_Delay(500);
          //transmitString(&huart1,"Look at OLED for start time. String will display once you can start.\n");
          //transmitString(&huart1,"Press enter when you are done typing.\n");

          while(countdown != 0){
              if(tim_flag == 1){
                  tim_flag = 0;
                  countdown--;

                  clearDisplay();
                  cursorHome();
                  char times[100];
                  sprintf(times, "%d", countdown);


                  displayString(times);
              }
          }

          if(countdown == 0){
              clearDisplay();
              cursorHome();
              displayString("start");
              state = 5;
          }
      }

      else if(state == 5){

          char* str;
          char* str1;

          if(level == 1){
              str = "Best Class";
              str1 = "Best Class";
              game_time = 4;
          }
          else if(level == 2){
              str = "ECE362 is best class";
              str1 = "ECE362 is best class";
              game_time = 6;
          }
          else if(level == 3){
              str = "Meyers is the best professor";
              str1 = "Meyers is the best  professor";
              game_time = 11;
          }
          else if(level == 4){
              str = "Meyers is the best professor";
              str1 = "Meyers is the best  professor";
              game_time = 8;
          }
          else if(level == 5){
              str = "Meyers is the best professor";
              str1 = "Meyers is the best  professor";
              game_time = 6;
          }

          char val[100];
          sprintf(val, "\nYou have \n%d seconds", game_time);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c(&val);
          HAL_Delay(2000);

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c(str1);

          HAL_Delay(100);
          TIM1->CCR1 = 1500;
          HAL_Delay(350);
          TIM1 ->CCR1 = 0;

          transmitString(&huart1,"\n");

          while(flag == 0 && game_time != 0){

              if(tim_flag == 1){
                  tim_flag = 0;
                  game_time--;

                  clearDisplay();
                  cursorHome();
                  char times[100];
                  sprintf(times, "%d", game_time);
                  displayString(times);
              }
          }
          flag = 0;

          transmiti2c("&");
          if(game_time == 0){
              TIM1->CCR1 = 1500;
              HAL_Delay(100);
              transmiti2c("Timeout.");
              HAL_Delay(1000);
              TIM1->CCR1 = 0;


              score = 0;
          }
          else{
              for(int i = 0; i < strlen(str); i++){
                  char* val = str[i];
                  if(data[i] == str[i]){
                      score += 5;
                      transmiti2c("$");
                      transmiti2c(&val);
                      transmiti2c("$");
                  }
                  else{
                      score -= 2;
                      transmiti2c("%");
                      transmiti2c(&val);
                      transmiti2c("%");
                  }
              }
          }
          state = 6;
      }

      else if(state == 6){
          char result[100];
          sprintf(result, "\nScore: %d", score);

          clearDisplay();
          cursorHome();
          displayString(result);

          transmitString(&huart1,"\n\n");
          transmiti2c(&result);
          transmitString(&huart1,"\n\n");

          if(level == 1){
              if(score >= threshold1){
                  state = 7;
              }
              else{
                  state = 50;
              }
          }
          else if(level == 2){
              if(score >= threshold2){
                  state = 7;
              }
              else{
                  state = 50;
              }
          }
          else if(level == 3){
              if(score >= threshold3){
                  state = 7;
              }
              else{
                  state = 50;
              }
          }
          else if(level == 4){
              if(score >= threshold4){
                  state = 7;
              }
              else{
                  state = 50;
              }
          }
          else if(level == 5){
              if(score >= threshold5){
                  state = 100; //game complete state
              }
              else{
                  state = 50;
              }
          }

      }

      else if(state == 7){
          char out[100];
          sprintf(out, "You have  cleared   level %d\n", level);
          HAL_Delay(2000);
          transmiti2c("&");
          transmiti2c(&out);
          HAL_Delay(2000);

          score = 0;

          level++;

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("x:Continue\n");
          HAL_Delay(500);
          transmiti2c("m:Menu\n");
          HAL_Delay(500);
          transmiti2c("r:Reset\n");

          while(flag == 0){
          }
          flag = 0;

          if(strcmp(data, "x") == 0){
              state = 30;
          }
          else if(strcmp(data, "m") == 0){
              state = 1;
          }
          else if(strcmp(data, "m") == 0){
              state = 200;
          }
          else{
              state = 25;
          }
      }

      else if(state == 50){
          score = 0;
          char out[100];
          sprintf(out, "You have  failed to clear     level %d.\n", level);

          HAL_Delay(2000);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c(&out);
          HAL_Delay(2000);

          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("x:Retry\n");
          HAL_Delay(500);
          transmiti2c("m:Menu\n");
          HAL_Delay(500);
          transmiti2c("r:Reset\n");

          while(flag == 0){
          }
          flag = 0;

          if(strcmp(data, "x") == 0){
              state = 30;
          }
          else if(strcmp(data, "m") == 0){
              state = 1;
          }
          else if(strcmp(data, "r") == 0){
              state = 200;
          }
          else{
              state = 50;
          }
      }

      else if(state == 100){
          HAL_Delay(2000);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("\n You have\n   won!!");
          HAL_Delay(2000);
          /*transmiti2c("#");
          HAL_Delay(2000);
          transmiti2c("#");
          HAL_Delay(500);
          transmiti2c("&");
          HAL_Delay(500);*/
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("m:Menu\n");
          HAL_Delay(500);
          transmiti2c("r:Restart\n");

          while(flag == 0){
          }
          flag = 0;

          level = 1;

          if(strcmp(data, "r") == 0){

              score = 0;
              state = 200;
          }
          else if(strcmp(data, "m") == 0){
              state = 1;
          }

      }

      else if(state == 30){
          char out[100];
          sprintf(out, "\n  Level %d \n", level);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c(&out);
          HAL_Delay(2000);


          state = 4;

      }

      else if(state == 25){
          transmitString(&huart1,"Press x to continue.\n");
          while(flag == 0){
          }
          flag = 0;

          if(strcmp(data, "x") == 0){
              state = 30;
          }
          else{
              state = 25;
          }
      }

      else if(state == 91){

          practice_score = 0;
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("Type when   buzzer \n   beeps!");
          HAL_Delay(500);
          transmiti2c("&");
          transmiti2c(&practice);

          /*int countdown2 = 4;

          while(countdown2 != 0){
              if(tim_flag == 1){
                  tim_flag = 0;
                  countdown2--;
              }
          }
    */
          HAL_Delay(100);
          TIM1 -> CCR1 = 1500;
          HAL_Delay(350);
          TIM1 -> CCR1 = 0;


          while(flag == 0 && practice_time != 0){

              if(tim_flag == 1){
                  tim_flag = 0;
                  practice_time--;

                  clearDisplay();
                  cursorHome();
                  char times[100];
                  sprintf(times, "%d", practice_time);
                  displayString(times);
              }
          }
          flag = 0;

          transmiti2c("&");

          if(practice_time == 0){
              //transmiti2c("Timeout.\n");
              TIM1->CCR1 = 1500;
              HAL_Delay(100);
              transmiti2c("Timeout.");
              HAL_Delay(1000);
              TIM1->CCR1 = 0;
              practice_score = 0;
          }
          else{
              for(int i = 0; i < 30; i++){
                  char* val = practice[i];
                  if(data[i] == practice[i]){
                      practice_score += 5;
                      transmiti2c("$");
                      transmiti2c(&val);
                      transmiti2c("$");
                  }
                  else{
                      practice_score -= 2;
                      transmiti2c("%");
                      transmiti2c(&val);
                      transmiti2c("%");
                  }
              }
          }

          state = 93;
      }

      else if(state == 92){
          practice_score = 0;
          practice_time = 0;

          HAL_Delay(2000);
          transmiti2c("&");
          HAL_Delay(500);
          transmiti2c("p:Practice\n");
          HAL_Delay(500);
          transmiti2c("m:Menu\n");
          HAL_Delay(500);
          transmiti2c("r:Restart");
          //state = 97;

          while(flag == 0){
          }
          flag = 0;

          if(strcmp(data, "m") == 0){
              state = 1;
          }
          else if(strcmp(data, "p")==0){
              state = 3;
          }
          else if(strcmp(data, "r")==0){
              state = 200;
          }
          else{
              state = 92;
          }
      }

      else if(state == 93){
          /*char result[100];
          sprintf(result, "\nScore: %d", practice_score);
          transmiti2c(&result);*/

          state = 92;
      }

  /* USER CODE END WHILE */

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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 132;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 47999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD4_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim){
    if(htim -> Instance == TIM2){
        tim_flag = 1;
        count2++;
    }

    /*if(htim -> Instance == TIM3){
        //tim3_flag = 1;
        count++;
        count2++;
    }*/
}

void App_reg_init(){
        //Configuring GPIOA as alternating function --------------------------------
        RCC->AHBENR |= 0x000E0000; //clock for GPIOA,C and B
        GPIOA->MODER |= 0x00020000;  // PA8,9,10 as alternate
        GPIOA->MODER |= 0x0000003C;  // PA1,2 as analog inputs and B1-PA0
        //configure as ch1
        GPIOA->AFR[1] |= 0x00000002;//PA8,9,10 set to PWM
        //GPIOA->AFR[2] |= 0x00000002;
    //  GPIOA->AFR[3] |= 0x00000002;

        //GPIOB->MODER |= 0x01550000;  // PB8,9,10,11,12 as output

        //Configuring TIM 1---------------------------------------------------------
        RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;;
        //setting requried freq
        //TIM1->PSC = 7;//EFFECT OF PSC?
        //TIM1->ARR = 99;
        TIM1->ARR = 4095;//?12 bit values

        //DUTY CYCLE
    //  TIM1->CCR1 = 100;          //-----------UNCOMMENT THIS FOR STEP 1
        //COUNT ENABLE
        //TIM1->CR1 &= 0XFFEF;
        //SETTING PWM
        TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE ;
        //e8 instead of 60   ENABLED OC1-COUNTER ENABLE-PWM MODE1 AND OC1 PRELOADER ENALBE
        //TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE ;
        //TIM1->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE ;

        TIM1->CCER |= TIM_CCER_CC1E;
//        TIM1->CCER |= TIM_CCER_CC2E;
  //      TIM1->CCER |= TIM_CCER_CC3E;//
        TIM1->BDTR |= 0x00008000; //16th bit
//               TIM1 -> DIER |= 0x0001;

         TIM1->CR1 |= 0x00000001;//&=

          TIM1->CCR1 = 0;
      //      TIM1->CCR2 = 0;
    //        TIM1->CCR3 = 0;

        //DIER NOT REQD
        //----------------------------------------------------------------

        //ADC
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        RCC->CR2 |= RCC_CR2_HSI14ON;
        while((RCC->CR2 & RCC_CR2_HSI14RDY) == 0);
        // Enable ADC
        ADC1->CR |= ADC_CR_ADEN;
        // Wait for ADC to be ready
        while((ADC1->ISR & ADC_ISR_ADRDY) == 0);


    //  -----------------------------------------
        //TIM2 config - 5ms interrupt service
        /*RCC->APB1ENR |= 0x00000001;
        TIM2->CR1 |= 0x00000000;
        TIM2->PSC = 7999;
        TIM2->ARR = 4;
        TIM2->DIER |= 0x00000001;
        TIM2->CR1 |= 0x00000001;


        NVIC->ISER[0] |= 0x00008000;
        NVIC->ICPR[0] |= 0x00008000;*/

    return;
}


void adcEnableChannel(int channel) {
// Wait for ADC to be ready
while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
// Ensure ADCStart = 0
while((ADC1->CR & ADC_CR_ADSTART) == 1);
ADC1->CHSELR = 0;
ADC1->CHSELR |= 1 << channel;
}

uint32_t adcRead(void) {
uint32_t adcValue = 0;
// Wait for ADC to be ready
while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
// Start the ADC (ADCStart = 1)
ADC1->CR |= ADC_CR_ADSTART;
// Wait for end of conversion
while((ADC1->ISR & ADC_ISR_EOC) == 0);
adcValue = ADC1->DR;
return adcValue;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
