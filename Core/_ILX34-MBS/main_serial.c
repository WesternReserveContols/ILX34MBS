#include "bsp_main.h"
#include "dsc.h" // debug com port
#include "dn_init.h"
#include "c505c.h"
#include "dn_uiobj.h"
#include "dn_eeprm.h"
#include "dn_tmobj.h"
#include "dn_dnobj.h"
#include "app_objs.h"
#include "dn_msgob.h"
#include "dn_cnobj.h"
#include "enbl_obj.h"
#include "dn_ahobj.h"
#include "mbport.h"

uchar		 AppObjectInitialized;
BOOL		 AppObjectCOS_Bit;
BOOL		 COSACKRcvd;
extern uchar GMMNewMACID;

static void COS_Code (void);

unsigned char SoftReset = 0;    // Rick_TEST Bug9 8/10
#define STARTUP1()  goto SOFT_RESET;


//***********************************************************************
//
// Function                  Main()
//
// Author                    Brian J. Taylor
// Date                      3/31/94
//
// Functional Description    This function is the main routine for the IDN 9000 software.
//            It will setup the MCAN and Timer ISR and call the Reset ISR.
//            The function will then loop forever since all action is initiated
//            by the Interrupt Service Routines.
//
//      NOTE: The HI-TECH C compiler forces the function powerup in the Vector Table for the reset
//      function.  The powerup function just jumps to main(), so we will call the RESET_ISR function from main.
//
// Inputs      None
//
// Outputs      Set CAN and Core Timer vectors in ISR Table
//                           ($1FF0 - $1FFF)
//
// Edit History
//         [10]  8/28/97   BJT   Init User Interface, EEPROM and application Objects.
//         [11]  08Nov99   dsw   Changed a while loop to wait for EnableIn Filter to go low
//         [12]  13Jun00   dsw   Removed baudrate save to EEPROM after autobaud
//         [13]  23Jun00   dsw   Fixed GMM start-up (in Pre-Soft Reset area)
//                               Added 'write 0 before 1' for enable_out pin
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/


void main_serial (void)
{
SOFT_RESET:

	/*
	// Disable interrupts
	*/

	INT_DISABLE;

	/*
	// Initialize the Microprocessor
	*/
	InitC505 ();

	IO_SET_EnLineOut (0); // write a zero before one to force hard pull-up
	IO_SET_EnLineOut (1); // prevent neighbor from dupmaccing

	KICK_WDOG ();

	// Initialize EEPROM
	EEPROMObjectInit ();

	KICK_WDOG ();

	DeviceNetObjectInit (); // init dnet object

	KICK_WDOG ();

	// prevent appobject timer routines from running
	AppObjectInitialized = FALSE;

	/*
	// Initialize the User Interface
	*/
	UIObjectInit ();
	AppObjectInitAppLEDs (); // how many app leds does the product have

	MessageObjectRAM.bCommParamChange = FALSE;
	EnableLineObjectInit ();

	KICK_WDOG ();

	// Enable interrupts
	INT_ENABLE;

	if (!DeviceNetObjectRAM.QuickConnect)
	{
		while (!UIObjectLEDPowerupDone ()) // wait for led powerup sequence to finish
		{
			TimerObjectSvcTimer (); // allow led's to flash

			if (MessageObjectRAM.bCommParamChange)
				goto PRE_SOFT_RESET;
		}
	}

	KICK_WDOG ();

	/*
	// Initialize DeviceNet communications
	*/
	if (POWERUP_FAIL != UIObjectRAM.cHealthState)
		InitDeviceNet ();

	KICK_WDOG ();

	/*
	// Initialize application objects
	*/
#ifdef SIM_CONSUME
	InitApplicationObjects ();
	AppObjectInitialized = TRUE;
#endif

	// wait in while loop to autobaud and for EnableIn to be enabled
	while (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud
		   || (FilterBusEnInPin != 0)) // don't send dupmac request until bus_en goes low
	{
		TimerObjectSvcTimer ();		 // allow led's to flash
		MessageObjectCheckInPktQ (); // for autobauding and dupmac detection

		// added this to allow a module to be "softly" reset
		// via the PointBus line.
		if (MessageObjectRAM.bCommParamChange)
			goto PRE_SOFT_RESET;
	};

	if (MessageObjectRAM.bCommParamChange)
		goto PRE_SOFT_RESET;

	/*
	// Initialize application objects
	*/
	InitApplicationObjects ();
	AppObjectInitialized = TRUE;


DO_DUPMACS:
	if (DUPLICATEMACFAIL != UIObjectRAM.cHealthState)
	{
		//  Reset DUP MAC ID counter
		DeviceNetObjectRAM.DupMACIDCounter = 0;
		DeviceNetObjectDupMACRequestSend ();
		// last routine called starts the dupmac timers without knowing that
		// the message has actually been transmitted.  BAD!!!!
		// They are set correctly below.
		ConnectionObjectRAM[EXPLICIT_CNXN].timerId[1] = 0;
		ConnectionObjectRAM[EXPLICIT_CNXN].timerId[0] = 0;

		// check if we're in quick connect mode
		//   and haven't gone BUSOFF
		if (DeviceNetObjectRAM.QuickConnect && (!(CANBUSOFF & UIObjectRAM.cHealthState)))
		{
			DeviceNetObjectRAM.DeviceOnLine = TRUE;
			DeviceNetObjectRAM.QCTimer		= 125;
		}

		// wait for message to go out before telling neighbor to wake up
		// Disable for emulation. rab.
		// while ((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) != IS_CLEAR_TXRQ)
		while (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_PENDING)
		{
			// keep kicking the dog, in case no other nodes present to CAN-ack msg
			TimerObjectSvcTimer ();

			// added this to allow a module to be "softly" reset
			// via the PointBus line.
			if (MessageObjectRAM.bCommParamChange)
				goto PRE_SOFT_RESET;
		};

		// start the dupmac timers now  (see note above about these timers)
		ConnectionObjectRAM[EXPLICIT_CNXN].timerId[1] = DUP_MAC_TIMEOUT_LO;
		ConnectionObjectRAM[EXPLICIT_CNXN].timerId[0] = DUP_MAC_TIMEOUT_HI;
	}
	DeviceNetObjectRAM.FirstDupMACSent = TRUE;
	EnableLineObjectEnOutLo ();

	COSACKRcvd = FALSE;

GMM_RESTART:

	/*
	// Loop forever.......
	*/

	for (;;)
	{

		if(SoftReset == SOFT_RESET_ACTIVE)
		{
			SoftReset = 0;
			STARTUP1();  //Rick_TEST Bug9    goto SOFT_RESET;
		}
		// nonblocking write to eeprom for any outstanding RAM values
		EEPROMObjectWriteRAMtoEE (0);

		TimerObjectSvcTimer ();

		MessageObjectCheckPktQs (); // check explicit input and output queues

		TimerObjectSvcTimer ();

		main_port_serial();
		// If our master changed a communication parameter then dellocate current
		// connections and reinitialize communications with new configuration
		if (MessageObjectRAM.bCommParamChange)
		{
			// let's make sure the outputs are turned off.
			ConnectionObjectRAM[POLL_CNXN].state   = CNXN_NONEXISTENT;
			ConnectionObjectRAM[STROBE_CNXN].state = CNXN_NONEXISTENT;
			AppObjectCnxnNotRunning (POLL_CNXN);
			AppObjectCnxnNotRunning (STROBE_CNXN);

			goto PRE_SOFT_RESET;
		}

		// check if we're waiting to send dupmac request to neighbor via enable line
		if ((EnableObjectRAM.xmt_state > STATE_ACTIVE)
			//	&& ((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) == IS_CLEAR_TXRQ))
			&& (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_NOT_PENDING))
		{
			if (STATE_WAIT_TO_SND == EnableObjectRAM.xmt_state)
			{
				// then send out our dupmac request
				MessageObjectDupMACMessageSend (REQUEST);
				EnableObjectRAM.xmt_state = STATE_WAIT_SENT;
			}
			else if (STATE_WAIT_SENT == EnableObjectRAM.xmt_state)
			{
				EnableLineObjectPropMsg (); // send already loaded msg over PointBus
			}
		}

		// if we're in dupmac failure state
		if (DUPLICATEMACFAIL == UIObjectRAM.cHealthState)
		{
			if (!MessageObjectRAM.Grp4WhoCntr)
			{
				// it's time to send out a who response message
				MessageObjectRAM.Grp4WhoCntr = 0xFFFF; // turn off counter
				MessageObjectSendGrp4WhoMessage ();
			}
		}
		else
		{
			// call application code
			AppObjectMonitorIO ();

			// check if we need to send COS/Cyclic message
			COS_Code ();
		}
	}

PRE_SOFT_RESET:
	DSC_Writes (DSC_LEVEL_INFO, "PRE_SOFT_RESET\n\r");
	uint8_t abauding				  = MessageObjectRAM.bCommParamChange; // should probably rename this
	MessageObjectRAM.bCommParamChange = FALSE;

	// Blocking write to save any pending writes to EEPROM
	EEPROMObjectWriteRAMtoEE (1);

	if (TRUE == abauding)
	{
		DSC_Writes (DSC_LEVEL_INFO, "Param Change RESET\n\r");
		// Make sure we send out response before reset (unless we're busoff).
		// We shouldn't have to worry about kickin the dog here.
		// We've got a few hundred ms.
		while ((STATE_IDLE != EnableObjectRAM.xmt_state)
			   //	   || (((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) != IS_CLEAR_TXRQ)
			   || ((CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_PENDING)
				   && (!(CANBUSOFF & UIObjectRAM.cHealthState))))
		{
			TimerObjectSvcTimer ();
		}

		UIObjectRAM.cHealthState = DEVICEOFFLINE;
		// changed er 12/28/01 ... we need to initialize the ram again!
		//
		// Actually doing a full NVIC System Reset here to reset all threads
		// vTaskDelay (50);
		HAL_NVIC_SystemReset ();
		goto SOFT_RESET;
	}
	else
	{

		if (CHANGED_COME_UP_FAST == abauding)
		{
			DSC_Writes (DSC_LEVEL_INFO, "CHANGED COME UP FAST RESET\n\r");
			UIObjectRAM.cHealthState = DEVICEOFFLINE;
			InitCAN ();
			goto DO_DUPMACS;
		}
		else // CHANGED_NO_DUPMAC - for GenericMasterMode ONLY!!!!
		{

			KICK_WDOG ();

			// wait until msg to neighbor has been transmitted
			while (STATE_IDLE != EnableObjectRAM.xmt_state)
			{
				TimerObjectSvcTimer ();
			}

			// setup GMM defaults
			DeviceNetObjectRAM.macId	 = GMMNewMACID;
			DeviceNetObjectRAM.baudrate	 = BAUD_1M;
			DeviceNetObjectRAM.bAutoBaud = ABAUD_NOTAUTOB;
			// now save data to EEPROM
			EEPROMObjectWriteAndUpdate (NVS_MAC_ID, GMMNewMACID);
			EEPROMObjectWriteAndUpdate (EE_Fast_Start, QUICK_CONN_ENABLED);
			EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, BAUD_1M);
			EEPROMObjectWriteAndUpdate (EE_AutoBaud, 0);

			IO_SET_CanEnable (1); // just in case we were autobauding

			InitDeviceNet ();
			// CAN ints re-enabled within initcan()
			InitApplicationObjects ();
			KICK_WDOG ();
			// send out dupmac RESPONSE message to knock
			// offline any modules that might have our new MACID
			MessageObjectDupMACMessageSend (RESPONSE);

			// wait for msg to to out on the wire
			// while ((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) != IS_CLEAR_TXRQ)
			while (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_PENDING)
			{
				TimerObjectSvcTimer ();
			}

			KICK_WDOG ();
			DeviceNetObjectRAM.DeviceOnLine	   = TRUE;
			UIObjectRAM.cHealthState		   = NORMALOPERATION;
			DeviceNetObjectRAM.DupMACIDCounter = 0;
			DeviceNetObjectRAM.busOffCntr	   = 0;
			DeviceNetObjectRAM.QCTimer		   = 0;

			AppObjectInitialized = TRUE; // goes with InitApplicationObjects() called before

			goto GMM_RESTART;
		}
	}
}
//***********************************************************************
static void COS_Code (void)
{
	if ((CNXN_ESTABLISHED == ConnectionObjectRAM[COS_CNXN].state) && MessageObjectCOSMsgObjAvailable ())
	{
		// check for ack retry of data
		if ((!(DeviceNetObjectRAM.AllocedCnxns & ACK_SUPPRESSION))
			&& (!(AckHandlerObjectRAM.AckTimer[0] || AckHandlerObjectRAM.AckTimer[1]))
			&& (AckHandlerObjectRAM.RetryCount))
		{
			// if inhibited, send old data, else send new data
			if (AckHandlerObjectRAM.InhibitTimer[0] || AckHandlerObjectRAM.InhibitTimer[1])
				MessageObjectSendCOSMessage (OLD_COS_DATA, NO_RST_TIMRS);
			else
			{
				INT_DISABLE;
				AppObjectCOS_Bit = 0;
				INT_ENABLE;
				MessageObjectSendCOSMessage (NEW_COS_DATA, NO_RST_TIMRS);
			}

			return;
		}

		// check for sending of COS data
		INT_DISABLE;
		if ((DeviceNetObjectRAM.AllocedCnxns & COS_MSG) // not cyclic
			&& (AppObjectCOS_Bit))
		{
			// if not inhibited and not waiting for ack
			if ((!(AckHandlerObjectRAM.InhibitTimer[0] || AckHandlerObjectRAM.InhibitTimer[1]))
				&& (!(AckHandlerObjectRAM.AckTimer[0] || AckHandlerObjectRAM.AckTimer[1])))
			{
				AppObjectCOS_Bit = 0;
				INT_ENABLE;
				MessageObjectSendCOSMessage (NEW_COS_DATA, RESET_TIMERS);
				return;
			}
		}
		INT_ENABLE;

		// check for sending of cyclic data
		//  - if epr not zero and trigger time expired
		if ((ConnectionObjectRAM[COS_CNXN].expPacketRate[0] || ConnectionObjectRAM[COS_CNXN].expPacketRate[1])
			&& (!(AckHandlerObjectRAM.XmtTrigTimer[0] || AckHandlerObjectRAM.XmtTrigTimer[1])))
		{
			INT_DISABLE;
			AppObjectCOS_Bit = 0;
			INT_ENABLE;
			MessageObjectSendCOSMessage (NEW_COS_DATA, RESET_TIMERS);
		}
	}

	if (COSACKRcvd)
	{
		// Restart timer since ack is activity
		ConnectionObjectStartInactivity (COS_CNXN);

		// Clear retry counter
		AckHandlerObjectRAM.RetryCount	= 0;
		AckHandlerObjectRAM.AckTimer[0] = 0;
		AckHandlerObjectRAM.AckTimer[1] = 0;
		COSACKRcvd						= FALSE;
	}
}
//***********************************************************************
