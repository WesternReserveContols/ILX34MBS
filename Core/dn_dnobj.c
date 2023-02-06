#include <app_objs.h>
#include <c505c.h>
#include <dn_cnobj.h>
#include <dn_dnobj.h>
#include <dn_eeprm.h>
#include <dn_init.h>
#include <dn_msgob.h>
#include <dn_uiobj.h>

DeviceNetObjectStructure DeviceNetObjectRAM;

//***********************************************************************
//
// Function                 DeviceNetObjectDupMACRequestSend()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function is used to kick off the duplicate MAC ID check process.
//            NOTE:  On no occasion should the hardware timer be asked to perform timeout functions for both the
//            duplicate MAC ID check process and an connection inactivity timer.  The duplicate MAC ID check must be
//            completed and the module transitioned to the ON LINE state before any messaging commences. This is
//            convenient because then we can use the timerId in the first cnxn instance as our DUPLICATE MACID CHECK
//            timer. This routine checks to see if the CAN XMIT buffer is busy sending the last message before writing
//            the new message to the buffer.  This is necessary to keep the new message in tact.  A write to a busy XMIT
//            buffer will result in the new bytes being lost and you'd never even know it.
//
// Inputs      CSTAT - MCAN Status
//
// Outputs      XMT_BUF - Transmit Buffer
//           ConnectionObjectRAM - Array of Connection Objects (Only CS connection timerid modified)
//         CTCSR - Core Timer Control and Status
//
// Edit History      [1]   3/24/94   BJT   Add to IDN 9000 code
//         [2]   5/18/94   BJT   Added AutoBAUD!
//         [3]   6/16/94   BJT   Set Health State to DUPMACNOACK to indicate a problem.
//         [4]   7/21/94   BJT   Make sure that LED Flashes Red is Dup MAC isn't being Ack'ed.
//         [5]   6/2/95   BJT   Added Support for DeviceNet Configurator Tool.  Do AutoBaud(without circuitry).
//         [6]   6/22/95   BJT   Added support for Autobaud
//         [7]   3/18/97   BJT   Remove feature [3].  Not compliant with spec!
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectDupMACRequestSend ()
{
	// Transmit the Duplicate MAC ID Check Message
	MessageObjectDupMACMessageSend (REQUEST);

	// Kick off the Duplicate MAC ID timeout timer
	ConnectionObjectRAM[EXPLICIT_CNXN].timerId[1] = DUP_MAC_TIMEOUT_LO;
	ConnectionObjectRAM[EXPLICIT_CNXN].timerId[0] = DUP_MAC_TIMEOUT_HI;
}

//***********************************************************************
//
// Function                 DeviceNetObjectDupMACCheck()
//
// Author                    Brian J. Taylor
// Date                      4/4/94
//
// Functional Description    This function is used check the Duplicate MAC ID algorithm.  If the timer ISR calls this
// function twice
//            then that means that no other device has replied back to tell this device that they are using our MAC ID
//            so we can go On-line.
//
// Inputs            DNG_cDupMACIDCounter - Counter of the number of Dup MAC ID messages sent
//
// Outputs           XMT_BUF - Transmit Buffer
//                CTCSR - Core Timer Control and Status
//                DNG_cDupMACIDCounter - Counter of the number of Dup MAC ID messages sent
//                DNG_bDeviceOnLine - Device Online
//                DNG_cHealthState - Device Health
//
// Edit History      [0]   4/4/94   BJT   Created
//         [1]   7/11/95   BJT   Clear Bus-off counter. If we passed Duplicated MAC everything should be fine.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectDupMACCheck ()
{
	/*
	// See if we need to send second request (The RESET ISR or DeviceNet Object
	// will send out initial DUP MAC message) or if we did send
	// out second request already then go On-line
	*/
	// if (!((CANMessageObject[RCV_DUP_MAC_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) == IS_SET_TXRQ))
	if (CM_MCR1_Get_TXRQ (RCV_DUP_MAC_MESSAGE_OBJECT) == CM_NOT_PENDING)
	{
		if (DeviceNetObjectRAM.DupMACIDCounter == 0)
		{
			/*
			// Increment the DeviceNetObjectRAM.DupMACIDCounter and send out second DUP MAC message
			*/
			DeviceNetObjectRAM.DupMACIDCounter++;
			DeviceNetObjectDupMACRequestSend ();
		}
		else
		{
			/*
			// Clear the DeviceNetObjectRAM.DupMACIDCounter and go On-line
			*/
			DeviceNetObjectRAM.DupMACIDCounter = 0;
			DeviceNetObjectRAM.DeviceOnLine	   = TRUE;
			UIObjectRAM.cHealthState		   = NORMALOPERATION;
			DeviceNetObjectRAM.busOffCntr	   = 0;
		}
	}
}

//***********************************************************************
//
// Function                  DeviceNetObjectDUPMACFail()
//
// Author                    Brian Taylor
// Date                      9/2/1997
//
// Functional Description    This function is called to set a DUP MAC ID failure.
//
// Inputs      None
//
// Ouputs      None
//
// Edit History      [0]   9/2/97    BJT   Created
//         [1]   6/9/98   BJT   Removed putting into autobaud
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectDUPMACFail ()
{
	/*
	// Set Device Offline
	*/
	DeviceNetObjectRAM.DeviceOnLine = FALSE;

	/*
	// Set Health to Duplicate MAC ID failure
	*/
	UIObjectRAM.cHealthState = DUPLICATEMACFAIL;

	// let's get ready for group4 messages (faulted node recovery)
	InitCANforGrp4 ();
}

//***********************************************************************
//
// Function                  DeviceNetObjectSetInstanceAttr()
//
// Author                    Pat Maloney
// Date                      12/27/1993
//
// Functional Description    This function is used to perform the Set_Attribute service for the DeviceNet object
//
// Inputs      RCV_BUF - Receive Buffer
//
// Ouputs      DNG_cStatus - Set if error detected
//          DeviceNetObjectRAM - DeviceNet Object
//
// Edit History      [1]   3/28/94   BJT   Added to IDN 9000 Code
//         [2]   4/14/94   BJT   Added Attibute value checking.
//         [3]   5/2/94   BJT    Set global Health state to Normal Operation after MAC ID changed.
//         [4]   5/10/94   BJT   Added support for BOI attribute.
//         [5]   5/19/94   BJT    Make sure to return success if MAC ID is changed. Added EEPROM support.
//         [6]   5/27/94   BJT    Removed [4].
//         [7]   6/29/94   BJT   Added changes to support real EEPROM driver.
//         [8]   7/15/94   BJT   Any set of Bus-Off counter should clear it according to the spec(Vol I, Rev. 1)
//         [9]   8/3/94   BJT   DN Spec Vol I changes must support BOI attribute.
//         [10]   8/18/94   BJT   Out of ROM.  Set of BOI and Bus-off Counter are back to optional so ifdef out code.
//         [11]   8/19/94   BJT   Added parameter validation for Baud rate and MAC ID.
//         [12]   11/18/94BJT   Parameter validate in 11 specified incorrect error code.  Change to
//         INVALID_ATTRIB_VALUE. [13]   1/11/95   BJT   Add attribuet not settable for allocation info and return
//         attribute not support for
//                  default error.
//         [14]   12/30/97BJT   Added Autobaud attribute.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectSetInstanceAttr ()
{
	/*
	// Switch on attribute to be set
	*/
	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	/*
	// Validate the MAC ID and if valid
	// then set value to received to our new MAC ID and set global to re-init
	// else return error.
		  */
	case DEVICENETOBJ_MACID:

		if (CurrFragObj.buffer[INDX_DATA] <= (unsigned char)MAXIDNMACID)
		{
			MessageObjectRAM.bCommParamChange = TRUE;
			DeviceNetObjectRAM.macId		  = CurrFragObj.buffer[INDX_DATA];

			EEPROMObjectWriteAndUpdate (NVS_MAC_ID, DeviceNetObjectRAM.macId);

			// Return message has no data
		}
		else
			MessageObjectFormatErrorMessage (INVALID_ATTRIB_VALUE, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// Validate the Baud Rate and if valid
	// then set value to received to our new Baud Rate
	// else return error.
	*/
	case DEVICENETOBJ_BAUDRATE:

	//	Rick_TEST bug39 if (EEPROMObjectRead (EE_AutoBaud))
	//	{
			// autobaud is set - don't allow buadrate set
	//		MessageObjectFormatErrorMessage (ATTR_NOT_SETTABLE, ADD_CODE_NOT_SPECIFIED);
	//	}
		if (CurrFragObj.buffer[INDX_DATA] <= (unsigned char)MAXBAUDRATE)
		{
			DeviceNetObjectRAM.baudrate = CurrFragObj.buffer[INDX_DATA];

			EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, DeviceNetObjectRAM.baudrate);

			// Return message has no data
		}
		else
			MessageObjectFormatErrorMessage (INVALID_ATTRIB_VALUE, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// Set device Bus-Off Interrupt Attribute
	// 1 = Reset on Bus off, 0 = Hold on Bus off
	*/
	case DEVICENETOBJ_BOI:

		if (CurrFragObj.buffer[INDX_DATA] <= (unsigned char)RESET_CAN)
		{
			DeviceNetObjectRAM.boi = CurrFragObj.buffer[INDX_DATA];

			EEPROMObjectWriteAndUpdate (EE_BOI, DeviceNetObjectRAM.boi);

			// Return message has no data
		}
		else
			MessageObjectFormatErrorMessage (INVALID_ATTRIB_VALUE, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// Any Set will reset the CAN Bus Off Counter
	*/
	case DEVICENETOBJ_BUS_OFF_CNTR:
		DeviceNetObjectRAM.busOffCntr = 0;
		// Return message has no data
		break;

	/*
	// Allocation Info is unsettable
	*/
	case DEVICENETOBJ_ALLOC_INFO_CNTR:

		MessageObjectFormatErrorMessage (ATTR_NOT_SETTABLE, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// PARAMETER 30 (EDS)
	// Attribute # (40h) - Autobaud
	*/
	case DEVICENETOBJ_AUTOBAUD: // actually autobaud disable

		// although this attribute is being set, it won't affect our behavior until
		//   the module is reset.
		if (CurrFragObj.buffer[INDX_DATA] <= (unsigned char)ENABLED)
		{
			// set cpvaluelow to either 0 or 0xAB, to be written to EEPROM

			//  Save to EEPROM
			EEPROMObjectWriteAndUpdate (EE_AutoBaud, ((CurrFragObj.buffer[INDX_DATA]) ? 0 : AUTO_BAUD_ENABLED));

			// if we're turning on autobaud, let's turn off quick connect
			//  else let's save the current baudrate
			if (!CurrFragObj.buffer[INDX_DATA])
			{
				DeviceNetObjectRAM.QuickConnect = 0;
				EEPROMObjectWriteAndUpdate (EE_Fast_Start, FALSE);
			}
			else
				EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, DeviceNetObjectRAM.baudrate);

			// Return message has no data
		}
		else
			MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// PARAMETER ?? (EDS)
	// Attribute # (66h) - Quick Connect
	*/
	case DEVICENETOBJ_QUICK_CONNECT:

		if (CurrFragObj.buffer[INDX_DATA] <= (unsigned char)ENABLED)
		{
			DeviceNetObjectRAM.QuickConnect = (CurrFragObj.buffer[INDX_DATA]) ? QUICK_CONN_ENABLED : 0;

			/*
			//  Save to EEPROM
			*/
			EEPROMObjectWriteAndUpdate (EE_Fast_Start, DeviceNetObjectRAM.QuickConnect);

			// make sure we save baud rate
			if (QUICK_CONN_ENABLED == DeviceNetObjectRAM.QuickConnect)
				EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, DeviceNetObjectRAM.baudrate);

			// Return message has no data
		}
		else
			MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// Unsupported Device Net attribute
	*/
	default:

		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                  DeviceNetObjectGetClassAttr()
//
// Author                    Pat Maloney
// Date                      12/27/1993
//
// Functional Description    This function is used to perform the Get_Attribute (class) service for the DeviceNet
// Object.
//            The response data length portion of the MCAN register TRTDL is set here.  Also, the data portion of
//            the MCAN XMT_BUF[] is set here in preparation for a connection based response.
//
// Inputs      RCV_BUF - Receive Buffer
//          TRTDL - Data Length set to GEN_RSP_MIN_LEN
//
// Ouputs      DNG_cStatus - Set if error detected
//          TRTDL - RTR and Data Length
//          XMIT_BUF - Transmit buffer
//
// Edit History        [1]   3/28/94   BJT   Added to IDN 9000 Code
//         [2]   4/14/94   BJT   Updated to Rev 1 of IDN Object.
//         [3]   5/2/94   BJT    Revision is only 1 byte now.
//         [4]   8/3/94   BJT   Dn Spec Vol I changes. Removed [3]  Revision back to 2 bytes.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectGetClassAttr ()
{
	/*
	// if get class attribute revision
	// then put revision in transmit data buffer and set length
	*/
	if (CurrFragObj.buffer[INDX_ATTR] == DEVICENETOBJ_REVISION)
	{
		CurrFragObj.buffer[2] = DEVICENET_OBJECT_REV_LO;
		CurrFragObj.buffer[3] = DEVICENET_OBJECT_REV_HI;
		CurrFragObj.numbytes += 2;
	}

	/*
	// can't get that attribute so return error
	*/
	else
		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
}

//***********************************************************************
//
// Function                  DeviceNetObjectGetInstanceAttr()
//
// Author                    Pat Maloney
// Date                      12/27/1993
//
// Functional Description    This function is used to perform the Get_Attribute (Instance) service for the IDN Object.
//            The response data length portion of the MCAN register TRTDL is set here.  Also, the data
//            portion of the MCAN XMT_BUF[] is set here in preparation for a connection based response.
//
// Inputs      DeviceNetObjectRAM - IDN Object
//         RCV_BUF - Receive Buffer
//         TRTDL - Data Length set to GEN_RSP_MIN_LEN
//
// Ouputs      DNG_cStatus - Set if error detected
//         TRTDL - RTR and Data Length
//         XMIT_BUF - Transmit buffer
//
// Edit History     [1]   3/28/94   BJT   Added to IDN 9000 Code
//         [2]   8/3/94   BJT   Dn Spec Vol I changes must support BOI attribute.
//         [3]   11/17/94BJT   Out of ROM.  Set of BOI and Bus-off Counter are back to optional so ifdef out code.
//         [4]   12/30/97BJT   Added Autobaud attribute.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectGetInstanceAttr ()
{
	unsigned char lTemp;

	CurrFragObj.numbytes++; // default - one more data byte

	/*
	// Select attribute
	*/
	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	/*
	// Get device MAC ID an put it into transmit buffer byte 2
	*/
	case DEVICENETOBJ_MACID:
		CurrFragObj.buffer[2] = DeviceNetObjectRAM.macId;
		break;

	/*
	// Get device Mode an put it into transmit buffer byte 2
	*/
	case DEVICENETOBJ_BAUDRATE:
		// return 0xAB is running at 1MegaBaud
		CurrFragObj.buffer[2] = (BAUD_1M == DeviceNetObjectRAM.baudrate) ? 0xAB : DeviceNetObjectRAM.baudrate;
		break;

	/*
	// Get device Bus-Off Interrupt Attribute an put it into transmit buffer byte 2
	*/
	case DEVICENETOBJ_BOI:
		CurrFragObj.buffer[2] = DeviceNetObjectRAM.boi;
		break;

	/*
	// Get device CAN Bus Off Counter an put it into transmit buffer byte 2
	*/
	case DEVICENETOBJ_BUS_OFF_CNTR:
		CurrFragObj.buffer[2] = DeviceNetObjectRAM.busOffCntr;
		break;

	/*
	// Get Device Allocation Information
	// Master MAC ID and allocation connections
	*/
	case DEVICENETOBJ_ALLOC_INFO_CNTR:

		// first determine what the active cnxns are
		lTemp = 0;

		if (ConnectionObjectRAM[EXPLICIT_CNXN].state & CNXN_ACTIVITY_MASK)
			lTemp += EXPLICIT_MSG;

		if (ConnectionObjectRAM[STROBE_CNXN].state & CNXN_ACTIVITY_MASK)
			lTemp += STROBED;

		if (ConnectionObjectRAM[COS_CNXN].state & CNXN_ACTIVITY_MASK)
			lTemp += (DeviceNetObjectRAM.AllocedCnxns & (COS_MSG | CYCLIC | ACK_SUPPRESSION));

		// make sure poll was allocated and not just part of COS cnxn
		if ((ConnectionObjectRAM[POLL_CNXN].state & CNXN_ACTIVITY_MASK) && (DeviceNetObjectRAM.AllocedCnxns & POLLED))
			lTemp += POLLED;

		CurrFragObj.buffer[2] = lTemp;
		CurrFragObj.buffer[3] = DeviceNetObjectRAM.MasterMACID;
		CurrFragObj.numbytes++;
		break;

	/*
	// PARAMETER 30 (EDS)
	// Attribute # (40h) - Autobaud
	*/
	case DEVICENETOBJ_AUTOBAUD: // should be autobaud disable
		// internally we use 0 or 0xAB (not 1)
		//  Read from EEPROM
		CurrFragObj.buffer[2] = (EEPROMObjectRead (EE_AutoBaud)) ? 0 : 1;

		break;

	/*
	// PARAMETER 30 (EDS)
	// Attribute # (40h) - Autobaud
	*/
	case DEVICENETOBJ_QUICK_CONNECT:
		// internally we use 0 or 0xFA (not 1)
		CurrFragObj.buffer[2] = (DeviceNetObjectRAM.QuickConnect) ? 1 : 0;
		break;

	/*
	// Can't get the request attribute
	*/
	default:

		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                 DeviceNetObjectInit()
//
// Author                    Pat Maloney
// Date                      11/15/93
//
// Functional Description    This function initializes the RAM portion of the DeviceNet object.
//
// Inputs      None
//
// Ouputs       DeviceNetObjectRAM - Set Mode and MAC ID counter to Defaults or EEPROM values
//
// Edit History      [1]   3/24/94   BJT   Added to IDN 9000 code.
//         [2]   5/10/94   BJT   Removed MSB/LSB Address vars.
//         [3]   6/29/94   BJT   Added changes to support real EEPROM driver.
//         [4]   8/3/94   BJT   Just check global DNG_bEEPROM_Initialized to see if EEPROM initialized.
//         [5]   8/3/94   BJT   Init BIO from EEPROM.
//         [6]   8/19/94   BJT   Remove option of setting BOI. This is optional and we are out of ROM.
//         [7]   9/8/94   BJT   EEPROMRead & EEPROMWrite take a word(not byte) offset as parameter.
//         [8]   9/9/94   BJT   To save ROM initially set value to factory defualts.
//         [9]   8/28/97   BJT   Updated to support EEPROMObject function call changes.
//		 [10]  14May99	dsw	added checks for invalid MacId and BaudRate
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectInit ()
{

	/*
	// Default macId and Baud Rate to Defaults
	*/
	DeviceNetObjectFactoryDefaults ();

	/*
	// Is the EEPROM already initialized
	// If so then read MAC ID and baud rate from EEPROM
	// else need to set MAC ID so that we can communicate
	// with test device to initialize the EEPROM.
	*/
	if (EEPROMObjectInitialized ())
	{
		DeviceNetObjectRAM.macId		= EEPROMObjectRead (NVS_MAC_ID);
		DeviceNetObjectRAM.baudrate		= EEPROMObjectRead (NVS_BAUD_RATE);
		DeviceNetObjectRAM.bAutoBaud	= EEPROMObjectRead (EE_AutoBaud);
		DeviceNetObjectRAM.QuickConnect = EEPROMObjectRead (EE_Fast_Start);
		DeviceNetObjectRAM.boi			= EEPROMObjectRead (EE_BOI);

		if ((DeviceNetObjectRAM.macId > 63) || (DeviceNetObjectRAM.boi > 1) || (DeviceNetObjectRAM.baudrate > BAUD_1M)
			|| (DeviceNetObjectRAM.bAutoBaud && (AUTO_BAUD_ENABLED != DeviceNetObjectRAM.bAutoBaud))
			|| (DeviceNetObjectRAM.QuickConnect && (QUICK_CONN_ENABLED != DeviceNetObjectRAM.QuickConnect)))
		{
			// at least one of the values wasn't right
			//   let's initialize all of them to defaults
			DeviceNetObjectFactoryDefaults ();

			DeviceNetObjectSaveEEPROM ();
		}
	}

	if (DeviceNetObjectRAM.bAutoBaud)
	{
		// if autobaud turned on, set state to enabled state
		DeviceNetObjectRAM.bAutoBaud = ABAUD_ENABLED;
	}

	if (DeviceNetObjectRAM.QuickConnect)
	{
		// if quickconnect active, turn off autobaud
		DeviceNetObjectRAM.bAutoBaud = ABAUD_NOTAUTOB; // don't touch EEPROM value
	}

	/*
	// Clear the BUS OFF counter
	*/
	DeviceNetObjectRAM.busOffCntr = 0;

	// no cnxns allocated
	DeviceNetObjectRAM.AllocedCnxns = 0;

	DeviceNetObjectRAM.DupMACIDCounter = 0;
	DeviceNetObjectRAM.FirstDupMACSent = 0;

	TURN_BOTH_RUN_BITS_OFF; // both IO cnxns in idle mode

	DeviceNetObjectRAM.QCTimer = 0;

	DeviceNetObjectRAM.DeviceOnLine = FALSE;
}

//***********************************************************************
//
// Function                 DeviceNetObjectFactoryDefaults()
//
// Author                    Brian J. Taylor
// Date                      5/27/94
//
// Functional Description    This function reads the factory default values from EEPROm for the DeviceNet Object
// attributes.
//
// Inputs      None
//
// Ouputs      DeviceNetObjectRAM - Set Mode, MAC ID and Baud Rate counter to EEPROM values
//
// Edit History      [0]   5/27/94   BJT   Created.
//         [1]   6/22/94   BJT   Factory Defaults for MAC ID and Baud rate will be hard coded.
//         [2]   8/3/94   BJT   Default BIO to reset.
//         [3]   8/5/94   BJT   BIO default is now HOLD.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************

void DeviceNetObjectFactoryDefaults ()
{
	// Set the Factory Default MAC ID
	DeviceNetObjectRAM.macId = 63;

	// Set the Factory Default Baud Rate
	DeviceNetObjectRAM.baudrate = BAUD_1M;

	// Set the Factory Default Bus-off Interrupt
	DeviceNetObjectRAM.boi = HOLD_CAN_RESET;

	// Set the Factory Default AutoBaud
	DeviceNetObjectRAM.bAutoBaud = AUTO_BAUD_ENABLED;

	// Set the Factory Quick Connect Mode
	DeviceNetObjectRAM.QuickConnect = DISABLED;
}

//***********************************************************************
//
// Function                  DeviceNetObjectSaveEEPROM()
//
// Author                    Brian J. Taylor
// Date                      5/20/1994
//
// Functional Description    This function stores an DeviceNet Object Attributes to EEPROM.
//
// Inputs      DeviceNetObjectRAM - Device Object
//
// Outputs      None
//
// Edit History      [0]   5/24/94   BJT   Created
//         [1]   6/1/94   BJT   Always return Success status.
//         [2]   6/29/94   BJT   Updates to support real EEPROM Driver.
//              [3]   8/3/94   BJT   Save BIO in EEPROM
//         [4]   8/19/94   BJT   Remove option of setting BOI. This is optional and we are out of ROM.
//         [5]   9/8/94   BJT   EEPROMRead & EEPROMWrite take word(not byte) offset as parameter.
//         [6]   8/28/97   BJT   Updated to support EEPROMObject function call changes.
//         [7]   5/11/98   BJT   Fix bug that saved autobaud in COMPLETED state.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void DeviceNetObjectSaveEEPROM ()
{
	EEPROMObjectWriteAndUpdate (NVS_MAC_ID, DeviceNetObjectRAM.macId);

	EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, DeviceNetObjectRAM.baudrate);

	EEPROMObjectWriteAndUpdate (EE_BOI, DeviceNetObjectRAM.boi);

	EEPROMObjectWriteAndUpdate (EE_AutoBaud, DeviceNetObjectRAM.bAutoBaud);

	EEPROMObjectWriteAndUpdate (EE_Fast_Start, DeviceNetObjectRAM.QuickConnect);
}

//***********************************************************************
//
// Function                  DeviceNetObject()
//
// Author                    Pat Maloney
// Date                      12/27/1993
//
// Functional Description    This function is used to handle all Client/Server requests directed towards the IDN OBJECT
// and
//            routes them to the appropriate IDN Object service function.
//
// Inputs      RCV_BUF - Receive Buffer
//          DNG_pstTargetCnxn - Pointer to Connection Object that received the message.
//
// Outputs      DNG_cStatus - Set if error detected
//           DNG_cPARAM2 - release chioce of Deallocate Group 2 Identifier Set.
//         TRTDL - RTR and Data Length
//
// Edit History      [1]   3/28/94   BJT   Added to IDN 9000 Code Modified to handle all C/S requests in this module.
//         [2]   5/5/94   BJT   Removed checking for connection established since this is already done in
//         message_router. [3]   5/24/94   BJT   Added support to setting and saving DeviceNet Object attribute Factory
//         Defaults in EEPROM.
//          [4]   6/1/94    BJT   Set response buffer length to min and then have each function increment if they
//          respond
//                  with more data.
//         [5]   6/22/94   BJT   Removed save service to save DN attributes to EEPROM.  This is now initated by setting
//                  an attribute in the sensor object.
//         [6]   7/1/94   BJT   Verify instance number for services.  Only support 1 instance of DeviceNet Object.
//         [7]   7/21/94   BJT   Just return SERVICE_NOT_SUPP for set of class attributes.
//         [8]   8/18/94   BJT   Need to return Object doesn't exists for error instances we don't support.
//         [9]   11/9/94   BJT   Save memory.  Check for instance number in one location.
//         [10]   2/09/98   AG   Removed IF statement, [6] and [7] since we support more than 1 instance.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

unsigned int COUNT_G2_IDSET = 0;
void		 DeviceNetObject ()
{
	/*
	// Switch on the received Service Code & perform the requested function
	*/
	switch (CurrFragObj.buffer[INDX_SRVC])
	{

	/*
	// If Get request then see if it is for a class attribute
	// or a Instance attribute by looking in the recieve buffer
	// for a instance number.
	*/
	case GET_REQ:

		if (CurrFragObj.buffer[INDX_INST] == 1)
			DeviceNetObjectGetInstanceAttr ();
		else if (CurrFragObj.buffer[INDX_INST] == CLASS_DESIGNATOR)
			DeviceNetObjectGetClassAttr ();
		else
			MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// If Set request then see if it is for class attribute or Instance attribute 1
	// by looking in the recieve buffer for a instance number.
	// (Current IDN Object doesn't support the setting of any class attributes).
	*/
	case SET_REQ:

		if (CurrFragObj.buffer[INDX_INST] == 1)
			DeviceNetObjectSetInstanceAttr ();
		else
			MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// If deallocate group 2 Id Set and correct
	// class and instance specified then deallocate the
	// release choice from the receive buffer.
	*/
	case DEALLOC_GRP2_IDSET:

		if (CurrFragObj.buffer[INDX_INST] == 1)
			ConnectionObjectDeallocGrp2IdSet ();
		else
			MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	/*
	// If the request to allocate the group 2 Id Set
	//   then check to see if allocation class is the
	//   IDN object and instance 1 since this is the
	//   only thing that can be allocate for group 2 Id Set.
	*/
	case ALLOC_GRP2_IDSET:
		COUNT_G2_IDSET++;
		if (CurrFragObj.buffer[INDX_INST] == 1)
			ConnectionObjectAllocGrp2IdSet ();
		else
			MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;

	default:
		MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}
