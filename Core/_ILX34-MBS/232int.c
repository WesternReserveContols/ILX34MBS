/**
 * @file MBPort.c
 * @author Allen-Bradley Co.
 * @date 25 Jul 2022
 * @brief 	This module will 'interface' with the application objects.
 * 			The idea here is that the DeviceNet portion of the does not
 * 			have to change when additional objects are added.  This module will
 * 			act as a multiplexer to that supports multiple application objects
 * 			where the module type is discovered at powerup.
 *
 */

//***********************************************************************
// Project Name              Slice
// Module                    DIG_OBJS.C
//
// Module Description	This module will 'interface' with the application
//			objects.  The idea here is that the DeviceNet
//			portion of the does not have to change when
//			additional objects are added.  This module will
//			act as a multiplexer to that supports
//			multiple application objects where the module
//			type is discovered at powerup.
//
//             Copyright (c) 1999-2001 Allen-Bradley Co.
//
//  Revision History:
//	[0]	dsw   15Jul99   created - digital only
//    [1]   dsw   14Sept01  Added OX2
//    [2]   dsw   24Sept01  Added OB2EP
//
//***********************************************************************
#include <string.h>
#include "c505c.h" // uP register declarations
#include <string.h>
#include "mbport.h"

#ifndef FCL
#define FCL 140
#endif
// INCLUDES

#include "app_objs.h" // for all modules
#include "msg.h"
#include "memopt.h"

#include "dn_eeprm.h" // to get ProductCode
#include "dn_def.h"	  // DNet definitions
#include "dn_dnobj.h" // declarations for the DN object
#include "dn_msgob.h" // declarations for the Message Object
#include "dn_uiobj.h" // declarations for the Message Object
#include "dn_cnobj.h" // declarations for the connection object
#include "dn_init.h"
#include "xdatacpy.h"
#include "dn_tmobj.h"

#include "gpio.h"
#include "serial_config.h"

// uncomment to allow the connection sizes to be changed.
//#define NewConnectionAllocationMethod

// DEFINES

// LOCAL FUNCTIONS
extern void AssyPFunc (MSG *msg);
#ifdef GMM

#define TX_FIFO_OVERFLOW 0x01
#define RX_FIFO_OVERFLOW 0x02
#define PARITY_ERR_BIT	 0x04

static BOOL GMM_config_data_valid (void);

static unsigned char LocalFraming;
static unsigned char LocalBaudRate;

// EXTERNAL DEFINITIONS
extern unsigned char TxStrLen;
extern unsigned char type; // xmt data type
extern unsigned char SRecSwap;
extern unsigned char TxStrDelimMode;
extern unsigned char RRecPadMode;
extern unsigned char TxStrDelim;
extern unsigned char RRecRxStrMaxLen;
extern unsigned char RRecRxStrType;
extern unsigned char RRecSwap;
extern unsigned char StartMode;
extern unsigned char EndMode;
extern unsigned char RRecStart;
extern unsigned char RRecEnd;
extern unsigned char RRecPadChar;
extern unsigned char RRecRcvMode;
extern bool			 AssyCRxRecNum;
extern unsigned char SRecMode;
extern bool			 SwapHdrBytes;

// EXTERNAL FUNCTIONS
extern void			 ascii_strc_f_b_set (unsigned char framing, unsigned char baud_rate);
extern void			 InitSerialIO (void);
extern void			 GMM_ser_data_rcvd (void); // if in idle mode, COS msg triggered
extern unsigned char current_status_byte (void);
extern void			 GMM_pad_buffers (void);
extern void			 GMM_set_msg_pointers (void);
#endif // GMM
extern void MBM_QueMbTxMsg(unsigned char  *P_InBuf);

// VARIABLES
unsigned int  AppProductCode;
unsigned char g_status;
unsigned char g_addCode = 0xff;
// Ed Riede, Western Reserve Controls 2001-11-08
// (c) Western Reserve Controls
// added to allow variable poll response sizes
extern unsigned char P_OutMsgBufferSize;
extern unsigned char C_OutMsgBufferSize;
// End Wrc

//**********************************************************************
// Function    AppObjectInitAppLEDs()
//
// Edit History
// [0]   12Aug99  crs   Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
extern unsigned char CompAssyCSize (void);
extern unsigned char CompAssyPSize (void);
void		  AppObjectInitAppLEDs (void)
{
	AppProductCode			 = 5000; //?
	UIObjectRAM.num_app_LEDs = 4;	 //?
}

//**********************************************************************
// Function		AppObjectInit()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
void SHWInit (void);

extern void InitRxTxAssy (void);
extern void CustParamInit (void);

void AppObjectInit ()
{
	CustParamInit ();
	InitMbParam();
	InitRxTxAssy ();
#ifndef NewConnectionAllocationMethod
	IOCnxnSize[CSI_P_CONS] = CompAssyCSize (); // poll consume size
	IOCnxnSize[CSI_P_PROD] = CompAssyPSize (); // poll produce size
	IOCnxnSize[CSI_S_PROD] = 0;				   // strobe produce size
	IOCnxnSize[CSI_C_PROD] = CompAssyPSize (); // COS produce size
#endif
}

//**********************************************************************
// Function		AppObjectFactoryDefaults
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
extern void ParamClassReset (void);
void AppObjectFactoryDefaults ()
{
	ParamClassReset ();
	CustParamInit ();
}

//**********************************************************************
// Function		AppObjectSaveEEPROM
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
void AppObjectSaveEEPROM ()
{
	// AppObjFactoryDefaults Is always caled right before this
	// to simplify, AppObjectSaveEEPROM is included with the above
	// this may cause problems in future revisions.
}

//**********************************************************************
// Function		AppObjectMonitorIO
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
bool		  polldatachanged	  = 0;
bool		  cosdatachanged	  = 0;
bool		  assymainprocnotbusy = 1;
unsigned char mainloopassydata[BYTES_OF_SER_DATA + 6];
unsigned char size_of_mainloopassydata = 0;
bool		  new_produce_data_avail   = 0;
void		  TriggerCOS (void)
{
	new_produce_data_avail = 1;
}

void AppObjectMonitorIO (void) //?
{

	TimerObjectSvcTimer ();

//	SHWMain ();

//	TimerObjectSvcTimer ();

//	RRecMain ();

//	TimerObjectSvcTimer ();

	// assy main proc
	if (new_produce_data_avail)
	{ // scope
		MSG msg;
		new_produce_data_avail = 0;
		assymainprocnotbusy	   = 0;
		msg.buf				   = mainloopassydata;
		AssyPFunc (&msg);
		size_of_mainloopassydata = msg.buflen;
		polldatachanged			 = 1;
		cosdatachanged			 = 1;
		// Trigger the COS
#ifdef GMM
		if (GMMRAM.active)
			GMM_ser_data_rcvd (); // if in idle mode, COS msg triggered
		else					  // set the appobjectcosbit to 1
#endif							  // GMM
			AppObjectCOS_Bit = 1;
		assymainprocnotbusy = 1;
	} // end scope

	TimerObjectSvcTimer (); // allow led's to flash
}

//**********************************************************************
// Function		AppObjectServiceTimer
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectServiceTimer (void)
{
#ifndef NewConnectionAllocationMethod

#ifdef GMM
	// prevent this from happening when in GMMode
	if (!GMMRAM.active
		&& (IOCnxnSize[CSI_P_CONS] != CompAssyCSize ()		// poll consume size
			|| IOCnxnSize[CSI_P_PROD] != CompAssyPSize ()	// poll produce size
			|| IOCnxnSize[CSI_C_PROD] != CompAssyPSize ())) // COS produce size
#else														// GMM
	if (IOCnxnSize[CSI_P_CONS] != CompAssyCSize () || // poll consume size
		IOCnxnSize[CSI_P_PROD] != CompAssyPSize () || // poll produce size
		IOCnxnSize[CSI_C_PROD] != CompAssyPSize ())	  // COS produce size
#endif														// GMM
	{
		new_produce_data_avail = 1;
		IOCnxnSize[CSI_P_CONS] = CompAssyCSize (); // poll consume size
		IOCnxnSize[CSI_P_PROD] = CompAssyPSize (); // poll produce size
		IOCnxnSize[CSI_S_PROD] = 0;				   // strobe produce size
		IOCnxnSize[CSI_C_PROD] = CompAssyPSize (); // COS produce size
	}
#endif
}

//**********************************************************************
// Function		AppObjectFillCOSData
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
extern unsigned char P_OutMsgBuffer[];
extern unsigned char C_OutMsgBuffer[];

void AppObjectFillCOSData (void)
{
// just copy the data from the assembly object buffer
// to the explicit message buffer????
//---->goooD---> called from the main loop!!!!
// we can just call the assembly object from a msg structure
// assymainprocnotbusy is always true
#ifdef GMM
	if (!GMMRAM.active) // don't do when in GMMode
#endif					// GMM
		if (cosdatachanged /* && assymainprocnotbusy*/)
		{
			/*
			unsigned char * src=mainloopassydata;
			unsigned char * dest=C_OutMsgBuffer;
			unsigned char len=C_OutMsgBufferSize=size_of_mainloopassydata;
			while(len--) *(dest++)=*(src++);
			*/

			// copy it up
			xdata_memcpy (C_OutMsgBuffer, mainloopassydata, C_OutMsgBufferSize = size_of_mainloopassydata);
			cosdatachanged = 0;
		}
}

//**********************************************************************
// Function		AppObjectFillPollData()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
void AppObjectFillPollData (void)
{
	// just copy the data from the assembly object buffer
	// to the explicit message buffer????
	// called from ISR---->>>> this  is not good.
	//
	if (polldatachanged && assymainprocnotbusy)
	{
		/*
		unsigned char * src=mainloopassydata;
		unsigned char * dest=P_OutMsgBuffer;
		unsigned char len=P_OutMsgBufferSize=size_of_mainloopassydata;
		while(len--) *(dest++)=*(src++);
		*/
		// copy it
		xdata_memcpy (P_OutMsgBuffer, mainloopassydata, P_OutMsgBufferSize = size_of_mainloopassydata);
		polldatachanged = 0;
	}
	// stuff the buffer if anything needs to be done here
}

//**********************************************************************
// Function		AppObjectFillStrobeData()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectFillStrobeData (void)
{
	// no strobe data
}

//**********************************************************************
// Function		AppObject()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

/****
 *
 *Function Definitions for WRC device net stack emulation
 *
 *
 ****/

extern void *ParamFuncRom (MSG *);
extern void *AsciiFunc (MSG *);
extern void *SRecProtFunc (MSG *);
extern void *RRecProtFunc (MSG *);

void AppObject (void)
{
	// transform AB no object oriented format to WRC object oriented format.
	MSG msg;
	void (*f) (MSG * msg);
	msg.service	 = CurrFragObj.buffer[INDX_SRVC];
	msg.class	 = (CurrFragObj.buffer[INDX_CL_HI] << 8) + CurrFragObj.buffer[INDX_CL_LO];
	msg.instance = CurrFragObj.buffer[INDX_INST];
	msg.buflen	 = CurrFragObj.buffindx - 5;
	g_status	 = 0;
	if (msg.service == GET_REQ || msg.service == SET_REQ)
	{
		msg.attribute = CurrFragObj.buffer[INDX_ATTR];
		msg.buflen--;
	}
	msg.buf = &CurrFragObj.buffer[INDX_DATA];
	// Ok, process the message!
	switch (msg.class)
	{
	case 0x0f:
		f = ParamFuncRom (&msg);
		break;

	case 0x70:
		f = AsciiFunc (&msg);
		g_status = SERVICE_NOT_SUPPORTED;
		g_addCode = 0xFF;
		break;
	case 0x71:
		f = SRecProtFunc (&msg);
		break;

	case 0x72:
		f = RRecProtFunc (&msg);
		break;

	default:
		MessageObjectFormatErrorMessage (OBJECT_DOES_NOT_EXIST, ADD_CODE_NOT_SPECIFIED);
		return;
	}
	if (!g_status)
		f (&msg);
	if (g_status)
	{
		MessageObjectFormatErrorMessage (g_status, g_addCode);
		return;
	}
	// transform WRC object oriented format back to AB non object oriented format.
	CurrFragObj.numbytes = msg.buflen + 2;

	xdata_memcpy (&CurrFragObj.buffer[2], msg.buf, msg.buflen);
}

//**********************************************************************
// Function		AppObjectPollConsume()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

unsigned char consume_data_size;
BOOL		  AppObjectPollConsume (void)
{
	MSG msg;
	memset (&msg, 0, sizeof (msg));

	msg.buflen = consume_data_size;
	msg.buf	   = &P_InMsgBuffer[0];

	MBM_QueMbTxMsg(&P_InMsgBuffer[0]);
	return (TRUE); // no data - simply send response
}

//**********************************************************************
// Function		AppObjectStrobeConsume()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

BOOL AppObjectStrobeConsume (void)
{
	return (TRUE); // no data - simply send response
}

//**********************************************************************
// Function		AppAssemblyObject()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
/*****
 *
 * This function is used for explicit messages to the assembly object
 * why?   I don't know.
 *
 *
 *
 ******/
extern void  AssemblyFill (MSG *msg);
extern void *AssemblyFunc (MSG *msg);

void AppAssemblyObject (void)
{
	MSG msg;
	void (*f) (MSG * msg);
	msg.service	 = CurrFragObj.buffer[INDX_SRVC];
	msg.class	 = (CurrFragObj.buffer[INDX_CL_HI] << 8) + CurrFragObj.buffer[INDX_CL_LO];
	msg.instance = CurrFragObj.buffer[INDX_INST];
	AssemblyFill (&msg);
	if (msg.attribute)
	{
		msg.attribute = CurrFragObj.buffer[INDX_ATTR];
		msg.buf		  = &CurrFragObj.buffer[INDX_DATA];
		msg.buflen	  = CurrFragObj.buffindx - INDX_DATA; //?
	}
	else
	{
		msg.buf	   = &CurrFragObj.buffer[INDX_ATTR];
		msg.buflen = CurrFragObj.buffindx - INDX_ATTR; //?
	}
	if (!g_status)
		f = AssemblyFunc (&msg);
	if (!g_status)
		f (&msg);
	if (g_status)
	{
		MessageObjectFormatErrorMessage (g_status, g_addCode);
		return;
	}
	CurrFragObj.numbytes = msg.buflen + 2;

	// To suppress the overlapping memory region warning we will use a loop
	for (int i = 0; i < msg.buflen; i++)
		CurrFragObj.buffer[2 + i] = msg.buf[i];
	/*
	{//scope
	unsigned char * dest=&CurrFragObj.buffer[2];
	while(msg.buflen--)
		*(dest++)=*(msg.buf++);
	}
	*/
}

//**********************************************************************
// Function		AppObjectCnxnNotRunning()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

// take action whe cnxns deleted
void AppObjectCnxnNotRunning (uchar cnxnindex)
{
	cnxnindex = cnxnindex;
}

void AppObjectCnxnAllocated (uchar cnxnindex)
{
#ifdef NewConnectionAllocationMethod
	switch (cnxnindex)
	{
	case POLL_CNXN:
		IOCnxnSize[CSI_P_CONS] = CompAssyCSize (); // poll consume size
		IOCnxnSize[CSI_P_PROD] = CompAssyPSize (); // poll produce size
		break;
	case STROBE_CNXN:
		IOCnxnSize[CSI_S_PROD] = 0; // strobe produce size
		break;
	case COS_CNXN:
		IOCnxnSize[CSI_C_PROD] = CompAssyPSize (); // COS produce size
		break;
	}
	// new data is not availible (old data is), but we need to resize the buffer.
	new_produce_data_avail = 1;
#else
	cnxnindex = cnxnindex;
#endif
}

//**********************************************************************
// Function		AppObjectFillProdPathLen()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectFillProdPathLen (uchar cnxn)
{
	cnxn				  = cnxn;
	CurrFragObj.buffer[2] = 6;
	CurrFragObj.buffer[3] = 0;
}

//**********************************************************************
// Function		AppObjectFillConsPathLen()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectFillConsPathLen (uchar cnxn)
{
	cnxn				  = cnxn;
	CurrFragObj.buffer[2] = 6;
	CurrFragObj.buffer[3] = 0;
}

//**********************************************************************
// Function		AppObjectFillProdPath()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectFillProdPath (uchar cnxn)
{
	cnxn				  = cnxn;
	CurrFragObj.buffer[2] = 0x20;
	CurrFragObj.buffer[3] = 0x4;
	CurrFragObj.buffer[4] = 0x24;
	CurrFragObj.buffer[5] = 101;
	CurrFragObj.buffer[6] = 0x30;
	CurrFragObj.buffer[7] = 0x3;
	CurrFragObj.numbytes  = 8;
}

//**********************************************************************
// Function		AppObjectFillConsPath()
//
// Edit History
//	[0]	15Jul99	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

void AppObjectFillConsPath (uchar cnxn)
{
	cnxn				  = cnxn;
	CurrFragObj.buffer[2] = 0x20;
	CurrFragObj.buffer[3] = 0x4;
	CurrFragObj.buffer[4] = 0x24;
	CurrFragObj.buffer[5] = 102;
	CurrFragObj.buffer[6] = 0x30;
	CurrFragObj.buffer[7] = 0x3;
	CurrFragObj.numbytes  = 8;
}

//**********************************************************************
// Function		AppObjectGMMConfigSet()
//
// Edit History
//	[0]	04Feb00	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

#define GMM_MSG_LENGTH 15 // 6 + number bytes(9)

void AppObjectGMMConfigSet ()
{
	unsigned char tempbyte;

	if ((CurrFragObj.buffindx != 6) && (CurrFragObj.buffindx < GMM_MSG_LENGTH))
		MessageObjectFormatErrorMessage (NOT_ENOUGH_DATA, ADD_CODE_NOT_SPECIFIED);
	else if (CurrFragObj.buffindx > GMM_MSG_LENGTH)
		MessageObjectFormatErrorMessage (TOO_MUCH_DATA, ADD_CODE_NOT_SPECIFIED);
	else // message correct length, time to decode message
	{
		if (CurrFragObj.buffindx == 6)
		{
			// no config data, so let's 'load' the default values

			CurrFragObj.buffer[INDX_DATA]	  = 0x80;
			CurrFragObj.buffer[INDX_DATA + 1] = 0x14;
			CurrFragObj.buffer[INDX_DATA + 2] = 0x08;
			CurrFragObj.buffer[INDX_DATA + 3] = 0x0D;
			CurrFragObj.buffer[INDX_DATA + 4] = 0x14;
			CurrFragObj.buffer[INDX_DATA + 5] = 0x83;
			CurrFragObj.buffer[INDX_DATA + 6] = 0x3A;
			CurrFragObj.buffer[INDX_DATA + 7] = 0x0D;
			CurrFragObj.buffer[INDX_DATA + 8] = 0x00;
		}

		// starting with first byte, let's get all the data.
		// Afterwards, we'll check the validity of all the data.

		// get the serial framing (7N2, etc)
		tempbyte	 = CurrFragObj.buffer[INDX_DATA];
		LocalFraming = tempbyte & 0x0F;

		// get the baud rate
		tempbyte >>= 4;
		LocalBaudRate = tempbyte & 0x07;

		// get the xmit pad mode
		tempbyte >>= 3;
		RRecPadMode = tempbyte; // last bit - no masking required  (& 0x01)

		// get the max transmit chars
		TxStrLen = CurrFragObj.buffer[INDX_DATA + 1];

		// get the xmit swap mode
		tempbyte = CurrFragObj.buffer[INDX_DATA + 2];
		SRecSwap = tempbyte & 0x03;

		// get the xmit end delim mode
		tempbyte >>= 2;
		TxStrDelimMode = tempbyte & 0x03;

		// get the swap header bytes parameter
		tempbyte >>= 2;
		SwapHdrBytes = tempbyte & 0x01;

		// get the xmit delim char
		TxStrDelim = CurrFragObj.buffer[INDX_DATA + 3];

		// get the max receive chars
		RRecRxStrMaxLen = CurrFragObj.buffer[INDX_DATA + 4];

		// get the use_rcv_length parameter
		tempbyte = CurrFragObj.buffer[INDX_DATA + 5];
		if (0x01 & tempbyte)
		{
			RRecRxStrType = (SwapHdrBytes) ? SHORT_STRING : STRING;
		}
		else
			RRecRxStrType = ARRAY;

		// get the xmit data type
		if (0x02 & tempbyte) // looking at the 2nd bit now
		{
			type = (SwapHdrBytes) ? SHORT_STRING : STRING;
		}
		else
			type = ARRAY;

		// get the rcv swap mode
		tempbyte >>= 2;
		RRecSwap = tempbyte & 0x03;

		// get the rcv start delim mode
		tempbyte >>= 2;
		StartMode = tempbyte & 0x03;

		// get the rcv end delim mode
		tempbyte >>= 2;
		EndMode = tempbyte; // don't have to mask - last 2 bits

		// get the rcv start delim char
		RRecStart = CurrFragObj.buffer[INDX_DATA + 6];

		// get the rcv end delim char
		RRecEnd = CurrFragObj.buffer[INDX_DATA + 7];

		// get the pad char
		// If not used, let's make it a null character.
		RRecPadChar = (RRecPadMode) ? CurrFragObj.buffer[INDX_DATA + 8] : 0;

		// now let's check the validity of the config data
		if (!GMM_config_data_valid ())
		{
			Mb_FactoryDefaults();
			AppObjectFactoryDefaults ();

			MessageObjectFormatErrorMessage (INVALID_ATTRIB_VALUE, ADD_CODE_NOT_SPECIFIED);
		}
		else
		{
			// any other parameters (not covered in GSD) to be set ?????
		}

		// parameter(s) set regardless of validity of config data
		SRecMode = 1; // 1 means produce all the time.  This is deceiving -
					  //  in GMMode, data only transmitted when a new msg
					  //  rcvd from PLC.

		// now, let's initialize all the hardware stuff
		InitSerialIO ();

		GMM_pad_buffers (); // seems that the first time thru, the pad byte
							//  has not been applied.

		RRecRcvMode	  = 1;
		AssyCRxRecNum = 0; // was (RRecRcvMode == 0) found in InitRxTxAssy()

		GMM_set_msg_pointers ();

		// in GMMode, only COS is supported
		IOCnxnSize[CSI_C_PROD] = 9; // COS produce size (instance 4)
		IOCnxnSize[CSI_P_CONS] = 8; // COS consume size (instance 2)

		// update the IO cnxn produce sizes within the msg objects
		CM_MCR1_Set_CPUUPD (XMIT_COS_MESSAGE_OBJECT, CM_ENABLED);

		CM_MCFG_Set_Direction (XMIT_COS_MESSAGE_OBJECT, CM_TRANSMIT);
		CM_MCFG_Set_Length (XMIT_COS_MESSAGE_OBJECT,
							IOCnxnSize[CSI_C_PROD] & 0xF); // TODO: value > 8 almost certainly invalid

		CM_MCR1_Set_CPUUPD (XMIT_COS_MESSAGE_OBJECT, CM_DISABLED);
	}
}

#ifdef GMM
static BOOL GMM_config_data_valid (void)
{
	if ((LocalFraming > 8) || (LocalBaudRate > 5) || (!TxStrLen) // must be non-zero
		|| (TxStrLen > 102) || (type > 2) || (SRecSwap > 3) || (TxStrDelimMode > 2) || (RRecPadMode > 1)
		|| (!RRecRxStrMaxLen) // must be non-zero
		|| (RRecRxStrMaxLen > 102) || (RRecRxStrType > 2) || (RRecSwap > 3) || (StartMode > 2) || (EndMode > 2))
		return (FALSE);
	else
	{
		return (TRUE);
	}
}
#endif // GMM

//**********************************************************************
// Function		AppObjectGMMStatus()
//
// Edit History
//	[0]	15Mar00	dsw	Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
#define GMM_ERR_NOERROR	  0x00
#define GMM_ERR_OVERRANGE 0x07
#define GMM_ERR_CONFIG	  0x0A
#define GMM_ERR_PARITY	  0x10 // device-specific CRS
#define GMM_CHANTYPE_BYT  0x80
#define GMM_DIR_IN		  0x80
#define GMM_DIR_OUT		  0x00
void AppObjectGMMStatus ()
{
#ifdef GMM
	unsigned char temp;

	if (CurrFragObj.buffer[INDX_DATA] > 0) // only supporting one channel
		MessageObjectFormatErrorMessage (INVALID_ATTRIB_VALUE, ADD_CODE_NOT_SPECIFIED);
	else
	{
		temp = current_status_byte ();

		CurrFragObj.buffer[2] = GMM_DIR_IN; // preload

		// check parity error first - seems most important
		if (temp & PARITY_ERR_BIT)
		{
			CurrFragObj.buffer[3] = (GMM_ERR_PARITY | GMM_CHANTYPE_BYT);
		}
		else if (temp & (RX_FIFO_OVERFLOW | TX_FIFO_OVERFLOW))
		{
			CurrFragObj.buffer[3] = (GMM_ERR_OVERRANGE | GMM_CHANTYPE_BYT);

			if (temp & TX_FIFO_OVERFLOW)
				CurrFragObj.buffer[2] = GMM_DIR_OUT;
		}
		else // no error
		{
			CurrFragObj.buffer[3] = (GMM_ERR_NOERROR | GMM_CHANTYPE_BYT);
		}

		// message length
		CurrFragObj.numbytes += 2;
	}
#endif // GMM
}

//**********************************************************************
// Function		AppObjectFillPBIdent()
//
// Edit History
//	[0]	13Sept00	dsw	Created
//
//             Copyright (c) 2000 Allen-Bradley Co.
//**********************************************************************

#define PB_IDENT 0xB7 // for 8 bytes
// #define PB_IDENT		0x73  // for 4 words
void AppObjectFillPBIdent ()
{
	CurrFragObj.buffer[2] = 0x01;	  // length
	CurrFragObj.buffer[3] = PB_IDENT; // profibus identifier
	CurrFragObj.numbytes += 2;
}
