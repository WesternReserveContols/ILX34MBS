/*
 * thread_can.c
 *
 *  Created on: Oct 15, 2019
 *      Author: jason
 */
#define _POSIX_C_SOURCE 200809L

#include "gpio.h"
#include "bsp_main.h"
#include "app_main.h"
#include "can.h"
#include "dsc.h"


#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#ifdef SIM_CONSUME
#include "dn_msgob.h"
#endif
#include <unistd.h>

static Context_t *can_context = 0;
#ifdef SIM_CONSUME
extern uchar	AppObjectInitialized;
#endif
void cp_transmit_in_progress (void);

void Timer_Thread_Yield (void)
{
	// Poll for tx complete
	cp_transmit_in_progress ();
	//
	// Give back to parent
	// Context_Child_Exit (can_context);
	xTaskNotifyGive (can_context->threads.parent.xTask);
	//
	// Wait for next poll
	ulTaskNotifyTake (pdTRUE, portMAX_DELAY);
	// Context_Child_Enter (&can_context->mutexes.timer);
}

void TimerObjectISR (void);

bool period_elapsed (void)
{
	TickType_t		  now	= xTaskGetTickCount ();
	static TickType_t last	= 0;
	static TickType_t delta = 0;
	delta += now - last;
	last = now;

	if (delta)
	{
		delta--;
		return true;
	}
	return false;
}

#ifdef SIM_CONSUME
extern void MessageObjectHandleRxPoll (void);
#endif
void Thread_Timer (void *argument)
{
	assert (argument);
	Context_t *context = (Context_t *)argument;
	can_context		   = context;

	DSC_Writes (DSC_LEVEL_INFO, "Timer UP\n\r");

	// Launch our CAN Hardware Thread
	// Context_Launch_CAN_Hardware (context);

	while (1)
	{
		// Wait for notification from main thread that we can run.
		// Indefinite wait, and binary semaphone
		ulTaskNotifyTake (pdTRUE, portMAX_DELAY);
		// Context_Child_Enter (&context->mutexes.timer);
		// HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_SET);

		static int myTimeTicks = 0;
		myTimeTicks++;
		if (period_elapsed ())
		{
			// DSC_Write_UInt (DSC_LEVEL_INFO, myTimeTicks, 0);
			myTimeTicks = 0;
			TimerObjectISR ();
		}

#ifdef SIM_CONSUME
		static int temp = 0;
		extern void AppObjectMonitorIO (void);
		if ((++temp >= 20000) && (AppObjectInitialized))
		{
			temp = 0;
			MessageObjectHandleRxPoll ();
		}

		if (AppObjectInitialized)
		{
			AppObjectMonitorIO(); //TODO Remove once RX is working via CAN messages //main_port_serial(); //This function used to test receive buffer.
		}
#else

#endif
		CP_Hardware_Handler ();

		CP_Software_Handler ();
		//
		// will flash a test LED on hardware
		// Debug_Heartbeat ();
		// HAL_GPIO_WritePin (LED6_GPIO_Port, LED6_Pin, GPIO_PIN_RESET);
		/* Debug TEST info for tasks
		 *  This should only be defined for debugging
		 */
		/*
		static uint8_t firstTime	   = 1;
		static int	   reportTicks	   = 0;
		const int	   reportTickTime  = 5000;
		UBaseType_t	   uxHighWaterMark = 1;
		char		   cBuff[24];
		TaskHandle_t   xHandleParent;
		TaskHandle_t   xHandleCanHW;
		TaskHandle_t   xHandleCanSW;

		reportTicks++;
		if (reportTicks > reportTickTime)
		{
			reportTicks = 0;
			// only obtain the task handles once.
			// Report tick time should be long enough so
			//  all the threads are initialized by the time we get here.
			if (firstTime)
			{
				firstTime = 0;
				// get handle
				xHandleParent = xTaskGetHandle ("parent");
				configASSERT (xHandleParent);
				xHandleCanHW = xTaskGetHandle ("can-h");
				configASSERT (xHandleCanHW);
				xHandleCanSW = xTaskGetHandle ("can-s");
				configASSERT (xHandleCanSW);
			}

			DSC_Writes (DSC_LEVEL_INFO, "HEAP--\n\r");
			sprintf (cBuff, "  Now: %u\r\n", xPortGetFreeHeapSize ());
			DSC_Writes (DSC_LEVEL_INFO, cBuff);

			sprintf (cBuff, "  Min: %u\r\n", xPortGetMinimumEverFreeHeapSize ());
			DSC_Writes (DSC_LEVEL_INFO, cBuff);

			DSC_Writes (DSC_LEVEL_INFO, "STACK--\n\r");

			// parent (main)
			uxHighWaterMark = uxTaskGetStackHighWaterMark (xHandleParent);
			sprintf (cBuff, "  Main: %lu\r\n", uxHighWaterMark);
			DSC_Writes (DSC_LEVEL_INFO, cBuff);

			// Timer
			uxHighWaterMark = uxTaskGetStackHighWaterMark (NULL);
			sprintf (cBuff, "  Timr: %lu\r\n", uxHighWaterMark);
			DSC_Writes (DSC_LEVEL_INFO, cBuff);

			// CAN HW
			uxHighWaterMark = uxTaskGetStackHighWaterMark (xHandleCanHW);
			sprintf (cBuff, "  C-HW: %lu\r\n", uxHighWaterMark);
			DSC_Writes (DSC_LEVEL_INFO, cBuff);

			// CAN SW
			uxHighWaterMark = uxTaskGetStackHighWaterMark (xHandleCanSW);
			sprintf (cBuff, "  C-SW: %lu\r\n", uxHighWaterMark);
			DSC_Writes (DSC_LEVEL_INFO, cBuff);
		}
		*/
		/* END Debug TEST info for tasks */
		// Context_Child_Exit (context);
		xTaskNotifyGive (context->threads.parent.xTask);
	}
}
