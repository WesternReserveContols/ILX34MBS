#include <app_objs.h>
#include <bld_nmbr.h> //include file for wrc build number
#include <c505c.h>
#include <dn_eeprm.h>
#include <dn_idobj.h>
#include <dn_init.h>
#include <dn_msgob.h>

#define WRC_BUILDNUMBER 101 // attribute 101 is the wrc buildnumber

#define SECRET_SERVICE		   74	// 'hidden' service to setup stuff
#define PASS_KEY			   7	// don't tell
#define SECRET_SET_SN		   0xC7 // set serial number
#define SECRET_SET_PTYPE	   0xC8 // set product type
#define SECRET_PCODE		   0xC9 // set product code
#define SECRET_SET_VENDID	   0xCA // set vendor ID
#define SECRET_SET_REVISION	   0xCC // set revision
#define SECRET_SET_NAME_LENGTH 0xCE // set name string length
#define SECRET_SET_NAME_STRING 0xCF // set the name string
#define SECRET_VIRGINIZE	   0xD3
#define SECRET_SET_EEEND	   0xD4 // end of area CRC will check
#define SECRET_SET_CCV		   0xD5 // set CCV

IdentityObjectStructure IdentityObjectRAM;
char DeviceResetSemaphore =0; // Rick_TEST fix bug45

void IdentityObjectSecretSet ();
void DebugIDObject ();
void DebugIDObjectGetInstanceAttr ();
void DebugIDObjectSetInstanceAttr ();
void NewIDObjBackDoor ();

extern void EEPROMObjectComputeCSFromEE (void);
extern void EEPROMObjectWriteCRC (void);

//***********************************************************************
//
// Function                  IdentityObjectGetInstanceAttr()
//
// Author                    Pat Maloney
// Date                      1/7/1994
//
// Description    Simply copies requested information out of RAM or EEPROM
//                and returns in message buffer
//
// Edit History      [1]   3/28/94   BJT   Added to IDN 9000 code
//         [7]   3/17/98   BJT   Added Configuration Consistency Value Attribute
//         [8]   17Jul99   dsw   Rewritten for Point
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void IdentityObjectGetInstanceAttr ()
{

	// Switch on the requested attribute
	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	case DEVOBJ_VENDOR_ID:
		CurrFragObj.buffer[2] = IdentityObjectRAM.vendor[0];
		CurrFragObj.buffer[3] = IdentityObjectRAM.vendor[1];
		CurrFragObj.numbytes += 2; // default
		break;

	case DEVOBJ_PROD_TYPE:

		CurrFragObj.buffer[2] = IdentityObjectRAM.type[0];
		CurrFragObj.buffer[3] = IdentityObjectRAM.type[1];
		CurrFragObj.numbytes += 2;
		break;

	case DEVOBJ_PROD_CODE:
		CurrFragObj.buffer[2] = IdentityObjectRAM.pc[0];
		CurrFragObj.buffer[3] = IdentityObjectRAM.pc[1];
		CurrFragObj.numbytes += 2;
		break;

	case DEVOBJ_REV:
		CurrFragObj.buffer[2] = IdentityObjectRAM.rev[0];
		CurrFragObj.buffer[3] = IdentityObjectRAM.rev[1];
		CurrFragObj.numbytes += 2;
		break;

	case DEVOBJ_STATUS:
		// If request gets this far we must be configured and owned
		CurrFragObj.buffer[2] = (DEVICE_OWNED_B1 | DEVICE_CONFIGURED_B1);
		CurrFragObj.buffer[3] = 0;
		CurrFragObj.numbytes += 2;
		break;

	case DEVOBJ_SERIAL_NUM:
		CurrFragObj.buffer[2] = IdentityObjectRAM.serial_number[0];
		CurrFragObj.buffer[3] = IdentityObjectRAM.serial_number[1];
		CurrFragObj.buffer[4] = IdentityObjectRAM.serial_number[2];
		CurrFragObj.buffer[5] = IdentityObjectRAM.serial_number[3];
		CurrFragObj.numbytes += 4;
		break;

	case DEVOBJ_PRODUCT_NAME:

	{
		unsigned char *p_name	  = IdentityObjectRAM.name;
		unsigned char *p_response = &CurrFragObj.buffer[2];
		unsigned char  size		  = (*p_name) + 1;
		CurrFragObj.numbytes += size;
		while (size--)
			*(p_response++) = *(p_name++);
		return;
	}
	break;

	case DEVOBJ_CCV:
		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	case WRC_BUILDNUMBER:
		CurrFragObj.buffer[2] = (unsigned char)((unsigned long)__BUILDNUMBER__);
		CurrFragObj.buffer[3] = (unsigned char)(((unsigned long)__BUILDNUMBER__) >> 8);
		CurrFragObj.buffer[4] = (unsigned char)(((unsigned long)__BUILDNUMBER__) >> 16);
		CurrFragObj.buffer[5] = (unsigned char)(((unsigned long)__BUILDNUMBER__) >> 24);
		CurrFragObj.numbytes += 4;
		break;

	default:
		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                  IdentityObjectSetInstanceAttr()
//
// Author                    Brian J. Taylor
// Date                      5/20/94
//
// Description    This function changes identity object values in RAM and EEPROM.
//                It is assumed the caller has permission.
//
// Edit History      [1]   5/20/94   BJT   Creation
//         [5]   11/17/94BJT   Compliancy Test problem. Remove else case to save ROM, Use XID bit instead
//                  of Fragmentation bit to indicate seting highest byte of serial number.
//         [6]   17Jul99  dsw  rewrite for point. Code is commented out until we decide what to do with it.
//         [7]   05Aug99  dsw  I removed all the comments - I kept the case statments.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void IdentityObjectSetInstanceAttr ()
{
	// Switch on attribute to be set
	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	case DEVOBJ_PROD_CODE:
	case DEVOBJ_REV:
	case DEVOBJ_SERIAL_NUM:
	case DEVOBJ_VENDOR_ID:
	case DEVOBJ_PRODUCT_NAME:
	case DEVOBJ_PROD_TYPE:
	default:
		MessageObjectFormatErrorMessage (ATTRIBUTE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                  IdentityObjectInit()
//
// Author                    Brian J. Taylor
// Date                      4/6/1994
//
// Description    This function is used to initialize the ram portion of
//                the Device Object.
//
// Edit History        [0]   4/6/94  BJT      Created
//                     [6]   4/2/98   BJT      Added product and rev defaults
//                        so we don't have to lock sensors during development.
//                     [7]   17Jul99 dsw  rewrite for Point
//
//             Copyright (c) 1994, 2001 Allen-Bradley Co.
//***********************************************************************
#define LOW(X)	((unsigned char)X)
#define HIGH(X) ((unsigned char)(X >> 8))

/* Change in Id_Info.c, or link in object with different info */

extern unsigned char name_mem[];
extern unsigned int	 product_type_mem;
extern unsigned int	 product_code_mem;
extern unsigned char major_revision_mem;
extern unsigned char minor_revision_mem;
extern unsigned int	 vendor_id_mem;

void IdentityObjectInit ()
{
	// Init the Serial Number
	IdentityObjectRAM.serial_number[0] = EEPROMObjectRead (EE_Serial_Num1);
	IdentityObjectRAM.serial_number[1] = EEPROMObjectRead (EE_Serial_Num2);
	IdentityObjectRAM.serial_number[2] = EEPROMObjectRead (EE_Serial_Num3);
	IdentityObjectRAM.serial_number[3] = EEPROMObjectRead (EE_Serial_Num4);

	// Init the Vendor ID
	IdentityObjectRAM.vendor[0] = LOW (vendor_id_mem);
	IdentityObjectRAM.vendor[1] = HIGH (vendor_id_mem);

	IdentityObjectRAM.type[0] = LOW (product_type_mem);
	IdentityObjectRAM.type[1] = HIGH (product_type_mem);
	IdentityObjectRAM.pc[0]	  = LOW (product_code_mem);
	IdentityObjectRAM.pc[1]	  = HIGH (product_code_mem);
	IdentityObjectRAM.rev[0]  = major_revision_mem;
	IdentityObjectRAM.rev[1]  = minor_revision_mem;
	IdentityObjectRAM.name	  = name_mem;
}

//***********************************************************************
//
// Function                  IdentityObject()
//
// Author                    Pat maloney
// Date                      1/7/1994
//
// Functional Description    This function is used to handle Client/Server requests directed toward the Device Object
// and
//            route them to the appropriate Device Object service function.
//
// Edit History      [1]   3/28/94   BJT   Added to IDN 9000 code
//         [13]   6/6/97   BJT   Compliancy TEst error.  Reset Default if restart, not out of box.
//         [14]  17Jul99   dsw   added DebugIDObject call - clean
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void IdentityObject ()
{

	if (0xEE == CurrFragObj.buffer[INDX_INST])
		DebugIDObject ();
	else

		if (0xBD == CurrFragObj.buffer[INDX_INST])
		NewIDObjBackDoor ();
	else

		// Switch on the received Service Code & perform the requested function
		if (0 == CurrFragObj.buffer[INDX_INST])
		MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
	else

		if (1 != CurrFragObj.buffer[INDX_INST])
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
	else
		switch (CurrFragObj.buffer[INDX_SRVC])
		{
		// if request is for get service then check recieve
		// buffer for instance number.  If instance number for class
		// then get class atttribute else if attribute number 1
		// get Instance attribute else return error.
		case GET_REQ:

			IdentityObjectGetInstanceAttr ();
			break;

		// Device Object doesn't support setting of any attributes
		// Unless we are putting in the Factory Defaults during manufacturing
		// The Set Instance attribute 1 will verify EEPROM.
		case SET_REQ:

			if (!EEPROMObjectInitialized ())
				IdentityObjectSetInstanceAttr ();
			else
				MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);

			break;

		// if request is for reset service two type of reset out of box and restart.
		// For the out of box reset the attributes are reset to the factory defaults before restarting.
		// The default case is Restart.
		case RESET_REQ:
#ifdef Rick_TEST
			/*
			// Delay Reset  Rick_TEST fix bug45
			*/

			if ((CurrFragObj.buffer[INDX_ATTR] == RESTART) || ((CurrFragObj.buffindx) == INDX_ATTR))
			{
				DeviceResetSemaphore = COLD_BOOT;
			}

			else if (CurrFragObj.buffer[INDX_ATTR] == OUTOFBOX_RESET)
			{
				DeviceResetSemaphore = OUTOFBOX_RESET;
			}


#else
			if ((CurrFragObj.buffer[INDX_ATTR] == RESTART) || ((CurrFragObj.buffindx) == INDX_ATTR))
			{
				MessageObjectRAM.bCommParamChange = TRUE;
			}

			else if (CurrFragObj.buffer[INDX_ATTR] == OUTOFBOX_RESET)
			{
				InitFactoryDefaults ();
				MessageObjectRAM.bCommParamChange = TRUE;
			}

#endif

			else
				MessageObjectFormatErrorMessage (INVALID_PARAMETER, ADD_CODE_NOT_SPECIFIED);

			break;

		case SECRET_SERVICE:
			if ((0x01 == CurrFragObj.buffer[INDX_INST]) && (PASS_KEY == CurrFragObj.buffer[(CurrFragObj.buffindx) - 1]))
				IdentityObjectSecretSet ();
			else
				MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
			break;

		// service not supported in device object
		default:
			MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
			break;
		}
}

//***********************************************************************
//
// Function         IdentityObjectSecretSet()
//
// Description		This routine is hidden from users.  If you don't get
//                  the message exactly correct, you will get a Service Not
//                  Supported message - like the service/object was never
//				there.  If this message is called, the service, instance,
//				and Pass Key are already correct.
//				The purpose of the routine is a back door to set up the
//				EEPROM.  The module must be on-line to do it.
//
// Edit History
//	[0]	05Aug99	dsw	created
//    [1]   07May01     dsw   Changed errors to Object Doesn't Exist because
//                            DNet Conformance Test is getting too smart
//    [2]   04Oct01     dsw   Fixed error in SECRET_PCODE
//
//             Copyright (c) 2001 Allen-Bradley Co.
//***********************************************************************

void IdentityObjectSecretSet ()
{
	unsigned char LCV;
	unsigned char temp;

	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	case SECRET_SET_SN:
		IdentityObjectRAM.serial_number[0] = CurrFragObj.buffer[INDX_DATA];
		EEPROMObjectWriteAndUpdate (EE_Serial_Num1, CurrFragObj.buffer[INDX_DATA]);
		IdentityObjectRAM.serial_number[1] = CurrFragObj.buffer[INDX_DATA + 1];
		EEPROMObjectWriteAndUpdate (EE_Serial_Num2, CurrFragObj.buffer[INDX_DATA + 1]);
		IdentityObjectRAM.serial_number[2] = CurrFragObj.buffer[INDX_DATA + 2];
		EEPROMObjectWriteAndUpdate (EE_Serial_Num3, CurrFragObj.buffer[INDX_DATA + 2]);
		IdentityObjectRAM.serial_number[3] = CurrFragObj.buffer[INDX_DATA + 3];
		EEPROMObjectWriteAndUpdate (EE_Serial_Num4, CurrFragObj.buffer[INDX_DATA + 3]);
		break;

	case SECRET_SET_PTYPE:
		EEPROMObjectWriteAndUpdate (EE_DEV_TYPE_LO, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectWriteAndUpdate (EE_DEV_TYPE_HI, CurrFragObj.buffer[INDX_DATA + 1]);
		break;

	case SECRET_PCODE:
		EEPROMObjectWriteAndUpdate (EE_Prod_Code_Lo, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectWriteAndUpdate (EE_Prod_Code_Hi, CurrFragObj.buffer[INDX_DATA + 1]);
		break;

	case SECRET_SET_VENDID:
		IdentityObjectRAM.vendor[0] = CurrFragObj.buffer[INDX_DATA];
		EEPROMObjectWriteAndUpdate (EE_Vendor_ID_Lo, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectWriteAndUpdate (EE_Vendor_ID_Hi, CurrFragObj.buffer[INDX_DATA + 1]);
		break;

	case SECRET_SET_REVISION:
		EEPROMObjectWriteAndUpdate (EE_Prod_Rev_Maj, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectWriteAndUpdate (EE_Prod_Rev_Min, CurrFragObj.buffer[INDX_DATA + 1]);
		break;

	case SECRET_SET_NAME_LENGTH:
		EEPROMObjectWriteAndUpdate (EE_Name_Str_Len, CurrFragObj.buffer[INDX_DATA]);
		break;

	case SECRET_SET_NAME_STRING:
		temp = CurrFragObj.buffer[INDX_DATA];
		EEPROMObjectWriteAndUpdate (EE_Name_Str_Len, temp);

		for (LCV = 0; LCV < temp; LCV++)
		{ // loop puts each character
			EEPROMObjectWriteAndUpdate (EE_Name_String + LCV, CurrFragObj.buffer[LCV + (INDX_DATA + 1)]);
		}
		break;

	case SECRET_VIRGINIZE:
		EEPROMObjectWriteAndUpdate (EE_Virgin, 0x69); // to make a virgin
		break;

	case SECRET_SET_EEEND:
		EEPROMObjectWrite (EE_End_Hi, CurrFragObj.buffer[INDX_DATA + 1]);
		EEPROMObjectWrite (EE_End_Lo, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectComputeCSFromEE (); // sets value in structure
		EEPROMObjectWriteCRC ();		// write value to EEPROM memory

		break;

	case SECRET_SET_CCV:
		EEPROMObjectWriteAndUpdate (EE_CCV_Lo, CurrFragObj.buffer[INDX_DATA]);
		EEPROMObjectWriteAndUpdate (EE_CCV_Hi, CurrFragObj.buffer[INDX_DATA + 1]);
		break;

	default:
		// use message like the service never existed
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function         NewIDObjBackDoor()
//
// Description
//
// Edit History
//	[0]	20Jul00	dsw	created
// [1]   07May01  dsw   Changed errors to Object Doesn't Exist because
//                      DNet Conformance Test is getting too smart
//
//             Copyright (c) 2001 Allen-Bradley Co.
//***********************************************************************

void NewIDObjBackDoor ()
{
	switch (CurrFragObj.buffer[INDX_SRVC])
	{
	case SET_REQ:
		switch (CurrFragObj.buffer[INDX_ATTR])
		{
		case SECRET_SET_SN:
			IdentityObjectRAM.serial_number[0] = CurrFragObj.buffer[INDX_DATA];
			EEPROMObjectWriteAndUpdate (EE_Serial_Num1, CurrFragObj.buffer[INDX_DATA]);
			IdentityObjectRAM.serial_number[1] = CurrFragObj.buffer[INDX_DATA + 1];
			EEPROMObjectWriteAndUpdate (EE_Serial_Num2, CurrFragObj.buffer[INDX_DATA + 1]);
			IdentityObjectRAM.serial_number[2] = CurrFragObj.buffer[INDX_DATA + 2];
			EEPROMObjectWriteAndUpdate (EE_Serial_Num3, CurrFragObj.buffer[INDX_DATA + 2]);
			IdentityObjectRAM.serial_number[3] = CurrFragObj.buffer[INDX_DATA + 3];
			EEPROMObjectWriteAndUpdate (EE_Serial_Num4, CurrFragObj.buffer[INDX_DATA + 3]);
			break;

		default:
			MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
			break;
		} // end attribute switch
		break;

	case GET_REQ:
		switch (CurrFragObj.buffer[INDX_ATTR])
		{
		case SECRET_SET_SN:
			CurrFragObj.buffer[2] = IdentityObjectRAM.serial_number[0];
			CurrFragObj.buffer[3] = IdentityObjectRAM.serial_number[1];
			CurrFragObj.buffer[4] = IdentityObjectRAM.serial_number[2];
			CurrFragObj.buffer[5] = IdentityObjectRAM.serial_number[3];
			CurrFragObj.numbytes += 4;
			break;
		default:
			MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
			break;
		} // end get switch
		break;

	default:
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		break;
	} // end service switch
}

//***********************************************************************
//
// Function                  DebugIDObject()
//
// Author                    David Wehrle
// Date                      17 June 1999
//
// Description If I told you, you must be killed.
//
// Inputs      none
//
// Outputs     none
//
// Edit History
// [1]   07May01  dsw   Changed errors to Object Doesn't Exist because
//                      DNet Conformance Test is getting too smart
//
//             Copyright (c) 2001 Allen-Bradley Co.
//***********************************************************************

void DebugIDObject ()

{
	switch (CurrFragObj.buffer[INDX_SRVC]) // switch on service
	{
	case GET_REQ: // Get Request
		DebugIDObjectGetInstanceAttr ();
		break;

	case SET_REQ: // Set Request
		DebugIDObjectSetInstanceAttr ();
		break;

	default: // Unknown Request
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                  DebugIDObjectGetInstanceAttr()
//
// Edit History
// [1]   07May01  dsw   Changed errors to Object Doesn't Exist because
//                      DNet Conformance Test is getting too smart
//
//             Copyright (c) 2001 Allen-Bradley Co.
//***********************************************************************

#define OneByte 1

// One Byte Attribute[4] = 1, Address[5,6]

void DebugIDObjectGetInstanceAttr ()
{
	switch (CurrFragObj.buffer[INDX_ATTR])
	{
	case OneByte:
		if (CurrFragObj.buffindx != 8)
			MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		else
		{
			CurrFragObj.buffer[2] = EEPROMObjectRead (
				(unsigned int)(CurrFragObj.buffer[INDX_DATA + 1] + (CurrFragObj.buffer[INDX_DATA] << 8)));
			CurrFragObj.numbytes += 1;
		}
		break;
	default: // Unknown Request
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		break;
	}
}

//***********************************************************************
//
// Function                  DebugIDObjectSetInstanceAttr()
//
// Edit History
// [1]   07May01  dsw   Changed errors to Object Doesn't Exist because
//                      DNet Conformance Test is getting too smart
//
//             Copyright (c) 2001 Allen-Bradley Co.
//***********************************************************************

#define PassByte 0xDB

void DebugIDObjectSetInstanceAttr ()
{
	// One Byte Attribute[4] = 1, Pass[5], Address[6,7], data[8]
	if ((PassByte == CurrFragObj.buffer[INDX_DATA]) && (OneByte == CurrFragObj.buffer[INDX_ATTR]))
	{
		EEPROMObjectWriteAndUpdate (
			(unsigned int)(CurrFragObj.buffer[INDX_DATA + 2] + (CurrFragObj.buffer[INDX_DATA + 1] << 8)),
			CurrFragObj.buffer[INDX_DATA + 3]);

		// recompute the checksum, we may have changed the length
		EEPROMObjectComputeCSFromEE ();
		// save checksum in EEPROM
		EEPROMObjectWriteCRC ();
	}
	else
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
}
