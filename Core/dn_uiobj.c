#include <app_objs.h>
#include <c505c.h>
#include <dn_dnobj.h>
#include <dn_eeprm.h>
#include <dn_init.h>
#include <dn_msgob.h>
#include <dn_uiobj.h>
#define SHIFTING_LEDS_SUCK
#define FLASHTIME 62 // 500 ms  (at 8ms resolution)

void UIObjectLedDrive (uchar ledbyte1, uchar ledbyte2);
void UIObjectLEDRefresh (void);

UIObjectStructure UIObjectRAM;
unsigned char ToggleLEDS = 0;

//***********************************************************************
//
// Function                  UIObjectLedDrive()
//
// Author                  Ron Bavol
// Date                      11/11/98
//
// Functional Description    turn on/off leds
//
// Inputs               byte containing desired led state
//
// Outputs               None
//
// Returns               None
//
// Edit History             [0]   11/11/98  RAB     Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

void UIObjectLedDrive (uchar ledbyte1, uchar ledbyte2)
{
	ledbyte2 = ledbyte2; // suppress error... we do not use status bytes on the a232(232asc)

	IO_SET_LEDModRed ((ledbyte1 & MOD_LED_RED_BIT) ? 1 : 0);
	IO_SET_LEDModGreen ((ledbyte1 & MOD_LED_GREEN_BIT) ? 1 : 0);
	IO_SET_LEDNetworkRed ((ledbyte1 & NET_LED_RED_BIT) ? 1 : 0);
	IO_SET_LEDNetworkGreen ((ledbyte1 & NET_LED_GREEN_BIT) ? 1 : 0);

	/*
	 * Replaced with HAL functions
	IO_MOD_RED ((ledbyte1 & MOD_LED_RED_BIT) ? 1 : 0);
	IO_MOD_GREEN ((ledbyte1 & MOD_LED_GREEN_BIT) ? 1 : 0);
	IO_NET_RED ((ledbyte1 & NET_LED_RED_BIT) ? 1 : 0);
	IO_NET_GREEN ((ledbyte1 & NET_LED_GREEN_BIT) ? 1 : 0);
	*/
}

//***********************************************************************
//
// Function                  UIObjectInit()
//
// Author                  Brian J. Taylor
// Date                      8/26/97
//
// Functional Description    This function initialized the variables used
//                     in the DeviceNet user interface.
//
// Inputs               None
//
// Outputs               None
//
// Returns               None
//
// Edit History             [0]   8/26/97  BJT     Created
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void UIObjectInit ()
{
	/*
	// Set our current health state to Power up
	// Set for LED test of a Green / Red Flash.
	// Set program mode to False
	*/
	// health state is powerupfail if ram check failed
	if (POWERUP_FAIL != UIObjectRAM.cHealthState)
		UIObjectRAM.cHealthState = POWERUP;

	UIObjectRAM.cLEDByte1	 = 0;
	UIObjectRAM.cLEDByte2	 = 0;
	UIObjectRAM.cAppLEDByte1 = 0;
	UIObjectRAM.cAppLEDByte2 = 0;
	UIObjectRAM.cSolidByte1	 = 0xF0;
	UIObjectRAM.cSolidByte2	 = 0x0F;

	if (DeviceNetObjectRAM.QuickConnect)
	{
		UIObjectRAM.cStartupLEDState = 0xff;
		// set timers for .5 s
		UIObjectRAM.cFlashTimerVal = FLASHTIME;
	}
	else
	{
		UIObjectRAM.cStartupLEDState = 0;
		// set timers for .25 s  - will be set to .5s after startup led sequence
		UIObjectRAM.cFlashTimerVal = FLASHTIME >> 1;
	}
	UIObjectRAM.cFlashTimer = 0; // let's get the startup led sequence underway
	UIObjectRAM.cLEDToggle	= 0;
}

//***********************************************************************
//
// Function                  UIObjectLEDPowerupDone()
//
// Author                  CRSchumaker
// Date                      10/28/98
//
// Functional Description    Return whether or not LED powerup sequence
//                     is finished (see ch8, vol1 of DNet spec)
//
// Inputs               None
//
// Outputs               None
//
// Returns               1 if done, 0 if not.
//
// Edit History             [0]   10/28/98  CRS     Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

unsigned char UIObjectLEDPowerupDone ()
{
	return (0xFF == UIObjectRAM.cStartupLEDState);
}

//***********************************************************************
//
// Function                  UIObjectNetLEDOnceRedGreen()
//
// Author                  CRSchumaker
// Date                      12/04/98
//
// Functional Description    Flash mod led red and then green for 250ms
//                           for each flash.  This will only happen during
//                           group 4 faulted node recovery.
//
// Inputs               None
//
// Outputs               None
//
// Returns               None
//
// Edit History             [0]   12/04/98  CRS     Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

void UIObjectNetLEDOnceRedGreen ()
{
	UIObjectRAM.cFlashTimer		 = 0;  // start the sequence pronto
	UIObjectRAM.cStartupLEDState = 10; // indicates that it's not for startup
	// set timers for .25 s  - will be set to .5s afterwards
	UIObjectRAM.cFlashTimerVal = FLASHTIME >> 1;
}

//***********************************************************************
//
// Function                  UIObjectLEDUpdate()
//
// Author                  Brian J. Taylor
// Date                      3/7/94
//
// Functional Description    This function is used to setup the Health LED
//                     depending on the current LED Function and
//                     the number of flashes requested.  This function
//                     writes directly to the LED via the parallel
//                     port on the 68HC05 (PORT_A).
//
// Inputs               DNG_cLEDFunction - LED function request
//                     DNG_cLEDFlash - Number of times to flash LED
//
// Outputs               PORT_A - Parallel IO Port A :
//                           Red LED (Bit 1), Green LED (Bit 2)
//
// Edit History             [0]   3/7/94     BJT     Created
//                     [1]     6/16/94   BJT      Added Flashing LED Red
//                           support to DUP MAC ID no ACK errors indicator.
//                     [2]     6/27/94   BJT      Added memory error to set
//                           LED to RED.
//                     [3]     7/28/94   BJT      To save ROM. Only check for
//                           health state once in if statement since
//                           the values are all the same for each LED
//                           state.
//                     [4]     9/28/94   BJT      ifdef out not used
//                           in DeviceLink.
//                     [5]     9/30/94   BJT      Fix not doing red/green
//                           flash at powerup caused by [4].
//                     [6]    11/9/94   BJT      Have LED flash red if timeout
//                        occurs and a connection is still active. (EPRERRORFLAG)
//                        From DeviceNet Chnage Review Board Meeting
//                        on 11/8/94.
//                     [7]      6/22/95   BJT      Added support for Autobaud.
//                           Flash red while in autobaud.
//                     [8]   3/6/98   BJT   Flash Output LED if coil broken on prox
//                     [9]   4/5/98   BJT   Use timerOutput to determine state of output LED
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void UIObjectLEDUpdate ()
{
	/*
	// Is it time to flash the LED
	*/
	if ((UIObjectRAM.cLEDFlashTime = UIObjectLEDFlashTimer ()))
	{
		UIObjectRAM.cLEDToggle = ~(UIObjectRAM.cLEDToggle); // want 0 or 0xFF

		if (UIObjectRAM.cLEDToggle == 0xFF)
			KICK_WDOG ();
	}

	/*
	// Update the LED's with any new data from Program Mode changes or
	// handle our powerup LED test.
	*/
	if (TRUE == UIObjectRAM.cLEDFlashTime)
	{
		if (POWERUP_FAIL == UIObjectRAM.cHealthState)
		{
			UIObjectRAM.cLEDByte1 &= 0x00; // clear out all bits
			MOD_LED_RED;
		}
		else if (UIObjectRAM.cStartupLEDState != 0xFF && DUPLICATEMACFAIL != UIObjectRAM.cHealthState)
		{
			switch (UIObjectRAM.cStartupLEDState)
			{
			case 0: // mod led green
				MOD_LED_GREEN;
				break;
			case 1: // mod led red
				SHIFTL_LEDBYTE1;
				break;
			case 8:
				if (UIObjectRAM.num_app_LEDs > 4)
				{
					UIObjectRAM.cLEDByte1 = 0x01; // set mod led green only
					UIObjectRAM.cLEDByte2 = 0x01; // get next led byte going
					break;
				}
				// break intentially removed - want to fall through
				//  if appleds < 4
			case 12:							  // end of LED startup sequence
				UIObjectRAM.cLEDByte1		 = 1; // mod led green
				UIObjectRAM.cLEDByte2		 = 0;
				UIObjectRAM.cStartupLEDState = 0xFF;
				UIObjectRAM.cFlashTimerVal = UIObjectRAM.cFlashTimer = FLASHTIME;
				break;
			default:
				if ((UIObjectRAM.num_app_LEDs > 4) && (UIObjectRAM.cStartupLEDState > 8))
				{
					SHIFTL_LEDBYTE2;
				}
				else
				{
					UIObjectRAM.cLEDByte1 &= 0xFE; // clr low bit before shifting
					SHIFTL_LEDBYTE1;
					MOD_LED_GREEN;
				} // these braces look funny - but leave them alone
				break;
			}

			if (0xFF != UIObjectRAM.cStartupLEDState)
				UIObjectRAM.cStartupLEDState++; // increment state
		}

		/*
		// Do Target and Diagnostic state LED update if not in program Mode
		*/
		else
		{
			UIObjectRAM.cLEDByte1 &= 0xF0; // clear out low 4 bits (for mod/net leds)

		    if(ToggleLEDS && MOD_TOGGLE)
		    {
		         if(ToggleLEDS & MOD_LAST_STATE) MOD_LED_GREEN;
		         else MOD_LED_RED;

				 ToggleLEDS ^= MOD_LAST_STATE;
				 UIObjectRAM.cFlashTimerVal = FLASHTIME;
			 }
		     else MOD_LED_GREEN;	   // Preload MOD_LED


			if (DUPLICATEMACFAIL == UIObjectRAM.cHealthState)
			{
				// we're in dupmac failure state.

				if (UIObjectRAM.cStartupLEDState < 10)
				{
					// startup led sequence was still in progress

					NET_LED_RED;
					UIObjectRAM.cStartupLEDState = 0xFF;
					UIObjectRAM.cFlashTimerVal = UIObjectRAM.cFlashTimer = FLASHTIME;
				}
				else if (UIObjectRAM.cStartupLEDState < 20)
				{
					// now in flash red/green once sequence
					if (10 == UIObjectRAM.cStartupLEDState)
					{
						NET_LED_RED;
						UIObjectRAM.cStartupLEDState++;
					}
					else if (11 == UIObjectRAM.cStartupLEDState)
					{
						NET_LED_GREEN;
						UIObjectRAM.cStartupLEDState++;
					}
					else
					{
						NET_LED_RED;
						UIObjectRAM.cStartupLEDState = 0xFF;
						UIObjectRAM.cFlashTimerVal	 = FLASHTIME;
					}
				}
				else
					NET_LED_RED;
			}

			else if (UIObjectRAM.cLEDToggle)
			{
				if (NORMALOPERATION == UIObjectRAM.cHealthState		   // Online is always Green
					|| GRP2IDSETALLOCATED == UIObjectRAM.cHealthState) // Online is always Green
				{
					if((ToggleLEDS & NET_TOGGLE) == 0) NET_LED_GREEN;
				}
				else
				{
					if (EPRERRORFLAG == UIObjectRAM.cHealthState || CANBUSOFF == UIObjectRAM.cHealthState)
						NET_LED_RED;
					// else net_led_off - no required
				}
			}
			else
			{
				/*  CRS - if config data potentially missing , add this code
							if (NoCfg != 0)
							   ledval = LED_Mod_X   ;
							else
				*/
				if (CANBUSOFF == UIObjectRAM.cHealthState)
					NET_LED_RED;
				else
				{
					if (GRP2IDSETALLOCATED == UIObjectRAM.cHealthState)
						if(ToggleLEDS && NET_TOGGLE)
						{
							if(ToggleLEDS & NET_LAST_STATE) NET_LED_GREEN;
							else NET_LED_RED;
							ToggleLEDS ^= NET_LAST_STATE;
							UIObjectRAM.cFlashTimerVal = FLASHTIME;
					    }
					    else NET_LED_GREEN;
					// else net_led_off - no required
				}
			}
		}
	}

	// must now check if a group4 'who' counter is running.
	if ((0xFFFF != MessageObjectRAM.Grp4WhoCntr) && MessageObjectRAM.Grp4WhoCntr)
		MessageObjectRAM.Grp4WhoCntr--;

	// each time thru, let's update the application leds
	if (0xFF == UIObjectRAM.cStartupLEDState)
	{
		// each time thru, let's update the application leds
		UIObjectRAM.cLEDByte1 &= 0x0F; // clear high 4 bits

		UIObjectRAM.cLEDByte1 |= (UIObjectRAM.cAppLEDByte1 & (UIObjectRAM.cLEDToggle | UIObjectRAM.cSolidByte1));

		// used to be ifdef here (num_app_leds > 4)
		UIObjectRAM.cLEDByte2 = (UIObjectRAM.cAppLEDByte2 & (UIObjectRAM.cLEDToggle | UIObjectRAM.cSolidByte2));
		// used to be endif here
	}

	// now push it out the port
	UIObjectLedDrive (UIObjectRAM.cLEDByte1, UIObjectRAM.cLEDByte2);
}
//***********************************************************************
//
// Function                  UIObjectLEDRefresh()
//
// Author                  CRSchumaker
// Date                      8/2/99
//
// Functional Description   Drive leds with current values.
//
// Inputs               None
//
// Outputs              None
//
// Edit History             [0]   8/2/99  CRS     Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//***********************************************************************/
void UIObjectLEDRefresh (void)
{
	UIObjectLedDrive (UIObjectRAM.cLEDByte1, UIObjectRAM.cLEDByte2);
}
//***********************************************************************
//
// Function                  UIObjectClearLEDs()
//
// Author                  CRSchumaker
// Date                      5/5/99
//
// Functional Description   Turn off leds in order to save power.
//
// Inputs               None
//
// Outputs              None
//
// Edit History             [0]   5/5/99  CRS     Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//***********************************************************************/
void UIObjectClearLEDs (void)
{
	UIObjectLedDrive (0, 0);
}

//***********************************************************************
//
// Function                  UIObjectLEDFlashTimer()
//
// Author                  Brian J. Taylor
// Date                      3/7/94
//
// Functional Description    This function is the LED flash timer which
//                     determines how long to keep the LED ON or
//                     OFF.  This is tied to the 68HC05 RTI which is
//                     configured in DN_INIT.C (InitMPU()) to be
//                     ~ 8 ms. The current flash time is set to 4
//                     loops which would be ~ 32 ms.
//
// Inputs               None
//
// Outputs               Return Code -    TRUE : Timer done
//                                 FALSE : Timer Active
//
// Edit History             [0]   3/7/94  BJT     Created
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

unsigned char UIObjectLEDFlashTimer ()
{
	/*
	// This timer will flash on/off the LED for the amount specified
	// by FLASHTIME * RTI Rate.
	*/
	if (UIObjectRAM.cFlashTimer == 0)
	{
		/*
		// Reset timer for next flash and return TRUE for time done
		*/
		UIObjectRAM.cFlashTimer = UIObjectRAM.cFlashTimerVal;
		return (TRUE);
	}
	else
	{
		/*
		// Timer not done so decrement and return FALSE
		*/
		UIObjectRAM.cFlashTimer--;
		return (FALSE);
	}
}
