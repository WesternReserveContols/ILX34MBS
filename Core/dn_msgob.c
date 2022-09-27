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
#include <string.h>

#include <xdatacpy.h>


#define CAN_INT_DISABLE (CP_Set_ECAN (0))
#define CAN_INT_ENABLE	(CP_Set_ECAN (1))

MessageObjectStructure MessageObjectRAM;
MessageObjectPktEntry  MessageObjectInPktQ;
MessageObjectPktEntry  MessageObjectOutPktQ;
MessageObjectPktEntry  CurrMessageObjectPkt;
unsigned char		   InPktsQd;  // number of packets queued
unsigned char		   OutPktsQd; // number of packets queued

MessageObjectFragObj CurrFragObj; // some day there will be one of these for
								  //   each explicit cnxn.

#ifdef GMM // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GMM

// values used by GMM_Frag_State
#define GMMFS_IDLE 0
#define GMMFS_OUT  1
#define GMMFS_IN   2
#define GMMFS_DONE 3 // done w/ a transaction, waiting for idle cmd

// bits defined in the 'status byte'
#define SB_TXBUF_OVRFLW	   0x01
#define SB_RXBUF_OVRFLW	   0x02
#define SB_RS_PARITY_ERR   0x04
#define SB_FRAG_ERROR	   0x40
#define SB_NEW_IN_DATA_AVL 0x80 // new input data available

// maximum string length when in GMMode
#define MAX_STR_LEN 102 // (14*7)+4

#define CMD_MASK 0x70 // where the command bits are
#define FRG_MASK 0x0F // where the frag bits are

#define CMD_IDLE 0x00
#define CMD_RCV	 0x10
#define CMD_SND	 0x20

#define PLC_IDLE	 0x00 // idle command w/ no count
#define XMT_1ST_BYTE (CMD_SND | 0x01)
#define REC_1ST_BYTE (CMD_RCV) // frag count is 0 for initial request
#define CONST_ZERO	 0x00	   // pad byte value that's always 0

static uchar CntrlByteSave; // stores the last control byte value
#ifndef SIM_CONSUME
static uchar Exp_Len;		// expected length - based on len bytes in msg
#endif
static uchar GMM_Frag_State;
static bool	 GMMFragError;

// these will point to CAN data byte registers
static uchar *CtlBytPtr;  // control/frag byte
static uchar *StatBytPtr; // status byte or pad byte
static uchar *LenPtr;	  // byte containing length (if used)
static uchar *Len0Ptr;	  // len byte always set to 0

// indeces into C_OutMsgBuffer[] array
static uchar OutCtlIndx;
static uchar OutStatIndx;
static uchar OutLenIndx;
static uchar OutLen1Indx;

void		  GMM_ser_data_rcvd (void);
void		  GMM_set_msg_pointers (void);
unsigned char local_status_byte (void);
#ifndef SIM_CONSUME
static void	  idle_w_frag_error (void);
#endif

bool SwapHdrBytes;

extern unsigned char RRecStatus;
extern unsigned char RRecRxStrMaxLen;
extern unsigned char RRecRxStrType;
extern unsigned char mainloopassydata[];
extern unsigned char size_of_mainloopassydata;
extern unsigned char TxStrLen;
extern unsigned char type; // xmt data type
extern BOOL			 AppObjectCOS_Bit;

extern unsigned char current_status_byte (void);
// values used by 'type'
#define ARRAY		 0
#define SHORT_STRING 1
#define STRING		 2

#define RESET_CTRL_BYTE_SAVE (CntrlByteSave = 0x80)

#endif // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GMM

#ifdef FCL
#define FRAG_TYPE_MASK 0xC0
static uchar ConsFragCnt;
// for a description of the following variables, see app_objs.h
uchar ConsFragPtr;
uchar P_InMsgBuffer[FCL];
#endif // FCL
#ifdef FPL
// for a description of the following variables, see app_objs.h
uchar P_FragCount;
uchar C_FragCount;
uchar P_OutMsgBuffer[FPL]; // Fragmented Produce Len
uchar C_OutMsgBuffer[FPL]; // Fragmented Produce Len
// Ed Riede, Western Reserve Controls 2001-11-08
// (c) Western Reserve Controls
// added to allow variable poll response sizes
uchar P_OutMsgBufferSize = 0;
uchar C_OutMsgBufferSize = 0;
// End Wrc
#endif // FPL

#ifdef GMM
// serial data was received.  If we're not in the middle of a transaction,
//  we should send a COS message.
void GMM_ser_data_rcvd (void)
{
	CAN_INT_DISABLE;
	if (GMMFS_IDLE == GMM_Frag_State)
	{
		// fill output bytes sent in COS msg
		C_OutMsgBuffer[OutCtlIndx]	= 0x00; // idle
		C_OutMsgBuffer[OutStatIndx] = local_status_byte ();
		C_OutMsgBuffer[8]			= GMMRAM.IOStatus;

		// trigger COS production with new data
		AppObjectCOS_Bit = 1;
	}
	CAN_INT_ENABLE;
}

// set some data pointers and array indeces based on whether or not the
//  user has selected to swap the 'header' bytes (first couple of bytes
//  in each fragment).
void GMM_set_msg_pointers (void)
{
	if (SwapHdrBytes)
	{
		CtlBytPtr  = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB1);
		StatBytPtr = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0);
		LenPtr	   = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB3);
		Len0Ptr	   = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB2);

		// indeces into C_OutMsgBuffer[] array
		OutCtlIndx	= 1;
		OutStatIndx = 0;
		OutLenIndx	= 3;
		OutLen1Indx = 2;
	}
	else
	{
		CtlBytPtr  = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0);
		StatBytPtr = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB1);
		LenPtr	   = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB2);
		Len0Ptr	   = &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB3);

		// indeces into C_OutMsgBuffer[] array
		OutCtlIndx	= 0;
		OutStatIndx = 1;
		OutLenIndx	= 2;
		OutLen1Indx = 3;
	}
}

// this routine calls current_status_byte() to get status byte
//  used by 'base' ascii and then adds the bit status for
//  GMM fragmentation error
unsigned char local_status_byte (void)
{
	unsigned char tempb;

	tempb = current_status_byte ();
	if (GMMFragError)
		tempb |= SB_FRAG_ERROR;

	return (tempb);
}

#ifndef SIM_CONSUME
// this was being set in enough places to warrant its own routine.
static void idle_w_frag_error (void)
{
	GMM_Frag_State = GMMFS_IDLE;

	// fill output bytes sent in COS msg
	C_OutMsgBuffer[OutCtlIndx] = CMD_IDLE;

	GMMFragError = 1; // set the frag error bit in the status byte
}
#endif
#endif // GMM

extern BOOL COSACKRcvd;
//***********************************************************************
//
// Function                  MessageObjectInit()
//
// Author                    Anatoly Grinberg
// Date                      9/3/1998
//
// Functional Description    This function is used to clear Message Object RAM
//
// Inputs
//
// Outputs
//
// Edit History      [0]   9/3/1998    AG   Created
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectInit ()
{
	MessageObjectRAM.bCommParamChange	 = 0;
	MessageObjectRAM.InputStrobeBytePntr = 0;
	MessageObjectRAM.cInputStrobeBitMask = 0;
	MessageObjectRAM.cCurrentMessage	 = 0;
	MessageObjectRAM.Grp4WhoCntr		 = 0xFFFF; // timer not running

	InPktsQd  = 0;
	OutPktsQd = 0;

	// fragmentation stuff
	CurrFragObj.buffindx = 0;
	CurrFragObj.fragbyte = 0;
	CurrFragObj.p_flag	 = PRODUCTION_COMPLETE;

#ifdef FCL // FragReceiveLength - frag reception enabled
	ConsFragCnt = 0;
#endif // FCL

#ifdef FPL			 // FragProduceLength - frag production enabled
	P_FragCount = 0; // production frag cnt for poll
	C_FragCount = 0; // production frag cnt for cos
#endif				 // FPL

#ifdef GMM
	RESET_CTRL_BYTE_SAVE; // set to invalid value
	GMM_Frag_State = GMMFS_IDLE;
	GMMFragError   = 0;
#endif // GMM
}

//***********************************************************************
//
// Function                 MessageObjectDupMACMessageSend()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function is used to send Duplicate MAC ID
//                      messages(both response and requests).
//            The XMT_BUF[0] must be set with either the REQUEST or RESPONSE
//                      dup MacId check message indicator.
//            We're purposely not using the explicit Q structure here.  The
//                   message can't wait for the other explicit messages.
//
// Inputs      cpMessageType - REQUEST or RESPONSE
//
// Outputs      None
//
// Edit History        [1]   3/24/94   BJT   Add to IDN 9000 code
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectDupMACMessageSend (unsigned char cpMessageType)
{
	// wait until buffer is available
	//  (Disable for emulation rab)

	// while ((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) != IS_CLEAR_TXRQ)
	while (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_PENDING)
	{
		TimerObjectSvcTimer ();

		if (CANBUSOFF & UIObjectRAM.cHealthState)
			return; // probably trying to go online at wrong baud rate
	}

	// Setup Message Control Registers
	CM_MCR1_Set_NEWDAT (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_ENABLED);

	// Build the DUPLICATE MACID CHECK Identifier Field
	CM_Set_UAR1 (XMIT_EXPLICIT_MESSAGE_OBJECT, (DUPLICATE_CHECK_MSGID << 5));

	CM_MCFG_Set_Direction (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_EXPLICIT_MESSAGE_OBJECT, DUP_MAC_LEN);

	// Insert the type, Device vendor Id and serial number into the xmt buffer
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB0 = cpMessageType;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB1 = IdentityObjectRAM.vendor[0];
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB2 = IdentityObjectRAM.vendor[1];
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB3 = IdentityObjectRAM.serial_number[0];
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB4 = IdentityObjectRAM.serial_number[1];
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB5 = IdentityObjectRAM.serial_number[2];
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB6 = IdentityObjectRAM.serial_number[3];

	// Transmit the message
	CM_MCR1_Set_NEWDAT (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
}

BOOL MessageObjectCOSMsgObjAvailable (void)
{
	return CM_MCR1_Get_TXRQ (XMIT_COS_MESSAGE_OBJECT) == CM_NOT_PENDING;
}

//***********************************************************************
//
// Function                  MessageObjectSendCOSMessage()
//
// Author                    Brian J. Taylor
// Date                      9/7/97
//
// Functional Description    This function sends a COS message using the
//                     predefined master slave connection set.
//
// Inputs      cpData1:2 - data to send in message
//
// Outputs      None
//
// Edit History      [0]   9/7/97   BJT   Created
//         [1]   2/23/98   BJT   Return TRUE/FALSE if COS message sent.
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/

unsigned char MessageObjectSendCOSMessage (unsigned char new_data, unsigned char reset_timers)
{
	/*
	// Only produce if allocated and not inhibited
	*/
	if (ConnectionObjectRAM[COS_CNXN].state == CNXN_ESTABLISHED)
	{
		// Make sure buffer is available
		while (!MessageObjectCOSMsgObjAvailable ())
		{
			TimerObjectSvcTimer ();
		}

		// Setup Message Control Registers
		CM_MCR1_Set_NEWDAT (XMIT_COS_MESSAGE_OBJECT, CM_PENDING);
		CM_MCR1_Set_CPUUPD (XMIT_COS_MESSAGE_OBJECT, CM_ENABLED);

		// don't need to Load the IDENTIFIER and MSG LENGTH for the COS request - preloaded

		/*
		// Fill in data values into transmit buffer
		*/
		if (new_data)
		{
			// added ER @ Western Reserve COntrols (c) 2002-9-24, for backwards compatibility
			// with previous pointbus code
			C_OutMsgBufferSize = IOCnxnSize[CSI_C_PROD];
			// end added ER

			AppObjectFillCOSData ();
		}
		// else use old data previously loaded

#ifdef FPL
		if (IOCnxnSize[CSI_C_PROD] > 8) // are we sending fragmented data
		{
			// build first fragment
			// MOdified to allow variable poll sizes
			CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB0 = // FIRST_FRAG
				(C_OutMsgBufferSize > 7) ? (FIRST_FRAG) : (0x3f);

			xdata_memcpy (&CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB1, &C_OutMsgBuffer[0], 7);

			// Modified to allow variable poll sizes
			C_FragCount = (C_OutMsgBufferSize > 7) ? 1 : 0;

			CM_MCFG_Set_Direction (XMIT_COS_MESSAGE_OBJECT, CM_TRANSMIT);
			CM_MCFG_Set_Length (XMIT_COS_MESSAGE_OBJECT,
								((C_OutMsgBufferSize > 7)
									 ? (0x8)
									 : ((C_OutMsgBufferSize + 1))) // JSW TODO: test/review potentially troublesome line
			);
		}
		else // not fragmented - even though device is capable
		{
			xdata_memcpy (&CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB0, &C_OutMsgBuffer[0], 8);
			C_FragCount = 0; // don't send more fragments
			// set the data length
			// Modified to allow variable poll sizes
			CM_MCFG_Set_Direction (XMIT_COS_MESSAGE_OBJECT, CM_TRANSMIT);
			CM_MCFG_Set_Length (XMIT_COS_MESSAGE_OBJECT,
								C_OutMsgBufferSize & 0xF); // TODO: value > 8 almost certainly invalid
														   // End WRC
		}
#endif // FPL

		/*
		// Transmit the message
		*/
		CM_MCR1_Set_NEWDAT (XMIT_COS_MESSAGE_OBJECT, CM_PENDING);
		CM_MCR1_Set_CPUUPD (XMIT_COS_MESSAGE_OBJECT, CM_DISABLED);
		CM_MCR1_Set_TXRQ (XMIT_COS_MESSAGE_OBJECT, CM_PENDING);

		/*
		// The acktimer will be started when the last frag is transmitted (in
		//   the ISR).
		// Clear them now to prevent them from timing out in the meantime.
		*/
		AckHandlerObjectRAM.AckTimer[0] = 0xff;
		AckHandlerObjectRAM.AckTimer[1] = 0xff;
		// 0's will cause an immediate ack!, we want 8 mins to ack!
		// major kludge, but if it take 8 mins to produce a message,
		// we are in trouble anyways
		// er

		if (reset_timers)
		{
			// Reset the retry limit
			AckHandlerObjectRAM.RetryCount = DEFAULT_ACK_RETRY;

			// Reset the transmission trigger timer
			AckHandlerObjectRAM.XmtTrigTimer[0] = ConnectionObjectRAM[COS_CNXN].expPacketRate[0] >> 3;
			AckHandlerObjectRAM.XmtTrigTimer[1] = (ConnectionObjectRAM[COS_CNXN].expPacketRate[0] << 5)
												  + (ConnectionObjectRAM[COS_CNXN].expPacketRate[1] >> 3);

			// Reset the inhibit timer
			AckHandlerObjectRAM.InhibitTimer[0] = ConnectionObjectRAM[COS_CNXN].InhibitTime[0] >> 3;
			AckHandlerObjectRAM.InhibitTimer[1] = (ConnectionObjectRAM[COS_CNXN].InhibitTime[0] << 5)
												  + (ConnectionObjectRAM[COS_CNXN].InhibitTime[1] >> 3);
		}
		else
			AckHandlerObjectRAM.RetryCount--;

		return (TRUE);
	}
	else
		return (FALSE);
}
//***********************************************************************
//
// Function                  MessageObjectSendCOSAck()
//
// Author                    CRSchumaker
// Date                      11/13/98
//
// Functional Description    Send an ack to a rcvd COS message over cnxn
//                     instance 2.  (our master sent a cos msg).
//
// Inputs       None
//
// Outputs      None
//
// Edit History      [0]   11/13/98   CRS   Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/
void Timer_Thread_Yield (void);
void MessageObjectSendCOSAck ()
{
	/*
	// Make sure buffer is available
	*/

	while (CM_MCR1_Get_TXRQ (XMIT_POLL_MESSAGE_OBJECT) == CM_PENDING)
	{
		Timer_Thread_Yield ();
	}

	/*
	// identifier and mesage length(0) were already loaded for this message object
	*/

	/*
	// Transmit the message
	*/
	CM_MCR1_Set_NEWDAT (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_POLL_MESSAGE_OBJECT, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
}

//***********************************************************************
//
// Function                  MessageObjectSendPollMessage()
//
// Author                    CRSchumaker
// Date                      11/6/98
//
// Functional Description    This function sends a poll message using the

//                   predefined master slave connection set.
//
// Inputs      cpData1:2 - data to send in message
//
// Outputs      None
//
// Edit History      [0]   11/6/98   CRS   replicated messageobjectsendstrobemessage()
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectSendPollMessage (void)
{
	/*
	// Make sure buffer is available
	*/

	while (CM_MCR1_Get_TXRQ (XMIT_POLL_MESSAGE_OBJECT) == CM_PENDING)
	{
		Timer_Thread_Yield ();
	}

	/*
	// Setup Message Control Registers
	*/
	CM_MCR1_Set_NEWDAT (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_POLL_MESSAGE_OBJECT, CM_ENABLED);

	/*
	// Fill in data values into transmit buffer
	*/

	// added ER @ Western Reserve COntrols (c) 2002-9-24, for backwards compatibility
	// with previous pointbus code
	P_OutMsgBufferSize = IOCnxnSize[CSI_P_PROD];
	// end added ER

	AppObjectFillPollData ();

#ifdef FPL
	if (IOCnxnSize[CSI_P_PROD] > 8) // are we sending fragmented data
	{


		// build first fragment
		// MOdified to allow variable poll sizes
		CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0 = // FIRST_FRAG
			(P_OutMsgBufferSize > 7) ? (FIRST_FRAG) : (0x3f);
		xdata_memcpy (&CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB1, &P_OutMsgBuffer[0], 7);

		// Modified to allow variable poll sizes
		P_FragCount = (P_OutMsgBufferSize > 7) ? 1 : 0;
		// let's set the data length to be 8 bytes
		CM_MCFG_Set_Direction (XMIT_POLL_MESSAGE_OBJECT, CM_TRANSMIT);
		CM_MCFG_Set_Length (
			XMIT_POLL_MESSAGE_OBJECT,
			((P_OutMsgBufferSize > 7) ? (0x8) : ((P_OutMsgBufferSize + 1))) // JSW TODO: review/test line
		);
	}
	else // not fragmented - even though device is capable
	{
		xdata_memcpy (&CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0, &P_OutMsgBuffer[0], 8);
		P_FragCount = 0; // don't send more fragments
		// Modified to allow variable poll sizes
		CM_MCFG_Set_Direction (XMIT_POLL_MESSAGE_OBJECT, CM_TRANSMIT);
		CM_MCFG_Set_Length (XMIT_POLL_MESSAGE_OBJECT,
							P_OutMsgBufferSize & 0xF); // TODO: value > 8 almost certainly invalid
	}
#endif // FPL

	/*
	// Transmit the message
	*/
	CM_MCR1_Set_NEWDAT (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_POLL_MESSAGE_OBJECT, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
}

//***********************************************************************
//
// Function                  MessageObjectSendStrobeMessage()
//
// Author                    Brian J. Taylor
// Date                      9/7/97
//
// Functional Description    This function sends a strobe message using the
//                   predefined master slave connection set.
//
// Inputs      cpData1:2 - data to send in message
//
// Outputs      None
//
// Edit History      [0]   9/7/97   BJT   Created
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/
/* remove for 232 module
void MessageObjectSendStrobeMessage( void )
{
   // Make sure buffer is available
   while (( CANMessageObject[ XMIT_STROBE_MESSAGE_OBJECT ].MCR1 & 0x30 ) != 0x10 ); //

   // Setup Message Control Registers
   CANMessageObject[ XMIT_STROBE_MESSAGE_OBJECT ].MCR1 = SET_NEWDAT & SET_CPUUPD; //

   // Fill in data values into transmit buffer
   AppObjectFillStrobeData();

   // Transmit the message
   CANMessageObject[ XMIT_STROBE_MESSAGE_OBJECT ].MCR1 = //
									SET_NEWDAT & RESET_CPUUPD & SET_TXRQ; //
}
end of remove for 232 module */
//***********************************************************************
//
// Function                  MessageObjectProcessCommFltdMsg()
//
// Author                    CRSchumaker
// Date                      12/02/98
//
// Functional Description    process a group 4 message
//
// Inputs       None
//
// Outputs      None
//
// Edit History      [0]   12/02/98   CRS   Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/
void MessageObjectProcessCommFltdMsg ()
{
	unsigned char macid, msglen, temp;

	macid  = CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 & 0x3f;
	msglen = CM_MCFG_Get_Length (RCV_COMM_FLTD_MSG_OBJ);

	switch (CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB1) // service byte
	{
	case 0x4B: // WHO request
		// message must be 3 bytes long
		//  and timedelay value must be 1-6
		if (3 == msglen && CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2 >= 0
			&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2 < 7)
		{
			// check if we're correct macid (mask or match)
			if (((CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 & MATCH_BIT) && (macid == DeviceNetObjectRAM.macId))
				|| (!(CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 & MATCH_BIT)
					&& ((macid & DeviceNetObjectRAM.macId) == DeviceNetObjectRAM.macId)))

			{
				// ok to process the message
				// we're gonna respond with a who response, but not until the timeout expires

				// timeout value is based on what will be transmitted one of the bytes
				//    (1-6) of the response message.
				switch (CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2)
				{
				case 0:
					temp = 0;
					break;
				case 1:
					temp = IdentityObjectRAM.vendor[0];
					break;
				case 2:
					temp = IdentityObjectRAM.vendor[1];
					break;
				default:
					temp = IdentityObjectRAM.serial_number[CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2 - 3];
					break;
				}
				// must wait for before responding on the wire (using 8 ms clock)
				MessageObjectRAM.Grp4WhoCntr = (unsigned int)(50 * temp) / 8;
			}
		}
		break;

	case 0x4C: // IDENTIFY request
		// message can be either 2 or 8 bytes long
		if (2 == msglen)
		{
			// check if we're correct macid (mask or match)
			if (((CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 & MATCH_BIT) && (macid == DeviceNetObjectRAM.macId))
				|| (!(CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 & MATCH_BIT)
					&& ((macid & DeviceNetObjectRAM.macId) == DeviceNetObjectRAM.macId)))

			{
				// multicast request

				MessageObjectSendGrp4IdentifyMsg ();
			}
		}
		else if (8 == msglen)
		{
			// point-to-point request
			// this one involves turning on flashing red/green leds and
			//   starting timers   ARGHHHHH!

			// spec says we can ignore byte 0
			if (CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2 == IdentityObjectRAM.vendor[0]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB3 == IdentityObjectRAM.vendor[1]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB4 == IdentityObjectRAM.serial_number[0]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB5 == IdentityObjectRAM.serial_number[1]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB6 == IdentityObjectRAM.serial_number[2]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB7 == IdentityObjectRAM.serial_number[3])
			{
				// do the timer/led stuff
				UIObjectNetLEDOnceRedGreen ();

				// and we have to respond on the wire too
				MessageObjectSendGrp4IdentifyMsg ();
			}
		}
		break;

	case 0x4D: // CHANGE_MACID request
		// message must be 8 bytes long
		if (8 == msglen)
		{
			if (CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB2 == IdentityObjectRAM.vendor[0]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB3 == IdentityObjectRAM.vendor[1]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB4 == IdentityObjectRAM.serial_number[0]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB5 == IdentityObjectRAM.serial_number[1]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB6 == IdentityObjectRAM.serial_number[2]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB7 == IdentityObjectRAM.serial_number[3]
				&& CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0 < 64) // has valid macid
			{
				// change our macid
				DeviceNetObjectRAM.macId = CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0;

				MessageObjectRAM.bCommParamChange = CHANGED_COME_UP_FAST;

				EEPROMObjectWriteAndUpdate (NVS_MAC_ID, DeviceNetObjectRAM.macId);
				EEPROMObjectWriteAndUpdate (NVS_BAUD_RATE, DeviceNetObjectRAM.baudrate);
			}
		}
		break;
	}
}

//***********************************************************************
//
// Function                  MessageObjectSendGrp4WhoMessage()
//
// Author                    CRSchumaker
// Date                      12/3/98
//
// Functional Description    This function sends a group 4 who response msg.
//
// Inputs       None
//
// Outputs      None
//
// Edit History      [0]   12/3/98   CRS   replicated messageobjectsendstrobemessage()
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/
void MessageObjectSendGrp4WhoMessage ()
{
	// Setup Message Control Registers
	CM_MCR1_Set_NEWDAT (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_COMM_FLTD_MSG_OBJ, CM_ENABLED);

	CM_MCFG_Set_Direction (XMIT_COMM_FLTD_MSG_OBJ, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_COMM_FLTD_MSG_OBJ, 7);

	// Insert the type, Device vendor Id and serial number into the xmt buffer
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB0 = 0x00; // port 0
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB1 = IdentityObjectRAM.vendor[0];
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB2 = IdentityObjectRAM.vendor[1];
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB3 = IdentityObjectRAM.serial_number[0];
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB4 = IdentityObjectRAM.serial_number[1];
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB5 = IdentityObjectRAM.serial_number[2];
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB6 = IdentityObjectRAM.serial_number[3];

	// Transmit the message
	CM_MCR1_Set_NEWDAT (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_COMM_FLTD_MSG_OBJ, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
}

//***********************************************************************
//
// Function                  MessageObjectSendGrp4IdentifyMsg()
//
// Author                    CRSchumaker
// Date                      12/4/98
//
// Functional Description    This function sends a group 4 identify response msg.
//
// Inputs       None
//
// Outputs      None
//
// Edit History      [0]   12/4/98   CRS   replicated messageobjectsendstrobemessage()
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/
void MessageObjectSendGrp4IdentifyMsg ()
{
	// Setup Message Control Registers
	CM_MCR1_Set_NEWDAT (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_COMM_FLTD_MSG_OBJ, CM_ENABLED);

	CM_MCFG_Set_Direction (XMIT_COMM_FLTD_MSG_OBJ, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_COMM_FLTD_MSG_OBJ, 2);

	// echo received bytes
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB0 = CANMessageObject[RCV_COMM_FLTD_MSG_OBJ].DB0;
	CANMessageObject[XMIT_COMM_FLTD_MSG_OBJ].DB1 = 0xCC;

	// Transmit the message
	CM_MCR1_Set_NEWDAT (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_COMM_FLTD_MSG_OBJ, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_COMM_FLTD_MSG_OBJ, CM_PENDING);
}

//***********************************************************************
//
// Function                  MessageObjectFormatErrorMessage()
//
// Author                    Brian J. Taylor
// Date                      9/4/97
//
// Functional Description    This function formats an error message using
//             the predefined master slave connection set.
//            The mesage is setup in the transmit regsiters and buffers.
//
// Inputs      cpErrorCode - Error code
//          cpAdditionalErrorCode - Additional Error code
//
// Outputs      None
//
// Edit History      [0]   9/4/97   BJT   Created
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectFormatErrorMessage (unsigned char cpErrorCode, unsigned char cpAdditionalErrorCode)
{
	/*
	// Fill in data values into transmit buffer
	//  Byte  0     : message header
	//  Byte  1   : RSP Bit set(Bit 7 ) and error service code
	//  Bytes 2   : Error Code
	//  Bytes 3   : Additional Error Code
	*/
	CurrFragObj.numbytes  = 4;
	CurrFragObj.buffer[1] = RSP_BIT | ERROR_RSP;
	CurrFragObj.buffer[2] = cpErrorCode;
	CurrFragObj.buffer[3] = cpAdditionalErrorCode;
}

//***********************************************************************
//
// Function                  MessageObjectStrobeBuildMask()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Inputs      DeviceNetObjectRAM - IDN Object (For device MAC ID)
//          RCV_BUF - Receive Buffer
//
// Outputs      DNG_pcStrobeBytePntr - Pointer to byte in Strobe
//                 request with Devices Strobe Input
//         DNG_cStrobeBitMask - Bit mask to mask out the Strobe Input bit.
//
// Edit History      [1]   3/25/94   BJT   Added to IDN 9000 Code.
//                 Fixed BUG caused by left shifting instead
//                  of right shifting to get bit mask.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectStrobeBuildMask ()
{
	unsigned char clStrobeBitNumber;

	/*
	// Set Input Strobe pointer to the byte in the Receive buffer
	// that contains our STROBE bit.
	*/
	MessageObjectRAM.InputStrobeBytePntr = (DeviceNetObjectRAM.macId >> 3);

	/*
	// Line the Input Strobe Bit Mask up with our STROBE bit position
	// in the byte pointed to by InputStrobeBytePntr
	*/
	MessageObjectRAM.cInputStrobeBitMask = 0x01;
	clStrobeBitNumber					 = DeviceNetObjectRAM.macId & 0x07;
	while (--clStrobeBitNumber)
		MessageObjectRAM.cInputStrobeBitMask = MessageObjectRAM.cInputStrobeBitMask << 1;
}
//***********************************************************************
//
// Function                  MessageObjectCheckInPktQ()
//
// Author                    CRSchumaker
// Date                      06/15/99
//
// Description    check input message queues for received (unprocessed) msgs
//
// Inputs      None
//
// Outputs      None
//
//         [0]   06/15/99 CRS   Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//***********************************************************************/

#define MAX_IN_PACKETS 5

/* This is intended as a temporary modification for the emulator.
 The addition of the buffer is to allow conformance testing to pass
 Items from InBuffer get copied to MessageObjectInPktQ each time
 MessageObjectCheckInPktQ is called. It is not impmeneted efficiently.
*/

MessageObjectPktEntry InBuffer[MAX_IN_PACKETS];
unsigned int		  MESSAGES_PROCESSED = 0;

void MessageObjectCheckInPktQ ()
{
	uint8_t i = 0;

	//  first check the input Q
	if (InPktsQd)
	{

		/*
			Originally this routine copied from MessageObjectInPktQ to CurrMessageObjectPkt
			Now InBuffer is used instead
		*/
		MESSAGES_PROCESSED++;
		// first we have to get the message to process

		CAN_INT_DISABLE; // ----------------------disabled---------------

		xdata_memcpy (&CurrMessageObjectPkt, &(InBuffer[0]), sizeof (MessageObjectPktEntry));

		// inefficient, move each of the buffer entries down one index (even if they are empty)
		for (i = 1; i < MAX_IN_PACKETS; i++)
			xdata_memcpy (&InBuffer[i - 1], &InBuffer[i], sizeof (MessageObjectPktEntry));

		memset (&InBuffer[MAX_IN_PACKETS - 1], 0, sizeof (MessageObjectPktEntry)); // clear top entry for good measure

		// decrement the number of messages queued
		InPktsQd--;

		CAN_INT_ENABLE; // ----------------------enabled----------------

		// now we can process the message
		MessageObjectProcessInPkt ();
	}
}

//***********************************************************************
//
// Function                  MessageObjectCheckPktQs()
//
// Author                    CRSchumaker
// Date                      12/16/98
//
// Description    check message queues for received (unprocessed) msgs
//
// Inputs      None
//
// Outputs      None
//
//         [0]   12/16/98 CRS   Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/
unsigned long MESSAGES_SENT = 0;
void		  MessageObjectCheckPktQs ()
{
	//  first check the input Q
	MessageObjectCheckInPktQ ();

	//  now check the output Q  - make sure explicit send msg obj free
	// if (OutPktsQd && ((CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].MCR1 & MASK_TXRQ) == IS_CLEAR_TXRQ))
	if (OutPktsQd && (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_NOT_PENDING))
	{
		// now place the packet into the message object
		MessageObjectXmitExpPkt ();

		// decrement the number of messages queued
		OutPktsQd--;
		MESSAGES_SENT++;
	}
}
//***********************************************************************
//
// Function                  MessageObjectXmitExpPkt()
//
// Author                    CRSchumaker
// Date                      02/16/99
//
// Description    place exp msg packet into can message object for transmission
//
//         [0]   12/16/98 CRS   Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectXmitExpPkt (void)
{
	CAN_INT_DISABLE;
	// shouldn't have to check if buffer available.  if it wasn't, we wouldn't be here

	// Setup Message Control Registers
	CM_MCR1_Set_NEWDAT (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_ENABLED);

	// Fill in Message Configuration Register with Explicit "C/S" Response and data length
	CM_Set_UAR1 (XMIT_EXPLICIT_MESSAGE_OBJECT, (EXPLICIT_MESSAGE_RSP << 5));

	CM_MCFG_Set_Direction (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_EXPLICIT_MESSAGE_OBJECT,
						MessageObjectOutPktQ.cMsgLen & 0xF); // TODO: value > 8 almost certainly invalid

	// Fill in data values into transmit buffer
	//   Byte  0     : (header filled in MessageObjecProduce)
	//   Byte  1       : RSP Bit set(Bit 7 ) and service from receive buffer
	//  Bytes 2 - 7 : Data to be returned
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB0 = MessageObjectOutPktQ.DB0;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB1 = MessageObjectOutPktQ.DB1;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB2 = MessageObjectOutPktQ.DB2;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB3 = MessageObjectOutPktQ.DB3;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB4 = MessageObjectOutPktQ.DB4;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB5 = MessageObjectOutPktQ.DB5;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB6 = MessageObjectOutPktQ.DB6;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB7 = MessageObjectOutPktQ.DB7;

	// now send it out on the wire
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);

	// updating the output Q head/tail pointers in calling routine
	CAN_INT_ENABLE;
}

//***********************************************************************
//
// Function                  MessageObjectProcessInPkt()
//
// Author                    Brian J. Taylor
// Date                      5/7/97
//
// Inputs      None
//
// Outputs      None
//
//         [2]   10/15/98 AG   Added Counter 2 for LimitSwitch.
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************/
unsigned int COUNT_MSG_OBJ_CONSUME = 0;
void		 MessageObjectProcessInPkt ()
{
	/*
	// Have special to handle Duplicate MAC ID message
	*/
	if ((DUPLICATE_CHECK_MSGID == CurrMessageObjectPkt.cMSGID)
		&& (ABAUD_NOTAUTOB == DeviceNetObjectRAM.bAutoBaud)) // we're not autobauding
	{
		/*
		// If received a duplicate macId check message and we aren't Online
		// then or we received a dup mac check response message
		// we can't go online
		*/
		if ((!DeviceNetObjectRAM.DeviceOnLine) || (CurrMessageObjectPkt.DB0 == RESPONSE))
			DeviceNetObjectDUPMACFail ();

		/*
		// We received a duplicate macId check message and we're online,
		// return a duplicate MAC ID check response message.
		*/
		else
			MessageObjectDupMACMessageSend (RESPONSE);
	}

	/*
	// if online then consume the message and produce the appropriate response
	*/
	else if (DeviceNetObjectRAM.DeviceOnLine)
	{
		/*
		// If receieve data and in deferred delete state set the connection to established
		*/
		if ((CurrMessageObjectPkt.cMSGID == EXPLICIT_MESSAGE_REQ)
			&& (ConnectionObjectRAM[EXPLICIT_CNXN].state == CNXN_DEFERRED_DELETE))
			ConnectionObjectRAM[EXPLICIT_CNXN].state = CNXN_ESTABLISHED;

		/*
		// Establish a pointer to the Explicit cnxn instance
		// if either a Master Explicit or Unconnected Explicit request.
		*/
		if (((CurrMessageObjectPkt.cMSGID == EXPLICIT_MESSAGE_REQ)
			 && (ConnectionObjectRAM[EXPLICIT_CNXN].state == CNXN_ESTABLISHED))
			|| (CurrMessageObjectPkt.cMSGID == GRP2_UNCONNECTED_REQ))
		{
			/*
			// Consume the Explicit message by routing the message to
			// the appropriate Object and then produce a response
			// to the message received.
			*/
#if (EXPL_CNXN_SIZE <= 7)
			if ((CurrMessageObjectPkt.DB0 & FRAGMENTATION_MASK) == 0)
			{
				// add here for 'frag error' message
			}
			else
#endif
			{
				COUNT_MSG_OBJ_CONSUME++;
				MessageObjectConsume ();
			}
			// packets are placed into a Q which is serviced in the main loop
		}
	}
}

//***********************************************************************
//
// Function                  MessageObjectConsume
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function is used to route the Connection Based Explicit request message has been
//            received.  This routine extracts the address and data from connection based messages and
//            distributes them to the appropriate object handler.
//
// Inputs      RCV_BUF - Receive Buffer
//
// Outputs      Return Code - Status of message consumption
//
// Edit History      [1]   3/25/94   BJT   Added to IDN 900 code.
//         [2]   5/4/94   BJT   Added check of non ID Set service messages under unconnected message type.
//         [3]   5/27/94   BJT   Added message and Assembly Object support.
//         [4]   6/30/94   BJT   Added message and Parameter Object support.
//         [5]   8/9/94   BJT   Added DeviceLink support. DeviceLink doesn't support Sensor, Assembly,
//                  and Parameter Object, but does support the Discrete Input Point Object.
//         [6]   8/10/94   BJT   DeviceLink will support a status value so it must support assembly object.
//         [7]   8/18/94   BJT   Need to return Object doesn't exists for error instances we don't support.
//         [8]   8/22/94   BJT   Support factory setup portion of sensor object in DeviceLink.
//         [9]   9/23/94   BJT   Must moved factory setup into Discrete Input point Object
//         [10]   9/28/94   BJT   DeviceLink supports parameter Object.
//         [11]   9/30/94   BJT   DeviceLink supports 2 instances of the parameter object so fix cases that
//                  verifying instance number to allow parameter object for DeviceLink.
//         [12]   10/7/94   BJT   Clear Transmit buffer bytes 3 and 4 here so that individual objects don't
//                  need to.  This saves ROM, which we can use!!!
//         [13]   4/5/95   BJT   Added Discrete Output Point support
//         [14]   6/5/95   BJT   Added Support for DeviceNet Configurator Tool.  Call Configuration
//                  Tool Object.
//         [15]   3/17/97   BJT   Added ACk handler Object suuport required for COS.
//         [16]   3/17/98   BJT   Added Timer and Counters
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

unsigned int COUNT_DEVICENET_OBJ = 0;
void		 MessageObjectConsume ()
{
	unsigned char *sptr, *dptr, fragtype, bytecount;
	unsigned char  consumption_status;

	// handle fragmentation protocol here
	if (CurrMessageObjectPkt.DB0 & FRAG_BIT)
	{
		fragtype = CurrMessageObjectPkt.DB1 & 0xc0; // 1st, last, middle, ack ...

		if (FRAG_ACK == fragtype) // don't want to consume an ack
		{
			if (PRODUCTION_PENDING == CurrFragObj.p_flag) // waiting to send more frags
			{
				if (!CurrMessageObjectPkt.DB2) // good status
				{
					// check frag count
					if ((++CurrMessageObjectPkt.DB1 & 0x3f) == CurrFragObj.fragbyte)
					{
						// fill message object with next fragment
						MessageObjectAddOutQ ();
					}
				}
				else // start over
				{
					CurrFragObj.p_flag	 = PRODUCTION_PENDING;
					CurrFragObj.fragbyte = FIRST_FRAG;
				}
			}

			return;
		}

		if (PRODUCTION_PENDING == CurrFragObj.p_flag)
			return; // i'm in the middle of sending out a frag msg

		// set default
		consumption_status = CONSUMPTION_PENDING;

		bytecount = CurrMessageObjectPkt.cMsgLen - 2;

		if (FIRST_FRAG == fragtype)
		{
			CurrFragObj.fragbyte  = FIRST_FRAG;
			CurrFragObj.buffer[0] = CurrMessageObjectPkt.DB0; // msg header
			CurrFragObj.buffindx  = 1;
		}
		else if (FIRST_FRAG == CurrFragObj.fragbyte)
			return;

		// check for a repeated fragment
		if (CurrFragObj.fragbyte == (CurrMessageObjectPkt.DB1 + 1))
		{
			MessageObjectFragAck (ACKFRAG_SUCCESS);
			return;
		}

		// if a frag sequence error occurs, abort the consumption
		if ((CurrFragObj.fragbyte & 0x3f) != (CurrMessageObjectPkt.DB1 & 0x3f))
		{
			CurrFragObj.buffindx = 0;
			CurrFragObj.fragbyte = FIRST_FRAG;
			return;
		}

		// make sure not exceeding size of buffer
		if (CurrFragObj.buffindx + bytecount > FRAG_BUF_SIZE)
		{
			// reset state machine and NACK his message
			CurrFragObj.buffindx = 0;
			CurrFragObj.fragbyte = FIRST_FRAG;
			MessageObjectFragAck (ACKFRAG_TOOMUCHDATA);
			return;
		}

		// set the source, destination pointers for the copy
		sptr = (unsigned char *)&(CurrMessageObjectPkt.DB2);

		// received another valid frag - inc the frag count
		CurrFragObj.fragbyte = ((CurrFragObj.fragbyte + 1) & 0x3f) | MIDDLE_FRAG;

		// is it the final frag
		if (LAST_FRAG == fragtype)
			consumption_status = CONSUMPTION_COMPLETE;

		MessageObjectFragAck (ACKFRAG_SUCCESS);
	}
	else // message not fragmented
	{
		// in case the cnxn had been in the middle of a frag message
		CurrFragObj.p_flag = PRODUCTION_COMPLETE;

		// setup info for copying of data to frag buffer
		CurrFragObj.buffindx = 0;
		sptr				 = (unsigned char *)&(CurrMessageObjectPkt.DB0);
		bytecount			 = CurrMessageObjectPkt.cMsgLen;

		consumption_status = CONSUMPTION_COMPLETE;
	}

	// Restart the inactivity timer if from explicit connection
	if (EXPLICIT_MESSAGE_REQ == CurrMessageObjectPkt.cMSGID)
		ConnectionObjectStartInactivity (EXPLICIT_CNXN);

	// now copy the data from the can packet to the fragmentation buffer
	dptr = (unsigned char *)&(CurrFragObj.buffer[CurrFragObj.buffindx]);
	CurrFragObj.buffindx += bytecount; // increment number of bytes in message
	while (bytecount)
	{
		*dptr++ = *sptr++;
		bytecount--;
	}

	if (CONSUMPTION_COMPLETE == consumption_status)
	{
		// Don't allow normal Explicit messages sevices other than the
		// Allocate/Deallocate ID set under the unconnected messag ID
		if ((CurrMessageObjectPkt.cMSGID == GRP2_UNCONNECTED_REQ)
			&& (CurrFragObj.buffer[INDX_SRVC] != DEALLOC_GRP2_IDSET)
			&& (CurrFragObj.buffer[INDX_SRVC] != ALLOC_GRP2_IDSET))
			MessageObjectFormatErrorMessage (RESOURCE_UNAVAILABLE, INVALID_MESSAGE);
		else
		{
			if (CurrMessageObjectPkt.cMSGID == GRP2_UNCONNECTED_REQ)
			{
				// insert 0 byte for high class byte
				CurrFragObj.buffer[6]		   = CurrFragObj.buffer[5];
				CurrFragObj.buffer[5]		   = CurrFragObj.buffer[4];
				CurrFragObj.buffer[4]		   = CurrFragObj.buffer[3];
				CurrFragObj.buffer[INDX_CL_HI] = 0; // the class code
			}

			// Invoke the object which is to handle this request.
			// The class type is the 2nd and 3rd bytes in the receive buffer

			// preload default message - assuming no errors
			CurrFragObj.numbytes = 2; // default

			if (ENABLE_LINE_CLASS_HI == CurrFragObj.buffer[INDX_CL_HI]
				&& ENABLE_LINE_CLASS_LO == CurrFragObj.buffer[INDX_CL_LO])
				EnableLineObject ();
			else if (!CurrFragObj.buffer[INDX_CL_HI])
			{
				switch (CurrFragObj.buffer[INDX_CL_LO])
				{
				case DEVICE_CLASS:
					IdentityObject ();
					break;

				case CONNECTION_CLASS:
					ConnectionObject ();
					break;

				case MESSAGE_ROUTER_CLASS:
					MessageObject ();
					break;

				case DEVICENET_CLASS:
					COUNT_DEVICENET_OBJ++;
					DeviceNetObject ();
					break;

				case ASSEMBLY_CLASS:
					AppAssemblyObject ();
					break;

				case ACK_HANDLER_CLASS:
					AckHandlerObject ();
					break;

				case NVSOBJECT_CLASS:
					NVSObject ();
					break;

				default:
					AppObject ();
					break;
				}
			}
			else
				AppObject (); // in case any other 16-bit class codes
		}

		CurrFragObj.buffer[INDX_SRVC] |= RESPONSE; // turn on response bit

		// above routines have placed a response message into the buffer
		// now we have to put a CAN packet in the Q
		CurrFragObj.buffindx = 0;
		CurrFragObj.fragbyte = FIRST_FRAG;
		MessageObjectAddOutQ ();
	}
}

void MessageObjectFragAck (unsigned char ackstatus)
{
	/*
	// Make sure buffer is available
	*/
	while (CM_MCR1_Get_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT) == CM_PENDING)
	{
		TimerObjectSvcTimer ();
	}

	/*
	// Setup Message Control Registers
	*/
	CM_MCR1_Set_NEWDAT (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_ENABLED);

	/*
	// Fill in Message Configuration Register with Explicit "C/S" Response and data length
	*/
	CM_Set_UAR1 (XMIT_EXPLICIT_MESSAGE_OBJECT, (EXPLICIT_MESSAGE_RSP << 5));

	CM_MCFG_Set_Direction (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_TRANSMIT);
	CM_MCFG_Set_Length (XMIT_EXPLICIT_MESSAGE_OBJECT, ACKFRAG_RSP_LEN);

	/*
	// Fill in data values into transmit buffer
	//  Byte  0     : message header
	//  Byte  1   : RSP Bit set(Bit 7 ) and error service code
	//  Bytes 2   : Error Code
	//  Bytes 3   : Additional Error Code
	*/
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB0 = CurrMessageObjectPkt.DB0;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB1 = FRAG_ACK | CurrMessageObjectPkt.DB1;
	CANMessageObject[XMIT_EXPLICIT_MESSAGE_OBJECT].DB2 = ackstatus;

	// transmit the message
	CM_MCR1_Set_CPUUPD (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_DISABLED);
	CM_MCR1_Set_TXRQ (XMIT_EXPLICIT_MESSAGE_OBJECT, CM_PENDING);
}

//***********************************************************************
//
// Function                  MessageObjectErrorHandler()
//
// Author                    Brian J. Taylor
// Date                      2/23/94
//
// Functional Description    This function is used to handle the CAN transitions to BUS-OFF state.  When this
//            is detected the device will be taken offline and the LED will indicate a CAN error.
//
// Inputs      CSTAT - CAN status register
//
// Outputs      None
//
// Edit History      [0]   2/23/94   BJT   Created
//         [1]   5/12/94   BJT   Added support to reset CAN chip if DeviceNet BOI attribute is set
//                  to do this.
//         [2]   8/23/94   BJT   BOI attribute is not settable in this version.  #ifdef out code
//                  to save ROM.  Default is HOLD in Reset.
//         [3]   7/11/95   BJT   Only reinit if we haven't gone bus-of ten times.  This would most likely
//                  mean we do have a real problem.
//         [4]   12/30/97BJT   Added to support autobaud
//         [5]   4/27/98   BJT   Fix autobaud bug.  don't increment baud rate if error, since messages
//                  can queue up in buffer.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void MessageObjectErrorHandler ()
{
	if (ABAUD_NOTAUTOB == DeviceNetObjectRAM.bAutoBaud)
	{
		// increment the IDN Object Bus off counter.
		if (0xFF != DeviceNetObjectRAM.busOffCntr)
			DeviceNetObjectRAM.busOffCntr++;

		// If BOI is set to CAN_RESET, then reset CAN chip by calling initIDN.
		// However, don't keep resetting forever (10 times max)
		if ((DeviceNetObjectRAM.boi == RESET_CAN) && (DeviceNetObjectRAM.busOffCntr < 10))
		{
			InitDeviceNet ();
		}
		else
		{
			// Set state to CAN BUS OFF
			UIObjectRAM.cHealthState		= CANBUSOFF;
			DeviceNetObjectRAM.DeviceOnLine = FALSE;
		}
	}
}

//***********************************************************************
//
// Function                  MessageObjectAddInQ()
//
// Author                    CRSchumaker
// Date                      12/14/98
//
// Functional Description    place received packet into Q to be handled by foreground
//
// Inputs       None
//
// Outputs      None
//
// Edit History      [0]   12/14/98   CRS   Created
//
//             Copyright (c) 1998 Allen-Bradley Co.
//***********************************************************************/

unsigned int DROPPED_IN_PACKETS = 0;
unsigned int MESSAGES_RECEIVED	= 0;

void MessageObjectAddInQ (unsigned char msgobj)
{
	MESSAGES_RECEIVED++;
	if (InPktsQd >= MAX_IN_PACKETS) // can only process one message at a time - drop the latest message
	{
		DROPPED_IN_PACKETS++;
		return;
	}

	InBuffer[InPktsQd].cMSGID  = CM_Get_UAR1 (msgobj) >> 5;
	InBuffer[InPktsQd].cMACID  = CANMessageObject[msgobj].DB0 & 0x3f;
	InBuffer[InPktsQd].cMsgLen = CM_MCFG_Get_Length (msgobj);
	xdata_memcpy (&InBuffer[InPktsQd].DB0, &CANMessageObject[msgobj].DB0, 8);

	// increment number of messages queued.
	InPktsQd++;
}

//***********************************************************************
//
// Function                  MessageObjectAddOutQ()
//
// Author                    CRSchumaker
// Date                      12/14/98
//
// Functional Description    place packet into Q to be sent out can port
//
//         - numbytes includes the message header
//         - numbytes does not include the frag type/cnt (this never appears
//             buffer, only in the CAN packets sent on wire).
//
// Edit History      [0]   02/16/99   CRS   Created
//
//             Copyright (c) 1999 Allen-Bradley Co.
//***********************************************************************/

unsigned int DROPPED_OUT_PACKETS = 0;

void MessageObjectAddOutQ (void)
{
	unsigned char *srcp, *dstp, numbyts;
	// messages added at head, removed from tail

	/* eventually the cnxn identifier will have to be "passed along"
	 * the following 2 fields aren't required for output messages
	   MessageObjectOutPktQ.cMSGID  = 0;
	   MessageObjectOutPktQ.cMACID  = 0;
	*/
	// is message fragmented - look at length to determine
	if (CurrFragObj.numbytes > 8)
	{
		// message requires fragmentation

		MessageObjectOutPktQ.DB0 = CurrFragObj.buffer[0] | FRAG_BIT;

		numbyts = 6; // default - unless sending last frag
		if (0 == CurrFragObj.buffindx)
		{
			// must send first fragment

			MessageObjectOutPktQ.DB1 = FIRST_FRAG; // first frag and count=0
			CurrFragObj.buffindx++;				   // first byte is msghdr

			CurrFragObj.p_flag = PRODUCTION_PENDING;
		}
		else if ((CurrFragObj.numbytes - CurrFragObj.buffindx) > 6)
		{
			// must send middle fragment

			MessageObjectOutPktQ.DB1 = MIDDLE_FRAG | CurrFragObj.fragbyte;
		}
		else
		{
			// must send last fragment

			MessageObjectOutPktQ.DB1 = LAST_FRAG | CurrFragObj.fragbyte;
			numbyts					 = CurrFragObj.numbytes - CurrFragObj.buffindx;

			CurrFragObj.p_flag = PRODUCTION_COMPLETE;
		}

		srcp = (unsigned char *)&(CurrFragObj.buffer[CurrFragObj.buffindx]);
		dstp = (unsigned char *)&(MessageObjectOutPktQ.DB2);
		CurrFragObj.fragbyte++; // no chance of rolling over

		if ((CurrFragObj.numbytes - CurrFragObj.buffindx) <= 6)
		{
			// sending last frag - reset stuff
			CurrFragObj.numbytes = 0;
		}
		else
		{
			// not last frag - adjust stuff for next time
			CurrFragObj.buffindx += 6;
		}

		// length of message to go on CAN wire
		MessageObjectOutPktQ.cMsgLen = numbyts + 2;
	}
	else // message not fragmented
	{
		numbyts = MessageObjectOutPktQ.cMsgLen = CurrFragObj.numbytes;

		srcp = (unsigned char *)&(CurrFragObj.buffer[0]);
		dstp = (unsigned char *)&(MessageObjectOutPktQ.DB0);

		// force off frag bit before copying
		(*srcp) &= (~FRAG_BIT);
	}

	// copy the data bytes
	while (numbyts--)
		*dstp++ = *srcp++;

	// increment number of messages queued.
	OutPktsQd++;

	if (OutPktsQd > 1)
		DROPPED_OUT_PACKETS++;
}

//***********************************************************************
//
// Function                  MessageObjectISR()
//
// Author                    Pat Maloney
// Date                      10/26/93
//
// Functional Description    This function processes a MCAN CIRQ Interrupt. When this interrupt service routine is
// informed
//            that a message has been received, (via a Rx interrupt) this routine is invoked to determine
//            whether or not the message should be processed further and, if it should, to interface to the
//            appropriate handler routines.
//
//         NOTE: Since this device is a group 2 only device, only group 2 messages will make it past the
//               MASK & MATCH screener causing an MCAN interrupt.
//
// Inputs      RBI - Receive Buffer Identifier Register
//         RRTDL - RTR R-bit and Data Length Register
//         DeviceNetObjectRAM - IDN Object
//         DNG_cMasterMACID - Master MAC ID
//
// Outputs      CCOM - Set CAN command to Release Recieve Buffer
//
// Edit History      [1]   3/25/94   BJT   Added to IDN 9000 Code.
//         [2]   5/31/94   BJT   Removed call to DupMACIDFault and just set LED like the function did.
//         [3]   7/1/94   BJT   For Poll and Strobe commands set the transmit registers up and send response.
//                  (This was in the sensor object functions and was moved out to this function)
//         [4]   7/6/94   BJT   Set TBI and TRTDL for strobe and Poll messages responses. Also, goto DUP MAC Fault
//                  state if ever get a DUP MAC response Message.
//         [5]   7/12/94   BJT   Moved logic in MessageObjectRouter into this function to save space.
//         [6]   7/21/94   BJT   Must release rcv buffer and exit ISR if a dup MAC failure is detected. Clear COP timer.
//         [7]   8/3/94   BJT   Move checks for strobe message as first check to speed up strobe response time.
//         [8]   8/9/94   BJT   Added DeviceLink Support. DevicelInk doesn't use sensor Object.
//         [9]   11/17/94BJT   Compliancy Test problem. Need to respond with specific error message for fragmented
//                  message request. (DN spec Vol 1 4-44, Rel 1.1)
//         [10]   1/11/95   BJT   Set TBI and TRTDL up from data not from producer connection ID attribute since this
//                  only has identifier.
//         [11]   6/2/95   BJT   Added Support for DeviceNet Configurator Tool. If DUP MAC detected increment our
//                  address to find a valid one..
//         [12]   6/22/95   BJT   Added support for Autobaud.
//         [13]   6/16/98   BJT   Fixed bug that didn't catch packet in buffer if received while processing another.
//         [14]   6/29/98   BJT   Changed in accordance with DAvE.
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

/*
void Tmp(void);
*/
extern unsigned char consume_data_size;

typedef struct
{
	unsigned int count_status;
	unsigned int count_poll;
	unsigned int count_rx_unconnected;
	unsigned int count_rx_cos_ack;
	unsigned int count_rx_explicit;
	unsigned int count_rx_dup_mac;
	unsigned int count_rx_comm_filtered;
	unsigned int count_tx_cos;
	unsigned int count_tx_poll;
	unsigned int count_default;
} CAN_ISR_STATS_T;
CAN_ISR_STATS_T CS; // CAN stats

void MessageObjectHandleStatus (void);
void MessageObjectHandleRxPoll (void);
void MessageObjectHandleRxUnconnected (void);
void MessageObjectHandleRxCosAck (void);
void MessageObjectHandleRxExplicitDupMac (char ubIntID);
void MessageObjectHandleRxCommFiltered (char ubIntID);
void MessageObjectHandleTxCos (void);
void MessageObjectHandleTxPoll (void);

unsigned long DROPPED_IN_FRAMES = 0;
unsigned long CAN_ERRORS		= 0;

//***********************************************************************
//
// Function                  MessageObject()
//
// Author                    Brian J. Taylor
// Date                      5/27/1994
//
// Functional Description    This function is used to handle Explicit Message requests directed toward the
//            Message Router Object and routes them to the appropriate Message Router Object
//            service function.
//
// Inputs      RCV_BUF - Receive Buffer
//
// Outputs      DNG_cStatus - Set if error detected
//
// Edit History      [0]   5/27/94   BJT   Created
//
//             Copyright (c) 1994 Allen-Bradley Co.
//***********************************************************************/

void MessageObject ()
{
	MessageObjectFormatErrorMessage (SERVICE_NOT_SUPP, ADD_CODE_NOT_SPECIFIED);
}

void MessageObjectHandleStatus (void)
{
	char error_code = 0;

	/*
	// if BUS off
	*/
	if (CH_Is_Busoff ()) // if (ubStatus_size & BOFF)
	{
		MessageObjectErrorHandler ();
	}

#if FALSE // CRS these were ifdef's before - but false IS defined
	/*
	// if TXOK then reset TXOK
	// Indicates that a message has been transmitted successfully
	// (error free and acknowledged by at least one other node).
	*/
	if (ubStatus_size & 0x08)
	{
		ubStatus_size &= 0xf7;
		SR = ubStatus_size;
	}
#endif

	/*
	// if RXOK then reset RXOK
	// Indicates that a message has been received successfully.
	*/
	if ((ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud) && CP_Get_RXOK ()) //(ubStatus_size & 0x10))
	{
		CP_Set_RXOK (0);
		DeviceNetObjectRAM.bAutoBaud = ABAUD_NOTAUTOB;
		InitDeviceNet ();
		return; // let's stop processing the message rcvd
	}

	/*
	// if LEC
	*/
	// call into our can hardware status registers
	error_code = CH_Last_Bus_Error_Code ();
	if (error_code)
	{
		CAN_ERRORS++;
		switch (error_code)
		{
		/*
		// Stuff Error
		// More than 5 equal bits in a sequence have occurred in a part of a received
		// message where this is not allowed.
		*/
		case 1:
			break;

		/*
		// Form Error
		// A fixed format part of a received frame has the wrong format.
		*/
		case 2:
			break;

		/*
		// Ack Error
		// The message this CAN controller transmitted was not acknowledged by another node.
		*/
		case 3:
			break;

		/*
		// Bit1 Error
		// exeption of the arbitration field), the device wanted to send a recessive
		// level ("1"), but the monitored bus value was dominant.
		*/
		case 4:
			break;

		/*
		// CRC Error
		// The CRC check sum was incorrect in the message received.
		*/
		case 6:
			break;

		/*
		// Bit0 Error
		// During the transmission of a message (or acknow ledge bit, active error flag,
		// or overload flag), the device wanted to send a dominant level ("0"), but the
		// monitored bus value was recessive. During busoff recovery this staus is set
		// each time a sequence of 11 recessive bits has been monitored. This enables
		// the CPU to monitor the proceeding of the busoff recovery sequence (indicating
		// the bus is not stuck at dominant or continously disturbed).
		*/
		case 5:
		{
			/*
			// if Busoff status, handle bit dominant error
			*/
			if (CH_Is_Busoff ())
			{
				MessageObjectErrorHandler ();
			}
		}
		break;

		} // end switch ( ubStatus_size & 0x07 )
	}	  // end if ( ubStatus_size & 0x07 )
}

void Stub_Fill(uchar * str, uchar msg_len)
{
	static uchar trans_id = 1;
	char buf[] = "123456789 ABCDEFGHIJ VVVVVVVVVVV abcdefghijkl abcdefghijkl"; // temp data to be sent on serial

	if (msg_len == 0xFF)
	{
		str[0] = 0; // RX byte for handshaking
		str[1] = 0; //
		str[2] = trans_id++; // to change transaction id
		str[3] = 01; //device ID
		str[4] = 04; // function name for modbus
		str[5] = 02; //Address LO
		str[6] = 00; // Address Hi
		str[7] = 03; //Length LO
		str[8] = 00; //Length HI
		if (trans_id % 20 == 0)
			trans_id = 1;
	}
	else
	{
		str[0] = 0; // RX byte for handshaking
		str[1] = trans_id++; // to change transaction id
		str[2] = 0; //reserve byte
		str[3] = msg_len; // length for short string only single byte
		if (trans_id % 20 == 0)
			trans_id = 1;
		xdata_memcpy (&(str[4]), &buf[trans_id], msg_len);
	}
}

void MessageObjectHandleRxPoll (void)
{
#ifdef FCL
#ifndef SIM_CONSUME
	uchar frag_cnt;
#endif
	uchar consume_poll_data;
#endif // FCL

#ifndef SIM_CONSUME
	unsigned char ubStatus_size; // used for 2 purposes
#endif
	BOOL		  produce_data;

#ifdef SIM_CONSUME    // this is the Consumer Stub

#define ARRAY_MSG 			1
#define SHORT_STRING_MSG 	2
#define STRING_MSG 			3
#define MODBUS_MSG			4

#ifdef SIM_MODBUS
	uchar msg_type = MODBUS_MSG;
#else
	uchar msg_type = STRING_MSG;
#endif
    if(TRUE)  // Has CM_PEENDING
	{
		if(TRUE) // Has CNXN_ESTABLISHED
		{
			switch (msg_type)
			{
				case ARRAY_MSG:
					ConsFragPtr = 8;
					break;
				case SHORT_STRING_MSG:
					ConsFragPtr = 12;
					break;
				case STRING_MSG:
					ConsFragPtr = 24;   // ASSY_CINST size in bytes.
					break;
				case MODBUS_MSG:
					ConsFragPtr = 0xFF;
					break;
				default:
					break;
			}
			Stub_Fill(&P_InMsgBuffer[0], ConsFragPtr);
			consume_poll_data = TRUE;
//			if(FALSE) // GMMRAM.Active
//			{
#else
	// is NEWDAT set
	// if ((CANMessageObject[RCV_POLL_MESSAGE_OBJECT].MCR1 & MASK_NEWDAT) == IS_SET_NEWDAT)
	if (CM_MCR1_Get_NEWDAT (RCV_POLL_MESSAGE_OBJECT) == CM_PENDING)
	{
		if (DeviceNetObjectRAM.DeviceOnLine && CNXN_ESTABLISHED == ConnectionObjectRAM[POLL_CNXN].state)
		{
			// use this variable as a temp
			ubStatus_size = CM_MCFG_Get_Length (RCV_POLL_MESSAGE_OBJECT);

#ifdef FCL
#ifdef GMM
			if (GMMRAM.active)
			{
				// always expecting 8 bytes of I/O data
				if (!ubStatus_size)
				{
					// we're in idle mode

					consume_poll_data = TRUE;
					ConsFragPtr		  = 0;
					TURN_POLL_RUN_BIT_OFF; // idle mode
					RESET_CTRL_BYTE_SAVE;  // set to invalid value

					// Restart the inactivity timer for the connection
					ConnectionObjectStartInactivity (POLL_CNXN);
					// always ack the COS msg
					MessageObjectSendCOSAck (); // 0-length response msg
				}
				else
				{
					TURN_POLL_RUN_BIT_ON; // run mode
					if (8 == ubStatus_size)
					{

						//-----------------------------------------------------------
						// Logic for processing an 8-byte message when in GMMode
						//-----------------------------------------------------------

						consume_poll_data = FALSE; // preload here
						if (CntrlByteSave != *CtlBytPtr)
						{
							CntrlByteSave = *CtlBytPtr;

							switch (GMM_Frag_State)
							{
							case GMMFS_IDLE: // not doing anything now
								if ((XMT_1ST_BYTE == CntrlByteSave) && (CONST_ZERO == *StatBytPtr))
								{
									// host wishes to send output data

									// get anticipated length if not array mode
									if (ARRAY == type)
										Exp_Len = TxStrLen;
									else
										Exp_Len = *LenPtr;

									if (!Exp_Len)
										Exp_Len = TxStrLen;

									GMM_Frag_State = GMMFS_OUT;
									ConsFragCnt	   = 2; // next frag should be 2

									// copy string data to buffer

									// not sure what to do with the first 3 bytes - clear for now
									P_InMsgBuffer[0] = 0;
									P_InMsgBuffer[1] = 0;

									xdata_memcpy (&(P_InMsgBuffer[2]), &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB2),
												  6);

									ConsFragPtr = 8;

									// are we already done
									if (Exp_Len <= 4)
									{
										// we're already done
										consume_poll_data = TRUE;

										// fill output bytes sent in COS msg
										C_OutMsgBuffer[OutCtlIndx] = 0x01; // idle and 1st byte
										GMM_Frag_State			   = GMMFS_DONE;
									}
									else
									{
										// not already done

										// fill output bytes sent in COS msg
										C_OutMsgBuffer[OutCtlIndx] = XMT_1ST_BYTE;
									}

									GMMFragError				= 0; // clear the fragerror bit
									C_OutMsgBuffer[OutStatIndx] = local_status_byte ();
								}
								else if ((REC_1ST_BYTE == CntrlByteSave)
										 && (RRecStatus & SB_NEW_IN_DATA_AVL)) // data is available
								{
									// host is requesting we send him data rcvd serially

									GMM_Frag_State = GMMFS_IN;

									// get the length of the serial data
									Exp_Len = size_of_mainloopassydata - 4;

									// is it too large
									if (Exp_Len > RRecRxStrMaxLen)
									{
										// transmit the max # of chars and set overflow bit?????
										Exp_Len = RRecRxStrMaxLen;
									}

									GMMFragError = 0; // clear the fragerror bit

									C_OutMsgBuffer[OutCtlIndx]	= (CMD_RCV | 0x1); // 0x11
									C_OutMsgBuffer[OutStatIndx] = local_status_byte ();

									// load the length (number of serial data bytes)
									if (ARRAY == RRecRxStrType)
									{
										C_OutMsgBuffer[2] = 0x00;
										C_OutMsgBuffer[3] = 0x00;
									}
									else
									{
										C_OutMsgBuffer[OutLenIndx]	= Exp_Len;
										C_OutMsgBuffer[OutLen1Indx] = 0x00;
									}

									// decrement Exp_Len
									Exp_Len = (Exp_Len >= 4) ? (Exp_Len - 4) : 0;

									ConsFragCnt = 1;

									// now copy the data bytes (4) into the buffer
									xdata_memcpy (&(C_OutMsgBuffer[4]), &(mainloopassydata[4]), 4);

									ConsFragPtr = 8; // points into mainloopassydata
								}
								break;

							case GMMFS_DONE: // done w/ transaction, waiting for idle
								GMM_Frag_State = GMMFS_IDLE;
								// fill output bytes sent in COS msg
								C_OutMsgBuffer[OutCtlIndx] = CMD_IDLE;

								if (CMD_IDLE == (CMD_MASK & CntrlByteSave))
									GMMFragError = 0;
								else // incorrect msg -> frag error
									GMMFragError = 1;
								break;

							case GMMFS_OUT: // getting output data from host
								if (CMD_SND == (CMD_MASK & CntrlByteSave))
								{
									if (ConsFragCnt == (FRG_MASK & CntrlByteSave))
									{
										// correct frag number

										// copy string data to buffer

										P_InMsgBuffer[ConsFragPtr++] = *StatBytPtr;
										xdata_memcpy (&(P_InMsgBuffer[ConsFragPtr]),
													  &(CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB2), 6);
										ConsFragPtr += 6;

										// have we reached the end of the message
										if (ConsFragPtr >= (Exp_Len + 4)) // 4 bytes of 'header'
										{
											consume_poll_data = TRUE;
											ConsFragPtr		  = Exp_Len + 4; // 4 bytes of 'header'

											// fill output bytes sent in COS msg
											C_OutMsgBuffer[OutCtlIndx] = (CMD_IDLE | ConsFragCnt);

											GMM_Frag_State = GMMFS_DONE;
										}
										else
										{
											// not done

											// fill output bytes sent in COS msg
											C_OutMsgBuffer[OutCtlIndx]	= (CMD_SND | ConsFragCnt);
											C_OutMsgBuffer[OutStatIndx] = local_status_byte ();
										}

										ConsFragCnt++;
									}
									else // wrong frag number - let's reset everything
										idle_w_frag_error ();
								}
								break;

							case GMMFS_IN: // sending input data to host
								if (CMD_IDLE == (CMD_MASK & CntrlByteSave))
								{
									GMM_Frag_State = GMMFS_IDLE;

									// fill output bytes sent in COS msg
									C_OutMsgBuffer[OutCtlIndx] = CMD_IDLE;

									if ((!Exp_Len) && (ConsFragCnt == (FRG_MASK & CntrlByteSave)))
									{
										RRecStatus &= ~SB_NEW_IN_DATA_AVL; // clear the bit
										GMMFragError = 0;
									}
									else
										GMMFragError = 1;
								}
								else if (CMD_RCV == (CMD_MASK & CntrlByteSave))
								{
									if (Exp_Len && (ConsFragCnt == (FRG_MASK & CntrlByteSave)))
									{
										// correct fragment
										// Now have to fill the output buffer
										C_OutMsgBuffer[OutStatIndx] = mainloopassydata[ConsFragPtr++];
										xdata_memcpy (&(C_OutMsgBuffer[2]), &(mainloopassydata[ConsFragPtr]), 6);

										if (Exp_Len >= 7) // how many data bytes left
										{
											Exp_Len -= 7;
											ConsFragPtr += 6;
										}
										else
											Exp_Len = 0; // this is our last frag

										ConsFragCnt++;

										C_OutMsgBuffer[OutCtlIndx] = (CMD_RCV | ConsFragCnt);
									}
									else // wrong fragment
										idle_w_frag_error ();
								}
								else // must be transmit command (CMD_SND) - goto idle mode
									idle_w_frag_error ();
								break;

							} // from the 'switch'

							// trigger COS production with new data
							AppObjectCOS_Bit = 1;
						}

						if ((GMMFS_IDLE == GMM_Frag_State) || (GMMFS_DONE == GMM_Frag_State))
							C_OutMsgBuffer[OutStatIndx] = local_status_byte ();

						// set profibus status byte value
						C_OutMsgBuffer[8] = GMMRAM.IOStatus;

						//-----------------------------------------------------------
						// end of Logic for processing an 8-byte msg when in GMMode
						//-----------------------------------------------------------

						// Restart the inactivity timer for the connection
						ConnectionObjectStartInactivity (POLL_CNXN);
						// always ack the COS msg
						MessageObjectSendCOSAck (); // 0-length response msg
					}
					else // invalid length
					{
						// reset frag stuff

						consume_poll_data = FALSE;
					}
				}
			}
			else
#endif	// GMM
				if (IOCnxnSize[CSI_P_CONS] > 8) // fragmented reception?
			{
				ubStatus_size--;		   // decrement - first byte is frag byte, not data
				consume_poll_data = FALSE; // preload with default value
				// let's get the frag count
				frag_cnt = 0x3F & CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0;
				// let's check the fragtype
				switch (FRAG_TYPE_MASK & CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0)
				{
				case FIRST_FRAG:
					if (!frag_cnt) // fragcount = 0 (as it normally should be)
					{
						if (7 != ubStatus_size)
						{
							// for a first frag, we've got to have 8 bytes
							ConsFragPtr = 0;
						}
						else // all expected bytes rcvd
						{
							ConsFragCnt = 1;

							// copy the 7 data bytes to buffer
							// Begin Modified ER to speed up fragment consumption
							/*
							for(ConsFragPtr=0;ConsFragPtr < 7;)
							   P_InMsgBuffer[ConsFragPtr++] = *(moptr++);
							*/
							xdata_memcpy (P_InMsgBuffer, &CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB1,
										  ConsFragPtr = 7);
							// End Modified ER
						}
					}
					else if (0x3f == frag_cnt) // first and last frag
					{
						// copy the data bytes to buffer
						// Begin Modified ER to speed up fragment consumption
						/*
					 for(ConsFragPtr=0;ConsFragPtr < ubStatus_size;)
						P_InMsgBuffer[ConsFragPtr++] = *(moptr++);
						*/
						xdata_memcpy (P_InMsgBuffer, &CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB1,
									  ConsFragPtr = ubStatus_size);
						// end modified ER
						consume_poll_data = TRUE;
					}
					break;
				case LAST_FRAG:
					consume_poll_data = TRUE;
					// 'break' intentionally missing
				case MIDDLE_FRAG:
					// first let's check if the frag count is right
					if (frag_cnt == ConsFragCnt++)
					{
						// check if too many data bytes
						if ((ConsFragPtr + ubStatus_size) > IOCnxnSize[CSI_P_CONS])
						{
							// too many data bytes - start all over
							ConsFragCnt		  = 0;
							consume_poll_data = FALSE;
						}
						else
						{
							// let's copy the data bytes
							// Modified ER
							/*
							for(i=0;i<ubStatus_size;i++)
							   P_InMsgBuffer[ConsFragPtr++] = *moptr++;
							*/
							xdata_memcpy (&P_InMsgBuffer[ConsFragPtr], &CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB1,
										  ubStatus_size);
							ConsFragPtr += ubStatus_size;
							// end modified ER
						}
					}
					else // wrong frag count - start all over
					{
						ConsFragCnt		  = 0;
						consume_poll_data = FALSE;
					}
					break;
				default:
					// invalid frag type - start all over
					ConsFragCnt = 0;
					break;
				}
			}
			else // cnxn size less than 8 bytes - not fragmented
			{
				// modified ER to speed up consumtion

				/*
				moptr = &CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0;
				// let's copy the data bytes

				for(ConsFragPtr=0;ConsFragPtr<ubStatus_size;)
				   P_InMsgBuffer[ConsFragPtr++] = *(moptr++);

			   */
				xdata_memcpy (P_InMsgBuffer, &CANMessageObject[RCV_POLL_MESSAGE_OBJECT].DB0,
							  ConsFragPtr = ubStatus_size);

				consume_poll_data = TRUE; // preload with default value

				// end modified ER
			}
#endif // FCL
#endif //SimConsume

#ifdef FCL
#ifdef GMM
			// is connection idle?
			if (GMMRAM.active)
			{
				if (consume_poll_data)
				{
					// Call appropriate object to consume poll data.
					consume_data_size = ConsFragPtr;
					produce_data	  = AppObjectPollConsume (/*ConsFragPtr*/);

					ConsFragCnt = 0; // reset for next time
					ConsFragPtr = 0; // reset for next time
				}
			}
			else
#endif // GMM
				if (consume_poll_data)
			{
				// is connection idle?
				if (!ConsFragPtr)
					TURN_POLL_RUN_BIT_OFF; // idle mode
				else
					TURN_POLL_RUN_BIT_ON; // run mode

				// Restart the inactivity timer for the connection selected above
				ConnectionObjectStartInactivity (POLL_CNXN);
				// Call appropriate object to consume poll data.
				consume_data_size = ConsFragPtr;
				produce_data	  = AppObjectPollConsume (/*ConsFragPtr*/);

				ConsFragCnt = 0; // reset for next time

				if (DeviceNetObjectRAM.AllocedCnxns & POLLED)
				{
					if (produce_data)
						MessageObjectSendPollMessage ();
				}
				else if (!(DeviceNetObjectRAM.AllocedCnxns & ACK_SUPPRESSION))
					MessageObjectSendCOSAck (); // 0-length response msg
			}
#else  // FCL

			// rcvd IO msg should be less than or equal to
			// expected size - otherwise ignore
			if (ubStatus_size <= IOCnxnSize[CSI_P_CONS])
			{
				// is connection idle?
				if (IOCnxnSize[CSI_P_CONS] && !ubStatus_size)
					TURN_POLL_RUN_BIT_OFF; // idle mode
				else
					TURN_POLL_RUN_BIT_ON; // run mode

				// Restart the inactivity timer for the connection selected above
				ConnectionObjectStartInactivity (POLL_CNXN);

				// Call appropriate object to consume poll data.
				produce_data = AppObjectPollConsume (/*ubStatus_size*/);

				if (DeviceNetObjectRAM.AllocedCnxns & POLLED)
				{
					if (produce_data)
						MessageObjectSendPollMessage ();
				}
				else if (!(DeviceNetObjectRAM.AllocedCnxns & ACK_SUPPRESSION))
					MessageObjectSendCOSAck (); // 0-length response msg
			}
#endif // FCL
		}

		/*
		// Release RcvBuffer
		*/
		CM_MCR0_Clear_INTPND (RCV_POLL_MESSAGE_OBJECT);

		CM_MCR1_Set_NEWDAT (RCV_POLL_MESSAGE_OBJECT, CM_NOT_PENDING);
		CM_MCR1_Set_TXRQ (RCV_POLL_MESSAGE_OBJECT, CM_NOT_PENDING);
	}
}

void MessageObjectHandleRxUnconnected (void)
{
	MessageObjectAddInQ (RCV_UNCNNCTD_MESSAGE_OBJECT); // JSW remove else to prevent double dropped messages

	/*
	// Release RcvBuffer
	*/
	CM_MCR0_Clear_INTPND (RCV_UNCNNCTD_MESSAGE_OBJECT);
	CM_MCR1_Set_NEWDAT (RCV_UNCNNCTD_MESSAGE_OBJECT, CM_NOT_PENDING);
}

void MessageObjectHandleRxCosAck (void)
{
	// setup the correct message object to use

	// if NEWDAT set
	if (CM_MCR1_Get_NEWDAT (RCV_COS_ACK_MESSAGE_OBJECT) == CM_PENDING)
	{
	ANOTHER_CA_MSG:
		CM_MCR1_Set_NEWDAT (RCV_COS_ACK_MESSAGE_OBJECT, CM_NOT_PENDING);

		if (CNXN_ESTABLISHED == ConnectionObjectRAM[COS_CNXN].state)
		{
			// rcvd cos msg should always be 0 - otherwise ignore
			if (CM_MCFG_Get_Length (RCV_COS_ACK_MESSAGE_OBJECT) == 0)
			{
				// CRS - 8-01
				// Setting a flag that will be used in foreground
				// We used to modify things that were also modified in the
				//  foreground.  This could cause unexpected behavior.
				COSACKRcvd = TRUE;
			}
		}

		// recheck newdat
		if (CM_MCR1_Get_NEWDAT (RCV_COS_ACK_MESSAGE_OBJECT) == CM_PENDING)
			goto ANOTHER_CA_MSG;

		/*
		// Release RcvBuffer
		*/
		CM_MCR0_Clear_INTPND (RCV_COS_ACK_MESSAGE_OBJECT);
	}
}

void MessageObjectHandleRxExplicitDupMac (char object)
{
	/*
	// setup the correct message object to use
	*/
	MessageObjectRAM.cCurrentMessage = object;

	/*
	// if NEWDAT set
	*/
	if (CM_MCR1_Get_NEWDAT (MessageObjectRAM.cCurrentMessage) == CM_PENDING)
	{
	ANOTHER_MSG:
		CM_MCR1_Set_NEWDAT (MessageObjectRAM.cCurrentMessage, CM_NOT_PENDING);
		MessageObjectAddInQ (MessageObjectRAM.cCurrentMessage);

		// recheck newdat
		if (CM_MCR1_Get_NEWDAT (MessageObjectRAM.cCurrentMessage) == CM_PENDING)
			goto ANOTHER_MSG;

		CM_MCR0_Clear_INTPND (MessageObjectRAM.cCurrentMessage);
	}
}

void MessageObjectHandleRxCommFiltered (char object)
{
	if (DUPLICATEMACFAIL == UIObjectRAM.cHealthState)
	{
		MessageObjectRAM.cCurrentMessage = object;
		MessageObjectProcessCommFltdMsg ();
	}
	/*
	// Release RcvBuffer
	*/
	CM_MCR0_Clear_INTPND (RCV_COMM_FLTD_MSG_OBJ);
	CM_MCR1_Set_NEWDAT (RCV_COMM_FLTD_MSG_OBJ, CM_NOT_PENDING);
}

void MessageObjectHandleTxCos (void)
{
#ifdef FPL
	uchar frag_ptr;
#endif // FPL

	if (C_FragCount)
	{
		frag_ptr = C_FragCount * 7;
		xdata_memcpy (&CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB1, &C_OutMsgBuffer[frag_ptr], 7);

		//  for variable size poll messages
		if ((frag_ptr += 7) >= C_OutMsgBufferSize) // last frag
		{
			CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB0 = (LAST_FRAG | C_FragCount);

			CM_MCFG_Set_Direction (XMIT_COS_MESSAGE_OBJECT, CM_TRANSMIT);
			CM_MCFG_Set_Length (
				XMIT_COS_MESSAGE_OBJECT,
				(C_OutMsgBufferSize - frag_ptr + 8) & 0xF // JSW TODO: Review //TODO: value > 8 almost certainly invalid
			);

			// signal that we're done
			C_FragCount = 0;
		}
		else // a middle frag
		{
			// num data bytes transmitted should still be 8
			CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB0
				= (MIDDLE_FRAG | ((CANMessageObject[XMIT_COS_MESSAGE_OBJECT].DB0 + 1) & 0x3f));
			C_FragCount++;
		}

		// ok - let's fire off the message now
		CM_MCR1_Set_NEWDAT (XMIT_COS_MESSAGE_OBJECT, CM_PENDING);
		CM_MCR1_Set_CPUUPD (XMIT_COS_MESSAGE_OBJECT, CM_DISABLED);
		CM_MCR1_Set_TXRQ (XMIT_COS_MESSAGE_OBJECT, CM_PENDING);
	}
	else
	{
		AckHandlerObjectRAM.AckTimer[0] = AckHandlerObjectRAM.AckTime[0] >> 3;
		AckHandlerObjectRAM.AckTimer[1] = (AckHandlerObjectRAM.AckTime[0] << 5) + (AckHandlerObjectRAM.AckTime[1] >> 3);
	}

	CM_MCR0_Clear_INTPND (XMIT_COS_MESSAGE_OBJECT);
}

void MessageObjectHandleTxPoll (void)
{
#ifdef FPL
	uchar frag_ptr;
#endif // FPL

	// JSW: wait for availability
	while (CM_MCR1_Get_TXRQ (XMIT_POLL_MESSAGE_OBJECT) == CM_PENDING)
	{
		Timer_Thread_Yield ();
	}

	if (P_FragCount)
	{
		frag_ptr = P_FragCount * 7;
		// Modified ER to speed up ISR
		/*
		//oh my, this is horrific! the will be very large
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB1 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB2 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB3 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB4 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB5 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB6 = P_OutMsgBuffer[frag_ptr++];
		CANMessageObject[ XMIT_POLL_MESSAGE_OBJECT ].DB7 = P_OutMsgBuffer[frag_ptr++];
		*/

		xdata_memcpy (&CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB1, &P_OutMsgBuffer[frag_ptr], 7);

		// end modified er
		// Ed Riede Western Reserve Controls 2001-11-8
		//(c) Western Reserve Controls
		//  added to allow variable size polls.
		// if(frag_ptr >= IOCnxnSize[CSI_P_PROD]) // last frag
		// if(frag_ptr >= P_OutMsgBufferSize)
		if ((frag_ptr += 7) >= P_OutMsgBufferSize)
		{
			// Wrong yet again!.... the frag count is only 3f long, it must roll over!
			CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0
				= (0x3f & (CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0 + 1)) | LAST_FRAG;

			CM_MCFG_Set_Direction (XMIT_POLL_MESSAGE_OBJECT, CM_TRANSMIT);
			CM_MCFG_Set_Length (XMIT_POLL_MESSAGE_OBJECT,
								(P_OutMsgBufferSize - frag_ptr + 8)
									& 0xF // JSW TODO: review // TODO: value > 8 almost certainly invalid
			);

			// signal that we're done
			P_FragCount = 0;
		}
		else // a middle frag
		{
			// num data bytes transmitted should still be 8
			CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0
				= (0x3f & (CANMessageObject[XMIT_POLL_MESSAGE_OBJECT].DB0 + 1)) | MIDDLE_FRAG;
			P_FragCount++;
		}

		// ok - let's fire off the message now
		CM_MCR1_Set_NEWDAT (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
		CM_MCR1_Set_CPUUPD (XMIT_POLL_MESSAGE_OBJECT, CM_DISABLED);
		CM_MCR1_Set_TXRQ (XMIT_POLL_MESSAGE_OBJECT, CM_PENDING);
	}
	CM_MCR0_Clear_INTPND (XMIT_POLL_MESSAGE_OBJECT);
}
