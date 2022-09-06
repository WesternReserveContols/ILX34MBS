//***********************************************************************
// Project Name              DeviceNet
// Module                    DN_UIOBJ.H
//
// Module Description       This Module contains the includes for the
//                     DeviceNet User Interface Object.
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************

#include <dn_def.h>
#include <std.h>

#define MOD_TOGGLE                  0x01  //set to toggle MOD LED RED/GREEN
#define NET_TOGGLE                  0x02  //set to toggle NET LED RED/GREEN
#define MOD_LAST_STATE              0x40  //used internally
#define NET_LAST_STATE              0x80  //used internally


#define MOD_LED_GREEN_BIT 0x01
#define MOD_LED_RED_BIT	  0x02
#define NET_LED_GREEN_BIT 0x04
#define NET_LED_RED_BIT	  0x08
#define APP_LED_BIT_01	  0x10
#define APP_LED_BIT_02	  0x20
#define APP_LED_BIT_03	  0x40
#define APP_LED_BIT_04	  0x80
#define APP_LED_BIT_05	  0x01
#define APP_LED_BIT_06	  0x02
#define APP_LED_BIT_07	  0x04
#define APP_LED_BIT_08	  0x08
#define APP_LED_BIT_09	  0x10
#define APP_LED_BIT_10	  0x20
#define APP_LED_BIT_11	  0x40
#define APP_LED_BIT_12	  0x80

#define MOD_LED_GREEN (UIObjectRAM.cLEDByte1 |= MOD_LED_GREEN_BIT)
#define MOD_LED_RED	  (UIObjectRAM.cLEDByte1 |= MOD_LED_RED_BIT)
#define NET_LED_GREEN (UIObjectRAM.cLEDByte1 |= NET_LED_GREEN_BIT)
#define NET_LED_RED	  (UIObjectRAM.cLEDByte1 |= NET_LED_RED_BIT)
#define CLEAR_MOD_LED (UIObjectRAM.cLEDByte1 |= 0xFC)
#define CLEAR_NET_LED (UIObjectRAM.cLEDByte1 |= 0xF3)

#define APP_LED_01_ON (UIObjectRAM.cAppLEDByte1 |= APP_LED_BIT_01)
#define APP_LED_02_ON (UIObjectRAM.cAppLEDByte1 |= APP_LED_BIT_02)
#define APP_LED_03_ON (UIObjectRAM.cAppLEDByte1 |= APP_LED_BIT_03)
#define APP_LED_04_ON (UIObjectRAM.cAppLEDByte1 |= APP_LED_BIT_04)
#define APP_LED_05_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_05)
#define APP_LED_06_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_06)
#define APP_LED_07_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_07)
#define APP_LED_08_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_08)
#define APP_LED_09_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_09)
#define APP_LED_10_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_10)
#define APP_LED_11_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_11)
#define APP_LED_12_ON (UIObjectRAM.cAppLEDByte2 |= APP_LED_BIT_12)

#define APP_LED_01_OFF (UIObjectRAM.cAppLEDByte1 &= (~APP_LED_BIT_01))
#define APP_LED_02_OFF (UIObjectRAM.cAppLEDByte1 &= (~APP_LED_BIT_02))
#define APP_LED_03_OFF (UIObjectRAM.cAppLEDByte1 &= (~APP_LED_BIT_03))
#define APP_LED_04_OFF (UIObjectRAM.cAppLEDByte1 &= (~APP_LED_BIT_04))
#define APP_LED_05_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_05))
#define APP_LED_06_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_06))
#define APP_LED_07_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_07))
#define APP_LED_08_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_08))
#define APP_LED_09_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_09))
#define APP_LED_10_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_10))
#define APP_LED_11_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_11))
#define APP_LED_12_OFF (UIObjectRAM.cAppLEDByte2 &= (~APP_LED_BIT_12))

#define APP_LED_01_SOLID (UIObjectRAM.cSolidByte1 |= APP_LED_BIT_01)
#define APP_LED_02_SOLID (UIObjectRAM.cSolidByte1 |= APP_LED_BIT_02)
#define APP_LED_03_SOLID (UIObjectRAM.cSolidByte1 |= APP_LED_BIT_03)
#define APP_LED_04_SOLID (UIObjectRAM.cSolidByte1 |= APP_LED_BIT_04)
#define APP_LED_05_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_05)
#define APP_LED_06_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_06)
#define APP_LED_07_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_07)
#define APP_LED_08_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_08)
#define APP_LED_09_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_09)
#define APP_LED_10_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_10)
#define APP_LED_11_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_11)
#define APP_LED_12_SOLID (UIObjectRAM.cSolidByte2 |= APP_LED_BIT_12)

#define APP_LED_01_BLINK (UIObjectRAM.cSolidByte1 &= (~APP_LED_BIT_01))
#define APP_LED_02_BLINK (UIObjectRAM.cSolidByte1 &= (~APP_LED_BIT_02))
#define APP_LED_03_BLINK (UIObjectRAM.cSolidByte1 &= (~APP_LED_BIT_03))
#define APP_LED_04_BLINK (UIObjectRAM.cSolidByte1 &= (~APP_LED_BIT_04))
#define APP_LED_05_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_05))
#define APP_LED_06_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_06))
#define APP_LED_07_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_07))
#define APP_LED_08_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_08))
#define APP_LED_09_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_09))
#define APP_LED_10_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_10))
#define APP_LED_11_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_11))
#define APP_LED_12_BLINK (UIObjectRAM.cSolidByte2 &= (~APP_LED_BIT_12))

#define SHIFTL_LEDBYTE1 (UIObjectRAM.cLEDByte1 <<= 1)
#define SHIFTL_LEDBYTE2 (UIObjectRAM.cLEDByte2 <<= 1)

// should define some blink/solid macros here

/*
// User Interface Object
*/
typedef struct
{
	unsigned char num_app_LEDs;
	unsigned char cStartupLEDState;
	unsigned char cLEDToggle; /* on or off */
	unsigned char cFlashTimer;
	unsigned char cFlashTimerVal;
	unsigned char cHealthState;
	unsigned char cLEDFlashTime;
	unsigned char cLEDByte1;
	unsigned char cLEDByte2;
	unsigned char cAppLEDByte1; // only using high 8 bits - 1 is flash
	unsigned char cAppLEDByte2; // only using low 8 bits
								// could name the following fields 'not-blinking'
	unsigned char cSolidByte1;	// only using high 8 bits - 1 is flash
	unsigned char cSolidByte2;	// only using low 8 bits
} UIObjectStructure;

extern UIObjectStructure UIObjectRAM;

extern void			 UIObjectInit ();
extern unsigned char UIObjectLEDPowerupDone ();
extern void			 UIObjectNetLEDOnceRedGreen ();
extern void			 UIObjectLEDUpdate ();
extern void			 UIObjectClearLEDs ();
extern unsigned char UIObjectLEDFlashTimer ();
extern void			 UIObjectSetMode ();
extern void			 UIObjectShowError ();
extern void			 UIObjectProcessMenu ();
