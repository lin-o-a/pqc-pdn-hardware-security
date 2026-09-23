/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Local & PQC Project Headers
#include "api.h"
#include "kem.h"
#include "stm32l5xx.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#define STRESS_LEN 512
volatile uint32_t ram_buf_a[STRESS_LEN];
volatile uint32_t ram_buf_b[STRESS_LEN];

// Define an un-initialized RAM section (persists across warm resets / BOR)
__attribute__((section(".noinit")))         uint32_t authorization_flag;

#define AUTH_OK       (1U << 0)  // 0x01
#define AUTH_REQUIRED (1U << 1)  // 0x02

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

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
I2C_HandleTypeDef hi2c1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);

/* USER CODE BEGIN PFP */
static void MX_I2C1_Init(void);

void System_PVD_Init(void);
static void Check_BOR_Recovery(void);
static void Display_Init(void);
static void Run_PQC_Benchmark(void);

static uint8_t current_y = 0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void OLED_PrintLine(const char *str) {
	// If exceed the 64-pixel display height, reset to top
	if (current_y > 52) {
		ssd1306_Fill(Black);
		current_y = 0;
	}

	ssd1306_SetCursor(0, current_y);
	ssd1306_WriteString((char*) str, Font_7x10, White);
	ssd1306_UpdateScreen();

	// Advance to next line (+12 pixels)
	current_y += 12;
}

uint8_t Has_Fault_Reset(void) {
    uint8_t fault_occurred = 0;

    // Check specifically for Software Reset (PVD trigger) OR Hardware BOR
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) || __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
        fault_occurred = 1;
    }

    // Always clear flags after checking so they don't persist into future boots
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return fault_occurred;
}

static void Configure_Leds(void) {
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_RED);

	BSP_LED_Off(LED_GREEN);
	BSP_LED_Off(LED_BLUE);
	BSP_LED_Off(LED_RED);

}

// Abstracted BOR Check Logic
static void Check_BOR_Recovery(void) {
	if (Has_Fault_Reset()) {
		//ssd1306_UpdateScreen();
		//OLED_PrintLine("Catch RESET");
		__HAL_RCC_CLEAR_RESET_FLAGS();

		if (authorization_flag == AUTH_OK) {
			BSP_LED_Toggle(LED_RED);
			HAL_Delay(200);
		}
	}
}

// Abstracted Display & I2C Address Probe
static void Display_Init(void) {
	ssd1306_Init();

	uint8_t i2c_addr = (0x3C << 1);
	if (HAL_I2C_IsDeviceReady(&hi2c1, i2c_addr, 3, 100) == HAL_OK) {
		BSP_LED_On(LED_GREEN);
	} else if (HAL_I2C_IsDeviceReady(&hi2c1, (0x3D << 1), 3, 100) == HAL_OK) {
		BSP_LED_On(LED_GREEN);
	} else {
		BSP_LED_On(LED_RED);
	}
}

// Abstracted PQC Execution Step
static void Run_PQC_Benchmark(void) {
	uint8_t pk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_PUBLICKEYBYTES];
	uint8_t sk[PQCLEAN_MLKEM512_CLEAN_CRYPTO_SECRETKEYBYTES];
	uint8_t ct[PQCLEAN_MLKEM512_CLEAN_CRYPTO_CIPHERTEXTBYTES];
	uint8_t ss_a[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];
	uint8_t ss_b[PQCLEAN_MLKEM512_CLEAN_CRYPTO_BYTES];

	OLED_PrintLine("Begin PQC");

	PQCLEAN_MLKEM512_CLEAN_crypto_kem_keypair(pk, sk);
	OLED_PrintLine("PQC keypair");

	PQCLEAN_MLKEM512_CLEAN_crypto_kem_enc(ct, ss_a, pk);
	OLED_PrintLine("PQC encapsulation");

	PQCLEAN_MLKEM512_CLEAN_crypto_kem_dec(ss_b, ct, sk);
	OLED_PrintLine("PQC decapsulation");
	OLED_PrintLine("PQC DONE");

	BSP_LED_Toggle(LED_GREEN);
	HAL_Delay(200);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	HAL_Init();
	SystemClock_Config();

	System_PVD_Init(); // Active voltage monitoring armed early

	MX_GPIO_Init();
	MX_ICACHE_Init();
	MX_I2C1_Init();

	/* USER CODE BEGIN 2 */
	Configure_Leds();

	Check_BOR_Recovery();
	Display_Init();

	authorization_flag = AUTH_OK;
	/* USER CODE END 2 */

	/* USER CODE BEGIN WHILE */
	while (1) {
		Run_PQC_Benchmark();
	}
	/* USER CODE END WHILE */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 55;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief Init power voltage detector
 * @param None
 * @retval None
 */
void System_PVD_Init(void) {
	PWR_PVDTypeDef sConfigPVD = { 0 };

	// 1. Enable PWR Peripheral Clock
	__HAL_RCC_PWR_CLK_ENABLE();

	// 2. Select PVD Threshold Voltage Level
	// Example: PWR_PVDLEVEL_6 (typically ~2.8V depending on family datasheet)
	sConfigPVD.PVDLevel = PWR_PVDLEVEL_4;

	// 3. Trigger on falling edge (as rail drops down)
	sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING_FALLING;

	// 4. Configure PVD in Power Control
	HAL_PWR_ConfigPVD(&sConfigPVD);

	// 5. Enable PVD
	HAL_PWR_EnablePVD();

	// 6. Enable NVIC Interrupt Line (PVD/AVD EXTI line 16)
	HAL_NVIC_SetPriority(PVD_PVM_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(PVD_PVM_IRQn);
}

/**
 * @brief High-priority interrupt handler
 * @param None
 * @retval None
 */
void PVD_PVM_IRQHandler(void) {
	BSP_LED_Toggle(LED_BLUE);
	OLED_PrintLine("Catch RESET");

	if (__HAL_PWR_PVD_EXTI_GET_FLAG()) {
		authorization_flag = 0x00000000U;

		// 3. Memory barrier to ensure SRAM write completes
		__DSB();
		__ISB();

		// 4. Clear flag & trigger immediate software reset
		__HAL_PWR_PVD_EXTI_CLEAR_FLAG();
		NVIC_SystemReset();
	}
}

/**
 * @brief ICACHE Initialization Function
 * @param None
 * @retval None
 */
static void MX_ICACHE_Init(void) {

	/* USER CODE BEGIN ICACHE_Init 0 */

	/* USER CODE END ICACHE_Init 0 */

	/* USER CODE BEGIN ICACHE_Init 1 */

	/* USER CODE END ICACHE_Init 1 */

	/** Enable instruction cache in 1-way (direct mapped cache)
	 */
	if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_ICACHE_Enable() != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ICACHE_Init 2 */

	/* USER CODE END ICACHE_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void MX_I2C1_Init(void) {
	/* Enable peripheral clocks */
	__HAL_RCC_I2C1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* Configure GPIO pins PB6 (SCL) and PB7 (SDA) */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Configure I2C1 Peripheral Handle */
	hi2c1.Instance = I2C1;
	// Timing value for STM32L5 @ 110MHz (Standard Mode 100 kHz)
	hi2c1.Init.Timing = 0x10707DBC;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
