#include "serial_config.h"
#include <app_objs.h>
#include <c505c.h>
#include <dn_ahobj.h>
#include <dn_cnobj.h>
#include <dn_dnobj.h>
#include <dn_eeprm.h>
#include <dn_idobj.h>
#include <dn_init.h>
#include <dn_msgob.h>
#include <dn_tmobj.h>
#include <dn_uiobj.h>
#include <enbl_obj.h>
#include <nvs_obj.h>

//***********************************************************************
//
// Function                    InitCAN()
//
// Author                    Brian Taylor
// Date                      9/22/97
//
// Functional Description    This function is responsible for initializing the MCAN control registers.
//
// Inputs            DeviceNetObjectRAM - DeviceNet Object
//
// Outputs           None
//
// Returns      None
//
// Edit History     [0]   B. Taylor   09/22/97   Created
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void InitCAN ()
{
	CP_Set_Init (1);
	CP_Set_SIE (0);
	CP_Set_MIE (0);

	if (DeviceNetObjectRAM.baudrate == BAUD_125K)
	{
		CH_Set_Baud (CH_BAUD_125K);
		CP_Set_BTR0 (BTR0_125KB);
	}
	else if (DeviceNetObjectRAM.baudrate == BAUD_250K)
	{
		CH_Set_Baud (CH_BAUD_250K);
		CP_Set_BTR0 (BTR0_250KB);
	}
#ifdef SUPP_1MB
	else if (DeviceNetObjectRAM.baudrate == BAUD_500K)
	{
		CH_Set_Baud (CH_BAUD_500K);
		CP_Set_BTR0 (BTR0_500KB);
	}
	else
	{
		CH_Set_Baud (CH_BAUD_1M);
		CP_Set_BTR0 (BTR0_1MB);
	}
#else /* oscillator not fast enough */
	else
		CP_Set_BTR0 (BTR0_500KB);
#endif

	CP_Set_BTR1 (BTR1_ALL);

	/*
	// Global Mask Short Register (GMS#)
	// Load 11-bit identifier of 0x80 for group 2 only messages.
	// GMS0 = Identifier bits 11 - 4
	// GMS1 = Identifier bits 3 - 1 (In high 3 bits, other undefined)
	*/
	if (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud)
	{
		// JSW: Theory, ensures onkly RXOK is triggered and messaged is not processed.
		CP_Set_GMS0 (0x00);
		CP_Set_GMS1 (0x00);
	}
	else
	{
		CP_Set_GMS0 (0xff);
		CP_Set_GMS1 (0xff);
	}

	/*
	// Initialize all message values to invalid
	// Then Setup receive and transmit message objects
	*/
	for (MessageObjectRAM.cCurrentMessage = 0; MessageObjectRAM.cCurrentMessage < NUM_MESSAGE_OBJECTS;
		 MessageObjectRAM.cCurrentMessage++)
	{
		CM_MCR0_Set_RXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR0_Set_TXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR0_Clear_INTPND (MessageObjectRAM.cCurrentMessage);
		CM_MCR0_Set_MSGVAL (MessageObjectRAM.cCurrentMessage, CM_INVALID);

		CM_MCR1_Set_NEWDAT (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);
		CM_MCR1_Set_CPUUPD (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR1_Set_TXRQ (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);

		if (MessageObjectRAM.cCurrentMessage >= RCV_COS_ACK_MESSAGE_OBJECT
			&& MessageObjectRAM.cCurrentMessage <= RCV_STROBE_MESSAGE_OBJECT)
			CM_Set_UAR0 (MessageObjectRAM.cCurrentMessage, 0x80);
		else
			CM_Set_UAR0 (MessageObjectRAM.cCurrentMessage, 0);

		CM_Set_UAR1 (MessageObjectRAM.cCurrentMessage, 0);
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB0 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB1 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB2 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB3 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB4 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB5 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB6 = 0x00;
		CANMessageObject[MessageObjectRAM.cCurrentMessage].DB7 = 0x00;
	}

	/*
	// Setup recieve meesages object for each message type we can get
	*/
	for (MessageObjectRAM.cCurrentMessage = RCV_EXPLICIT_MESSAGE_OBJECT;
		 MessageObjectRAM.cCurrentMessage <= RCV_POLL_MESSAGE_OBJECT; MessageObjectRAM.cCurrentMessage++)
	{
		// CRS - should RCV_Enable (RXIE) really be set now - or when
		//       the cnxn set is allocated ????????

		// let's not turn on strobe rcv message until allocated, when we know
		//  for certain what the entire identifier will be.  if 2 masters are
		//  active on same network, we'd get interrupts for strobe messages
		//  that aren't meant for us.
		if (RCV_STROBE_MESSAGE_OBJECT == MessageObjectRAM.cCurrentMessage)
		{
			CM_MCR0_Set_RXIE (MessageObjectRAM.cCurrentMessage, CM_ENABLED);
			CM_MCR0_Set_TXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
			CM_MCR0_Clear_INTPND (MessageObjectRAM.cCurrentMessage);
			CM_MCR0_Set_MSGVAL (MessageObjectRAM.cCurrentMessage, CM_INVALID);
		}
		else
		{
			CM_MCR0_Set_RXIE (MessageObjectRAM.cCurrentMessage, CM_ENABLED);
			CM_MCR0_Set_TXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
			CM_MCR0_Clear_INTPND (MessageObjectRAM.cCurrentMessage);
			CM_MCR0_Set_MSGVAL (MessageObjectRAM.cCurrentMessage, CM_VALID);
		}

		CM_MCR1_Set_NEWDAT (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);
		CM_MCR1_Set_CPUUPD (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR1_Set_TXRQ (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);

		CM_MCFG_Set_Direction (MessageObjectRAM.cCurrentMessage, CM_RECEIVE);
		CM_MCFG_Set_Length (MessageObjectRAM.cCurrentMessage, 0);
	}

	CM_Set_UAR1 (RCV_COS_ACK_MESSAGE_OBJECT, 0x40);
	CM_Set_UAR0 (RCV_COS_ACK_MESSAGE_OBJECT, 0x80 + DeviceNetObjectRAM.macId);

	CM_Set_UAR1 (RCV_EXPLICIT_MESSAGE_OBJECT, 0x80);
	CM_Set_UAR0 (RCV_EXPLICIT_MESSAGE_OBJECT, 0x80 + DeviceNetObjectRAM.macId);

	CM_Set_UAR1 (RCV_POLL_MESSAGE_OBJECT, 0xA0);
	CM_Set_UAR0 (RCV_POLL_MESSAGE_OBJECT, 0x80 + DeviceNetObjectRAM.macId);

	CM_Set_UAR1 (RCV_UNCNNCTD_MESSAGE_OBJECT, 0xc0);
	CM_Set_UAR0 (RCV_UNCNNCTD_MESSAGE_OBJECT, 0x80 + DeviceNetObjectRAM.macId);

	CM_Set_UAR1 (RCV_DUP_MAC_MESSAGE_OBJECT, 0xe0);
	CM_Set_UAR0 (RCV_DUP_MAC_MESSAGE_OBJECT, 0x80 + DeviceNetObjectRAM.macId);

	CM_Set_UAR1 (RCV_STROBE_MESSAGE_OBJECT, 0x00);
	CM_Set_UAR0 (RCV_STROBE_MESSAGE_OBJECT, 0x80);

	/*
	// Setup transmit meesages object for each type( POLL, strobe, COS and explicit)
	*/
	// Setup the cnxn id's for the xmit message objects
	CM_Set_UAR0 (XMIT_EXPLICIT_MESSAGE_OBJECT, (GROUP2 << 6) | DeviceNetObjectRAM.macId);
	// UAR1 can change (explicit/dupmac)

	CM_Set_UAR0 (XMIT_COS_MESSAGE_OBJECT, (COS_REQ << 3) | (DeviceNetObjectRAM.macId >> 3));
	CM_Set_UAR1 (XMIT_COS_MESSAGE_OBJECT, (DeviceNetObjectRAM.macId << 5));

	CM_MCFG_Set_Direction (XMIT_COS_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_COS_MESSAGE_OBJECT,
						IOCnxnSize[CSI_C_PROD] & 0xF); // TODO: value > 8 almost certainly invalid
	// TODO This is getting assert when value beocmes > 8, need to heck this.

	CM_Set_UAR0 (XMIT_STROBE_MESSAGE_OBJECT, (STROBE_RSP << 3) | (DeviceNetObjectRAM.macId >> 3));
	CM_Set_UAR1 (XMIT_STROBE_MESSAGE_OBJECT, (DeviceNetObjectRAM.macId << 5));

	CM_MCFG_Set_Direction (XMIT_STROBE_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_STROBE_MESSAGE_OBJECT,
						IOCnxnSize[CSI_S_PROD] & 0xF); // TODO: value > 8 almost certainly invalid

	CM_Set_UAR0 (XMIT_POLL_MESSAGE_OBJECT, (POLL_RSP << 3) | (DeviceNetObjectRAM.macId >> 3));
	CM_Set_UAR1 (XMIT_POLL_MESSAGE_OBJECT, (DeviceNetObjectRAM.macId << 5));

	CM_MCFG_Set_Direction (XMIT_POLL_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_POLL_MESSAGE_OBJECT,
						IOCnxnSize[CSI_P_PROD] & 0xF); // TODO: value > 8 almost certainly invalid

	for (MessageObjectRAM.cCurrentMessage = XMIT_COS_MESSAGE_OBJECT;
		 MessageObjectRAM.cCurrentMessage <= XMIT_EXPLICIT_MESSAGE_OBJECT; MessageObjectRAM.cCurrentMessage++)
	{
		CM_MCR0_Set_RXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR0_Set_TXIE (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR0_Clear_INTPND (MessageObjectRAM.cCurrentMessage);
		CM_MCR0_Set_MSGVAL (MessageObjectRAM.cCurrentMessage, CM_VALID);

		CM_MCR1_Set_NEWDAT (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);
		CM_MCR1_Set_CPUUPD (MessageObjectRAM.cCurrentMessage, CM_DISABLED);
		CM_MCR1_Set_TXRQ (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);
	}

#ifdef FPL
	// enable transmit interrupt for the COS transmit connection
	CM_MCR0_Set_TXIE (XMIT_COS_MESSAGE_OBJECT, CM_ENABLED);
	CM_MCR0_Set_MSGVAL (XMIT_COS_MESSAGE_OBJECT, CM_VALID);
#endif // FPL

	/*
	// CAN Control Register (CR)
	// CAN register initialization Completed
	*/
	if (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud)
	{
		CP_Set_SIE (1);
		CP_Set_RXOK (0);
	}
	else
	{
		CP_Set_SIE (0);
	}

	CP_Set_MIE (1);
	CP_Set_Init (0);

	CP_Set_ECAN (1);

	/*
	// Set Device OnLine to FALSE
	*/
	DeviceNetObjectRAM.DeviceOnLine = FALSE;

	/*
	 * Call our Can Hardware Init here.
	 * NOTE: only need to set Baud rate before calling...
	 */
	CH_Init ();
}
//***********************************************************************
//
// Function                    InitCANforGrp4()
//
// Author                    Carl Schumaker
// Date                      12/02/98
//
// Functional Description    This function is responsible for initializing the MCAN
//                           control registers.  Gonna use the lower message objects
//                           for group 4 messages.  All other message objects will
//                           be ignored.
//
// Inputs            None
//
// Outputs           None
//
// Returns      None
//
// Edit History     [0]   CRSchumaker   12/02/98   Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

void InitCANforGrp4 ()
{
	unsigned char index;

	INT_DISABLE;

	for (index = 0; index < NUM_MESSAGE_OBJECTS; index++)
	{
		// turn off all the message objects
		CM_MCR0_Set_RXIE (index, CM_DISABLED);
		CM_MCR0_Set_TXIE (index, CM_DISABLED);
		CM_MCR0_Clear_INTPND (index);
		CM_MCR0_Set_MSGVAL (index, CM_INVALID);

		CM_MCR1_Set_NEWDAT (index, CM_NOT_PENDING);
		CM_MCR1_Set_CPUUPD (index, CM_DISABLED);
		CM_MCR1_Set_TXRQ (index, CM_NOT_PENDING);
	}

	// set up the two group 4 message objects
	CM_Set_UAR0 (XMIT_COMM_FLTD_MSG_OBJ, 0xFD);
	CM_Set_UAR0 (RCV_COMM_FLTD_MSG_OBJ, 0xFD);

	CM_Set_UAR1 (XMIT_COMM_FLTD_MSG_OBJ, 0x80);
	CM_Set_UAR1 (RCV_COMM_FLTD_MSG_OBJ, 0xA0);

	// the rcv object
	CM_MCFG_Set_Direction (RCV_COMM_FLTD_MSG_OBJ, CM_RECEIVE);
	CM_MCFG_Set_Length (RCV_COMM_FLTD_MSG_OBJ, 0);

	//   - the xmt comm fault response message varies (2 or 7) - set when sent

	// now turn on the two group 4 message objects
	CM_MCR0_Set_MSGVAL (XMIT_COMM_FLTD_MSG_OBJ, CM_VALID);

	CM_MCR0_Set_RXIE (RCV_COMM_FLTD_MSG_OBJ, CM_ENABLED);
	CM_MCR0_Set_MSGVAL (RCV_COMM_FLTD_MSG_OBJ, CM_VALID);

	INT_ENABLE;
}
//***********************************************************************
//
// Function                 InitDeviceNet()
//
// Author                    Brian J. Taylor
// Date                      3/24/94
//
// Functional Description   This function will setup the MCAN chip, initialize the Objects and send out the DUP
//            MAC ID check.
//
// Inputs          None
//
// Outputs      DNG_cHealthState - Set to Device Offline.
//          DNG_cMasterMACID - Set to Default
//
// Edit History     [0]   3/24/94   BJT   Created
//         [1]   4/11/94   BJT     Call DeviceObjectInit to initialize the device object.
//         [2]   5/4/94   BJT   Call SensorObjectInit to initialize the sensor object.
//         [3]   6/29/94   BJT   Only need to init Sensor, DeviceNet amd Device Object at powerup.
//         [4]   8/3/94   BJT   Call EEPROMInitialized to set global DNG_bEEPROM_Initialized
//         [5]   8/9/94   BJT   Added DeviceLink Support. DevicelInk doesn't use sensor Object.
//         [6]   8/23/94   BJT   Moved around to provided EEPROM time to get up an running. This didn't
//                  fix EEPROM problem but doesn't hurt.
//         [7]   9/22/94   BJT   Call DIPObjectInit to initialize the DIP object.
//         [8]   12/1/94   BJT   Call ParameterObjectInit to initialize the Parameter object for DeviceLink
//         [9]   6/2/95   BJT   Added Support for DeviceNet Configurator Tool.  It doesn't need any
//                  application specific objects.
//         [10]   3/12/97   BJT   Added support for Ack Handler Object.
//         [11]   8/28/97   BJT   Init EEPROM in main.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void InitDeviceNet ()
{
	/*
	// Set DeviceNet Object Master MAC ID to default master's macId
	*/
	DeviceNetObjectRAM.MasterMACID = DEFAULT_MASTER_MACID;

	/*
	// Initialize the connections for a group 2 only slave device
	*/
	ConnectionObjectInit ();

	/*
	// Initialize the message object
	*/
	MessageObjectInit ();

	// should we enable our transmission on the dbus bus
	if (ABAUD_NOTAUTOB == DeviceNetObjectRAM.bAutoBaud)
	{
		IO_SET_CanEnable (1);
	}
	else
	{
		IO_SET_CanEnable (0);
	}

	/*
	// Initialize CAN chip using the DeviceNet mode
	*/
	InitCAN ();
}

//***********************************************************************
//
// Function                 InitApplicationObjects()
//
// Author                    Brian Taylor
//
// Date                      8/29/97
//
// Functional Description    This function calls the intialization functions for all application objects..
//
// Inputs      None
//
// Outputs      None
//
// Edit History     [0]   8/29/97 BJT      Created
//         [1]   4/5/98   BJT      Call Timer and counter object init functions
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/

void InitApplicationObjects ()
{
	/*
	// Initialize the DeviceNet Object with defaults or data from EEPROM.
	*/
	// called directly in main   DeviceNetObjectInit()

	/*
	// Initialize the Identity Object with defaults or data from EEPROM.
	*/
	IdentityObjectInit ();

	/*
	// Initialize the Discrete Input Point Object with defaults or data from EEPROM.
	*/
	AppObjectInit ();

	/*
	// Initialize the Ack Handler Object with defaults or data from EEPROM.
	*/
	AckHandlerObjectInit ();

	// Initalize NVS Object
	NVSObjectInit ();

	// Initialize the Enable Line Object with defaults
	// CRS DEBUG - moving earlier   EnableLineObjectInit()
}
//***********************************************************************
//
// Function                    InitC505()
//
// Author                    Pat Maloney
// Date                      11/15/93
//
// Functional Description    This function is responsible for initializing the MicroProcessor control registers.
//
// Inputs          None
//
// Outputs         None
//
// Returns         None
//
// Edit History        [0]   09/22/97   BJT   Created
//            [1]   10/1/97      AG   Changed for Timer0 Interrupt
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/
void InitC505 ()
{
	IO_SET_CanEnable (0);
	TimerObjectInit (); /* See DN_TMOBJ.C */
}

//***********************************************************************
//
// Function                 InitFactoryDefaults()
//
// Author                    Brian Taylor
//
// Date                      9/9/97
//
// Functional Description    This function calls the factory defaults functions for all application objects.
//
// Inputs      None
//
// Outputs      None
//
// Edit History        [0]   9/9/97 BJT      Created
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/

void InitFactoryDefaults ()
{
	DeviceNetObjectFactoryDefaults ();
	DeviceNetObjectSaveEEPROM ();
	Mb_FactoryDefaults();
	AppObjectFactoryDefaults ();
	AppObjectSaveEEPROM ();
}
