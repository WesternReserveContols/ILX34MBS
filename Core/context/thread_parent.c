/*
 * thread_parent.c
 *
 *  Created on: Oct 15, 2019
 *      Author: jason
 */

#include "gpio.h"
#include "dsc.h"
#include "app_main.h"

extern void main_serial (void);
void yield_thread (void);

static Context_t *myContext;

void Thread_Parent (void *argument)
{
	assert (argument);
	Context_t *context = (Context_t *)argument;
	myContext		   = context;

	DSC_Writes (DSC_LEVEL_INFO, "Main UP\n\r");

	Context_Launch_Timer (context);

	while (1)
	{
		main_serial ();
	}
}

void yield_thread (void)
{
	HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_SET);
	// Context_Parent_Dispatch (myContext, &myContext->mutexes.timer);
	xTaskNotifyGive (myContext->threads.timer.xTask);
	ulTaskNotifyTake (pdTRUE, portMAX_DELAY);
	HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
	// Context_Parent_Dispatch (myContext, &myContext->mutexes.can_hardware);
	// HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_SET);
	// Context_Parent_Dispatch (myContext, &myContext->mutexes.can_software);
	// HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
	// Context_Parent_Dispatch (myContext, &myContext->mutexes.serial);
}
