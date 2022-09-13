/**
 * @file main.c
 * @author Jignesh
 * @date 25 Jul 2022
 * @brief This file contains main.c based initial code of the device.
 *
 */

#include "bsp_main.h"
#include "cmsis_os.h"
#include "can.h"
#include "i2c_eeprom.h"
#include "tim.h"
#include "wwdg.h"
#include "iwdg.h"
#include "gpio.h"
#include "io_peripheral.h"
#include "1734_debug.h"
#include "dsc.h"
#include "dsc_uart.h"

#include "serial_config.h"
#include "bld_nmbr.h"
#include "module_id.h"

void SystemClock_Config (void);
extern void MX_FREERTOS_Init (void);

int main (void)
{
	HAL_Init ();
	SystemClock_Config ();
	MX_GPIO_Init ();

	// I2C for EEPROM
	MX_I2C1_Init ();
	//
	// Independent watchdog
	MX_IWDG_Init ();
	//
	// Init Debug Serial Console interface
	MX_DSC_UART_Init ();
	DSC_Init ();
	DSC_Writes (DSC_LEVEL_INFO, "\n\r=== System Start ===\n\rbuild number: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, __BUILDNUMBER__, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rrelease string: ");
	DSC_Writes (DSC_LEVEL_INFO, RELEASE_STRING);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rbuild date: ");
	DSC_Writes (DSC_LEVEL_INFO, BUILD_DATE);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rbuild time: ");
	DSC_Writes (DSC_LEVEL_INFO, BUILD_TIME);
	DSC_Writes (DSC_LEVEL_INFO, "\n\r");
	MX_TIM7_Init ();

	// Alert if we are coming out of certain reset states
	if (__HAL_RCC_GET_FLAG (RCC_FLAG_IWDGRST))
	{
		DSC_Writes (DSC_LEVEL_INFO, "IWDT RESET\n\r");
	}

	if (__HAL_RCC_GET_FLAG (RCC_FLAG_SFTRST))
	{
		DSC_Writes (DSC_LEVEL_INFO, "SW RESET\n\r");
	}

	/* manually reset module ID for testing */

#ifdef Rick_TEST_NOT  // Rick_TEST 9/2/2022  used for enable writing to flash for writing.
		HAL_FLASH_Unlock ();
		FLASH_EraseInitTypeDef erase;
		erase.NbPages	  = 1;
		erase.PageAddress = 0x801F800;
		erase.TypeErase	  = FLASH_TYPEERASE_PAGES;

		uint32_t PageError = 0;
		HAL_FLASHEx_Erase (&erase, &PageError);
		HAL_FLASH_Lock ();
#endif

	Module_ID_Init ();

	/*Disabled: deemed unnesesary
	//
	// placing the jumper will perform a test pattern on the eeprom
	// this effectively resets the eeprom too
	if (Debug_IsJumperInstalled () == true)
	{
		//
		// Indicate by LEDs we are doing a hardware test
		IO_SET_LEDModRed (true);
		IO_SET_LEDModGreen (true);
		IO_SET_LEDNetworkRed (true);
		IO_SET_LEDNetworkGreen (true);
		//
		// if eeprom fails, we will keep on all DN leds
		// since this is a special 'hardware test' mode the customer should never see
		if (Eeprom_test () != 0)
		{
			Error_Handler ();
		}
	}
	*/

	//
	// Init parent task
	MX_FREERTOS_Init ();
	vTaskStartScheduler ();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	while (1)
	{
	}
}

void SystemClock_Config (void)
{
	RCC_OscInitTypeDef		 RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef		 RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit	   = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType	  = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState			  = RCC_HSE_ON; // RCC_HSE_ON;  RCC_HSE_OFF RCC_HSE_BYPASS
	RCC_OscInitStruct.HSIState			  = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState			  = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState		  = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource		  = RCC_PLLSOURCE_HSE; // RCC_PLLSOURCE_HSE; // External crystal
	RCC_OscInitStruct.PLL.PLLMUL		  = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PREDIV		  = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig (&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler ();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType		 = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource	 = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider	 = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig (&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler ();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_I2C1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig (&PeriphClkInit) != HAL_OK)
	{
		Error_Handler ();
	}
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		HAL_IncTick ();
	}
	else if (htim->Instance == TIM15)
	{
		MB_Rtu_TimedOut();
	}
}

// Used by standard C library assert()
void __assert_func (const char *a, int b, const char *c, const char *d)
{
	DSC_Writes (DSC_LEVEL_INFO, "ASSERT FAILED\n\r");
	DSC_Writes (DSC_LEVEL_INFO, (char *)a);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rLine ");
	DSC_Write_UInt (DSC_LEVEL_INFO, b, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rFunc ");
	DSC_Writes (DSC_LEVEL_INFO, (char *)c);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rCond ");
	DSC_Writes (DSC_LEVEL_INFO, (char *)d);
	DSC_Writes (DSC_LEVEL_INFO, "\n\r");
	Error_Handler ();
	while (1)
		; // will never get here
}

#ifdef USE_FULL_ASSERT
// Used by ST HAL assertions
void assert_failed (uint8_t *file, uint32_t line)
{
	DSC_Writes (DSC_LEVEL_INFO, "ASSERT FAILED\n\r");
	DSC_Writes (DSC_LEVEL_INFO, (char *)file);
	DSC_Writes (DSC_LEVEL_INFO, "\n\rLine ");
	DSC_Write_UInt (DSC_LEVEL_INFO, line, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\n\r");
	Error_Handler ();
	while (1)
		; // will never get here
}
#endif /* USE_FULL_ASSERT */

// unhandled exceptions and interrupt vectors
void Default_Exception_and_ISR_Handler (void)
{
	Error_Handler ();

	while (1)
	{
	}
}
