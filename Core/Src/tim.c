#include "tim.h"


TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim15;

void MX_TIM7_Init (void)
{
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	htim7.Instance				 = TIM7;
	htim7.Init.Prescaler		 = 47;
	htim7.Init.CounterMode		 = TIM_COUNTERMODE_UP;
	htim7.Init.Period			 = 1000 - 1;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init (&htim7) != HAL_OK)
	{
		Error_Handler ();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode	  = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization (&htim7, &sMasterConfig) != HAL_OK)
	{
		Error_Handler ();
	}

	HAL_TIM_Base_Start_IT (&htim7);

}

void HAL_TIM_Base_MspInit (TIM_HandleTypeDef *tim_baseHandle)
{
	if (tim_baseHandle->Instance == TIM7)
	{
		/* TIM7 clock enable */
		__HAL_RCC_TIM7_CLK_ENABLE ();
	}
	else if (tim_baseHandle->Instance == TIM15)
	{
		/* TIM7 clock enable */
		__HAL_RCC_TIM15_CLK_ENABLE ();
	}
}

void HAL_TIM_Base_MspDeInit (TIM_HandleTypeDef *tim_baseHandle)
{
	if (tim_baseHandle->Instance == TIM7)
	{
		/* Peripheral clock disable */
		__HAL_RCC_TIM7_CLK_DISABLE ();
	}
	else if (tim_baseHandle->Instance == TIM15)
	{
		/* TIM7 clock enable */
		__HAL_RCC_TIM15_CLK_DISABLE ();
	}
}


void MX_TIM15_Init (int period)
{
	RCC_ClkInitTypeDef clkconfig;
	uint32_t		   uwTimclock		= 0;
	uint32_t		   uwPrescalerValue = 0;
	uint32_t		   pFLatency;

	/*Configure the TIM6 IRQ priority */
	HAL_NVIC_SetPriority (TIM15_IRQn, 0, 0);

	/* Enable the TIM6 global Interrupt */
	HAL_NVIC_EnableIRQ (TIM15_IRQn);

	NVIC_GetPendingIRQ(TIM15_IRQn);

	NVIC_ClearPendingIRQ (TIM15_IRQn);

	/* Enable TIM6 clock */
	__HAL_RCC_TIM15_CLK_ENABLE ();

	/* Get clock configuration */
	HAL_RCC_GetClockConfig (&clkconfig, &pFLatency);

	/* Compute TIM15 clock */
	uwTimclock = 2 * HAL_RCC_GetPCLK1Freq ();

	/* Compute the prescaler value to have TIM6 counter clock equal to 1MHz */
	uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000));

	/* Initialize TIM6 */
	htim15.Instance = TIM15;

	/* Initialize TIMx peripheral as follow:
	+ Period = [(TIM15CLK/1000) - 1]. to have a (1/1000) s time base.
	+ Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
	+ ClockDivision = 0
	+ Counter direction = Up
	*/
	htim15.Init.Period		 = (period); //value in micro seconds as Prescaler makes clock 1MHz
	htim15.Init.Prescaler	 = uwPrescalerValue * 10;
	htim15.Init.CounterMode	 = TIM_COUNTERMODE_UP;
	htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim15.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init (&htim15) == HAL_OK)
	{
//		__HAL_TIM_CLEAR_IT(&htim15, TIM_IT_UPDATE);
//		HAL_TIM_Base_Start_IT (&htim15);
	}
}

void MX_TIM15_Start (void)
{
	__HAL_TIM_CLEAR_IT(&htim15, TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT (&htim15);
}

void MX_TIM15_Stop (void)
{
	HAL_TIM_Base_Stop_IT (&htim15);
}
