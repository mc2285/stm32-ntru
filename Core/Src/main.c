/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>

#include "usbd_cdc_if.h"
#include "stm32h7xx_hal_hash.h"
#include "stm32h7xx_hal_rng.h"

#include "ntru_api_gauss.h"
#include "rng.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define AT_COMMAND_LENGTH 4
#define N_BENCH 20

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

HASH_HandleTypeDef hhash;

RNG_HandleTypeDef hrng;

/* USER CODE BEGIN PV */

volatile uint8_t USBD_Connected = 0;

char hal_sourced_random_seed[crypto_stream_salsa20_KEYBYTES];

char current_buff[64];

__attribute__((section("._ram_d1"))) char rx_buffer[6144], tx_buffer[10240],
    pub_key[CRYPTO_PUBLICKEYBYTES], sec_key[CRYPTO_SECRETKEYBYTES];

uint32_t n_received = 0, n_stored = 0;
uint8_t tr_flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_HASH_Init(void);
static void MX_RNG_Init(void);
/* USER CODE BEGIN PFP */

static void schedule_error_message(void)
{
  strcpy(tx_buffer, "ERROR\r\n");
}
static void schedule_ok_message(void)
{
  strcpy(tx_buffer, "OK\r\n");
}
static void schedule_overflow_message(void)
{
  strcpy(tx_buffer, "OVERFLOW\r\n");
}

static int8_t encode_hex_string(char *src, char *out, uint32_t len);
static int8_t expand_hex_string(char *src, char *out, uint32_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_USB_DEVICE_Init();
  MX_HASH_Init();
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */

  memset(rx_buffer, 0, sizeof(rx_buffer));
  memset(tx_buffer, 0, sizeof(tx_buffer));
  memset(current_buff, 0, sizeof(current_buff));
  memset(pub_key, 0, sizeof(pub_key));
  memset(sec_key, 0, sizeof(sec_key));

  // Generate a random seed for the random number generator
  for (uint8_t i = 0; i < 8; i++)
  {
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *)(hal_sourced_random_seed + i * 4));
  }

  // Generate an initial key pair
  crypto_sign_keypair((unsigned char *)pub_key, (unsigned char *)sec_key);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    n_received = CDC_RXQueue_Dequeue(current_buff, sizeof(current_buff));
    if (tr_flag)
    {
      CDC_TransmitString(tx_buffer);
      tr_flag = 0;
    }
    if (n_received == 0)
      continue;
    // Check if the buffer can store expanded hex data
    if (n_stored + n_received < sizeof(rx_buffer))
    {
      memcpy(rx_buffer + n_stored, current_buff, n_received);
      n_stored += n_received;
    }
    else
    {
      n_stored = 0;
      schedule_overflow_message();
      tr_flag = 1;
      continue;
    }
    // Check if we have a complete command
    if (rx_buffer[n_stored - 1] == '\n')
    {
      rx_buffer[n_stored - 1] = '\0';
      if (n_stored >= 2 && rx_buffer[n_stored - 2] == '\r')
      {
        rx_buffer[n_stored - 2] = '\0';
      }
      n_stored = 0;

      // A complete command was received - send a response
      tr_flag = 1;

      /*
       * AT command reference:
       *
       * All commands are case-insensitive
       * Unless otherwise specified, commands return "OK" on success, "ERROR" on failure
       * and "OVERFLOW" when too much data is supplied
       *
       * AT+S <raw_data> signs the <hex_string> and returns the signed data as <hex_string>
       * AT+M <hex_string> sets the private key to the <hex_string> if supported by the algorithm
       * AT+V <hex_string> verifies the <hex_string> signature
       *
       * AT+B signs the pub_key (data signed does not matter, just benchmarking)
       * N_BENCH times and returns the average time in msec as <number_string>
       *
       * AT+I returns info about the currently used algorithm as <string>
       * AT+P returns the currently used public key as <hex_string>
       * AT+K returns the currently used private key as <hex_string>
       * AT+T returns the current HAL_Tick value[msec] as <number_string>; used to estimate performance
       * AT+G generates a new key pair
       * 
       * AT+R benchmarks key generation over N_BENCH iterations
       * and returns the average time in msec as <number_string>
       */

      if (strncmp(rx_buffer, "AT+S ", AT_COMMAND_LENGTH + 1) == 0)
      {
        uint64_t len = strlen(rx_buffer + AT_COMMAND_LENGTH + 1);
        if (expand_hex_string(rx_buffer + AT_COMMAND_LENGTH + 1, rx_buffer, len) != 0)
        {
          schedule_error_message();
          continue;
        }
        len /= 2;
        if ((len + CRYPTO_BYTES) * 2 > sizeof(tx_buffer) || len + CRYPTO_BYTES > sizeof(rx_buffer))
        {
          schedule_overflow_message();
          continue;
        }
        crypto_sign((unsigned char *)rx_buffer, &len, (unsigned char *)rx_buffer, len, (unsigned char *)sec_key);
        encode_hex_string(rx_buffer, tx_buffer, len);
      }
      else if (strncmp(rx_buffer, "AT+M ", AT_COMMAND_LENGTH + 1) == 0)
      {
        if (strncmp(CRYPTO_ALGNAME, "NTRU", 4) == 0)
        {
          schedule_error_message();
          continue;
        }
        uint32_t len = strlen(rx_buffer + AT_COMMAND_LENGTH + 1);
        if (len != CRYPTO_SECRETKEYBYTES * 2 ||
            expand_hex_string(rx_buffer + AT_COMMAND_LENGTH + 1, sec_key, len) != 0)
        {
          schedule_overflow_message();
          continue;
        }
        // TODO: Implement public key generation
        schedule_ok_message();
        continue;
      }
      else if (strncmp(rx_buffer, "AT+V ", AT_COMMAND_LENGTH + 1) == 0)
      {
        // TODO: Implement signature verification
        schedule_ok_message();
        continue;
      }
      else if (strncmp(rx_buffer, "AT+B", AT_COMMAND_LENGTH + 1) == 0)
      {
        uint64_t len;
        uint32_t start = HAL_GetTick();
        for (uint8_t i = 0; i < N_BENCH; i++)
        {
          // Uses the public key as the data to sign
          crypto_sign((unsigned char *)rx_buffer, &len, (unsigned char *)pub_key, sizeof(pub_key), (unsigned char *)sec_key);
        }
        utoa((HAL_GetTick() - start) / N_BENCH, tx_buffer, 10);
      }
      else if (strncmp(rx_buffer, "AT+I", AT_COMMAND_LENGTH) == 0)
      {
        strcpy(tx_buffer, CRYPTO_ALGNAME);
        strcat(tx_buffer, "\r\n");
        strcat(tx_buffer, "Public key length: ");
        utoa(CRYPTO_PUBLICKEYBYTES, tx_buffer + strlen(tx_buffer), 10);
        strcat(tx_buffer, "\r\n");
        strcat(tx_buffer, "Private key length: ");
        utoa(CRYPTO_SECRETKEYBYTES, tx_buffer + strlen(tx_buffer), 10);
      }
      else if (strncmp(rx_buffer, "AT+P", AT_COMMAND_LENGTH) == 0)
      {
        encode_hex_string(pub_key, tx_buffer, sizeof(pub_key));
      }
      else if (strncmp(rx_buffer, "AT+K", AT_COMMAND_LENGTH) == 0)
      {
        encode_hex_string(sec_key, tx_buffer, sizeof(sec_key));
      }
      else if (strncmp(rx_buffer, "AT+T", AT_COMMAND_LENGTH) == 0)
      {
        utoa(HAL_GetTick(), tx_buffer, 10);
      }
      else if (strncmp(rx_buffer, "AT+G", AT_COMMAND_LENGTH) == 0)
      {
        crypto_sign_keypair((unsigned char *)pub_key, (unsigned char *)sec_key);
        schedule_ok_message();
        continue;
      }
      else if (strncmp(rx_buffer, "AT+R", AT_COMMAND_LENGTH) == 0)
      {
        uint32_t start = HAL_GetTick();
        for (uint8_t i = 0; i < N_BENCH; i++)
        {
          crypto_sign_keypair((unsigned char *)pub_key, (unsigned char *)sec_key);
        }
        utoa((HAL_GetTick() - start) / N_BENCH, tx_buffer, 10);
      }
      else
      {
        schedule_error_message();
        continue;
      }
      strcat(tx_buffer, "\r\n");
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 15;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief HASH Initialization Function
 * @param None
 * @retval None
 */
static void MX_HASH_Init(void)
{

  /* USER CODE BEGIN HASH_Init 0 */

  /* USER CODE END HASH_Init 0 */

  /* USER CODE BEGIN HASH_Init 1 */

  /* USER CODE END HASH_Init 1 */
  hhash.Init.DataType = HASH_DATATYPE_32B;
  if (HAL_HASH_Init(&hhash) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN HASH_Init 2 */

  /* USER CODE END HASH_Init 2 */
}

/**
 * @brief RNG Initialization Function
 * @param None
 * @retval None
 */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */
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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_OTG_FS_PWR_EN_GPIO_Port, USB_OTG_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5
                           PE6 PE7 PE8 PE9
                           PE10 PE11 PE12 PE13
                           PE14 PE15 PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 PF3
                           PF4 PF5 PF6 PF7
                           PF8 PF9 PF10 PF11
                           PF12 PF13 PF14 PF15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC2 PC3 PC6
                           PC7 PC8 PC9 PC10
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA3 PA4 PA5
                           PA6 PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_10 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 PB10 PB11
                           PB12 PB15 PB3 PB4
                           PB5 PB6 PB7 PB8
                           PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG1 PG2 PG3
                           PG4 PG5 PG6 PG8
                           PG9 PG10 PG12 PG14
                           PG15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STLINK_RX_Pin STLINK_TX_Pin */
  GPIO_InitStruct.Pin = STLINK_RX_Pin | STLINK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_PWR_EN_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_OTG_FS_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD11 PD12 PD13 PD14
                           PD15 PD0 PD1 PD2
                           PD3 PD4 PD5 PD6
                           PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_OVCR_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_OVCR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OTG_FS_OVCR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PG11 PG13 */
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

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

static int8_t encode_hex_string(char *src, char *out, uint32_t len)
{
  static char _hex_lookup[] = "0123456789ABCDEF";
  while (len--)
  {
    *out++ = _hex_lookup[((unsigned char)*src) >> 4];
    *out++ = _hex_lookup[((unsigned char)*src) & 0x0F];
    src++;
  }
  // Null-terminate the string
  *out = '\0';
  return 0;
}

static inline int8_t hex_char_to_bin(char c, char *out)
{
  if (c >= '0' && c <= '9')
    *out = (c - '0');
  else if (c >= 'A' && c <= 'F')
    *out = (c - 'A') + 10;
  else if (c >= 'a' && c <= 'f')
    *out = (c - 'a') + 10;
  else
  {
    return -1;
  }
  return 0;
}

static int8_t expand_hex_string(char *src, char *out, uint32_t len)
{
  // It is unclear what side to pad
  if (len < 2 || (len % 2) != 0)
    return -1;

  do
  {
    char value = 0;
    if (hex_char_to_bin(*src++, &value) != 0)
    {
      return -1;
    }
    *out = value << 4;
    if (hex_char_to_bin(*src++, &value) != 0)
    {
      return -1;
    }
    *out++ |= value;
  } while (len -= 2);
  return 0;
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
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
