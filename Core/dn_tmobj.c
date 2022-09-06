//#pragma iv
#include <app_objs.h>
#include <c505c.h>
#include <dn_ahobj.h>
#include <dn_cnobj.h>
#include <dn_dnobj.h>
#include <dn_init.h>
#include <dn_msgob.h>
#include <dn_tmobj.h>
#include <dn_uiobj.h>
#include <enbl_obj.h>

#include "serial_config.h"

#define EN_BUS_FILTER 2 // X ms before turn enable line is accepted at powerup

extern void UIObjectLEDRefresh (void);

extern uchar AppObjectInitialized;

static unsigned int 	TimerObjectTick;
static unsigned char TimerTicks; // inc'ed in ISR, dec'ed in foreground
unsigned char		 TryCount;
unsigned char		 FilterBusEnInPin;

//***********************************************************************
//
// Function                  TimerObjectRoundUp()
//
// Author                    Brian Taylor
// Date                      3/18/97
//
// Functional Description    This function rounds a timer up to the closest
//                     possible timer increment.  Since the Timer value
//                     is in 1 ms increments and our timer can only do
//                     10.24 ms we need to round up to increments of 8ms
//                     for our resolution.
//
// Inputs               cpInputTimerLO - Low Byte of Timer value to be rounded
//                     cpInputTimerHI - High Byte of Timer value to be rounded
//
// Outputs               cpOutputTimerLO - pointer to rounded timer Low byte
//                     cpOutputTimerHI - pointer to rounded timer High byte
//
// Edit History        [0]   3/18/97     BJT   Created
//                     [1]     8/20/97   BJT   Add parameters since we have more
//                        memory with the C505
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/
void TimerObjectRoundUp (unsigned char cpInputTimerLO, unsigned char cpInputTimerHI, unsigned char *cpOutputTimerLO,
						 unsigned char *cpOutputTimerHI)
{
	/*
	// Store the input timer value in the output location
	*/
	*cpOutputTimerLO = cpInputTimerLO;
	*cpOutputTimerHI = cpInputTimerHI;

	/*
	// Round the EPR Up
	*/
	if (cpInputTimerLO & 0x07)
	{
		*cpOutputTimerLO = (cpInputTimerLO + 0x07) & 0xf8;

		if (*cpOutputTimerLO == 0)
			(*cpOutputTimerHI)++;

		/*
		// If overflow, then make highest possible value
		*/
		if ((*cpOutputTimerLO == 0) && (*cpOutputTimerHI == 0))
		{
			*cpOutputTimerLO = 0xf8;
			(*cpOutputTimerHI)--;
		}
	}
}

//***********************************************************************
//
// Function                  TimerObjectISR()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function is the Core Timer Interrupt Service Routine.
//            It does nothing except increment a variable that is checked in the foreground.
//
//            Note : This routine is tied to the Real Time Interrupt (RTI) of the Core Timer (CTIME)
//
// Functional Description (changed AG): This function is an interrupt service routine for TIMER 2.
//               It should never be called by a C or assembly function.
//               It will be executed automatically when TIMER 2 overflows.
//
// Inputs      None
//
// Outputs               None
//
// Edit History
//         [11]     12/11/98   CRS     Moving bulk of processing to foreground (TimerObjectSvcTimer)
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DisableInterrupts (void);
void EnableInterrupts (void);

void TimerObjectISR (void)
{
	TimerTicks++;
	TimerObjectTick++; //TODO to check 8 ms timer based logic
	DisableInterrupts ();
	EnableLineObjectProcess ();
	EnableInterrupts ();
}
//***********************************************************************
//
// Function                  TimerObjectSvcTimer()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function services the core firmware timer.  It's purpose is to scan through
//            the connection instances and decrement any active connection timers that are found.
//            If a timer expires the inactivityTimeout() routine will be called.  If the Device is not
//            currently On-line then call the duplicate MAC ID check routine to see if we can go On-line
//            with our MAC ID.
//
// Functional Description (changed AG): This function is an interrupt service routine for TIMER 0.  It should never be
// called
//               by a C or assembly function.  It will be executed automatically when TIMER 0 overflows.
//
// Inputs      DNG_cHealthState - Device Health State
//         DNG_bDeviceOnLine - Device On-line
//         DNG_astCnxns_ram - Array of Connection Objects
//
// Outputs               None
//
// Returns          Value of TimerTicks
//
// Edit History
//         [11]     12/11/98   CRS     Moving bulk of processing to foreground (TimerObjectSvcTimer)
//		 [12]	13May99	dsw	added call to DIPObjectServiceTimer
//         [13]     10Nov99   dsw  added BusEnableIn filter
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

extern void yield_thread (void);

unsigned long uptime = 0;
unsigned char TimerObjectSvcTimer (void)
{
	yield_thread ();

	if (!TimerTicks) // get out if no ticks have occurred
		return (0);

	uptime++;



	/*
	// Every 8ms check conenction timers and update Uon timers and update User Interface
	*/
//	TimerObjectTick = (TimerObjectTick + 1) % 8;
	int timer_elapse_1 = 0;
	timer_elapse_1 = (TimerObjectTick / 8);
	TimerObjectTick = (TimerObjectTick % 8);

	if (!TimerObjectTick || timer_elapse_1)
	{
		static int MB_TimeOut;
	      if(ModbusConfig.timeout != 0)
	      {
	         //			  if ((waiting == 1) && (MB_TimeOut < ModbusConfig.timeout)) MB_TimeOut++;
	         if (waiting == 1)
	         {
	            if (MB_TimeOut < ModbusConfig.timeout) MB_TimeOut++;
	            else
	            {
	               MB_Exception = MODBUS_TIMEOUT_ERROR;
	               MB_TimeOut = 0;
	            }
	         }
	         else
	         {
	            waiting = 0;
	            MB_TimeOut = 0;
	         }
	      }
	      else
	      {
	         waiting = 0;
	         MB_TimeOut = 0;
	      }

	      if ( ASCII_Mode_InterChar_TO_ON ) {
	         Ascii_Mode_InterChar_Time += 8;
	         if ( Ascii_Mode_InterChar_Time >= (unsigned int) ASCII_MODE_INTER_CHAR_TO_INTERVAL ) {
	            ASCII_Mode_InterChar_TO_flg = TRUE;
	         }
	      }

		/*
		// Only check the connection timeouts if not in DUPLICATE MAC ID
		//   or BUSOFF Faults
		*/
		if ((!(UIObjectRAM.cHealthState & (DUPLICATEMACFAIL | CANBUSOFF))) && (FilterBusEnInPin == 0))
		{
			/*
			// Decrement Ack Handler Timer and Inhibit Timer and COS prod trig timer
			*/
			AckHandlerObjectDecrementTime ();

			/*
			// If not On-line then check the duplicate Mac Message timeout
			*/
			if (DeviceNetObjectRAM.DeviceOnLine == FALSE)
			{
				if (UIObjectLEDPowerupDone ())
				{
					if (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud)
					{
						if (++TryCount > 13)
						{
							TryCount = 0;
							// Cycle through the baud rates
							if (DeviceNetObjectRAM.baudrate == 0)
								DeviceNetObjectRAM.baudrate = BAUD_1M;
							else
								DeviceNetObjectRAM.baudrate--;

							InitCAN ();
						}
					}
					else
					{
						/*
						// Decrement the Explicit(C/S) connection timer
						*/
						ConnectionObjectDecrementTimer (EXPLICIT_CNXN);

						/*
						// If Exlicit(C/S) connection timer has timed out see if we
						// can go On-line by checking DUP MAC.
						*/
						if (!ConnectionObjectTimerActive (EXPLICIT_CNXN))
							DeviceNetObjectDupMACCheck ();
					}
				}
			}
			else
			{
				/*
				// Since we are On-line check the connection objects inactivity timers
				*/
				ConnectionObjectVerifyActiveConnections ();

				// check if quickconnect dupmac sequence ongoing
				if (DeviceNetObjectRAM.QuickConnect && DeviceNetObjectRAM.QCTimer)
				{
					// our quickconnect dupmac counter is running
					if (0 == --DeviceNetObjectRAM.QCTimer)
					{
						// time to send out the final dupmac request msg
						DeviceNetObjectRAM.DupMACIDCounter++;
						DeviceNetObjectDupMACRequestSend ();
					}
				}
			}
		}
		// update LED's
		UIObjectLEDUpdate ();
	}
	else
	{
		if ((ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud) && (TimerObjectTick < 5)) //TODO 3 is changed with 5 value
			UIObjectLEDRefresh (); // update LED's
		else
			UIObjectClearLEDs (); // save power - turn off leds
	}

	if (AppObjectInitialized)
		AppObjectServiceTimer (); // any application-specific timer functions

	// This is a filter for the Bus Enable Input Pin - if some noise occurs, we don't want
	// it to trip.  Enable if line is low for EN_BUS_FILTER ms.
	// FilterBusEnInPin = 0; // TODO JSW Emulator HACK

	if (FilterBusEnInPin != 0)
	{
		if (IO_GET_EnLineIn () == 0)
			FilterBusEnInPin--;
		else
			FilterBusEnInPin = EN_BUS_FILTER;
	}

	DISABLE_TIME_INTS;
	TimerTicks--;
	ENABLE_TIME_INTS;

	return (TimerTicks);
}

/*****************************************************************************************/
void TimerObjectInit (void)
{
	TimerObjectTick = 0;
	TimerTicks		= 0;

	TryCount = 0;

	FilterBusEnInPin = EN_BUS_FILTER; // Xms filter (time before enble pin is accepted)
}
