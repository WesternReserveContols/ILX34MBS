/*
 *
 *SRecprot.c
 *	Record Protection algoritems for the 1782-JDC
 *
 */
#include <stddef.h>
#include <stdbool.h>
#include "memopt.h"
#include "_ILX34-MBS/ee_adr.h"
#include "msg.h"
#include "dn_def.h"
#include "dn_cnobj.h"
#include "xdatacpy.h"
#ifdef GMM
#include "app_objs.h"
#endif // GMM

// manifest constants for delimiter mode operatons
#define NO_DELIMITER	  0
#define EXCLUDE_DELIMITER 1
#define INCLUDE_DELIMITER 2

#define TX_MAX_STRING_LEN BYTES_OF_SER_DATA
#define ARRAY			  0
#undef STRING
#undef SHORT_STRING
#define STRING		 2
#define SHORT_STRING 1
unsigned char txrecnum;
unsigned char txrecstr[BYTES_OF_SER_DATA + 2] = { 0, 0 }; // string
unsigned char type							  = SHORT_STRING;
unsigned char SRecMode						  = 0; // rec prot=0  disable rec prot =1
unsigned char SRecSwap						  = 0; // 0 = byte aligned 1= wordaligned 3=dword aligned
unsigned char TxStrLen						  = 20;
unsigned char TxStrDelim					  = 0x0d;
unsigned char TxStrDelimMode				  = 2;
unsigned char TxSts							  = 0;
unsigned char paramtxlen					  = 0;
bool		  UseTxDelimAlgor				  = 0;
bool		  ParamUseTxDelimAlgor			  = 0;

extern bool isparityerror (unsigned char portnum);
extern void clearparityerror (unsigned char portnum);

void SRecProtGetTxRec (MSG *msg)
{
	msg->buf[0] = txrecnum;
	msg->buflen = 1;
}

void SRecProtGetTxStr (MSG *msg)
{
	msg->buf	= txrecstr;
	msg->buflen = txrecstr[0];
	txrecstr[1] = 0; // for short string overwrites this data
	if (type == SHORT_STRING || msg->class == 0xf)
	{
		msg->buf++;
		txrecstr[1] = msg->buflen;
		msg->buflen++;
	}
	else if (type == STRING)
	{
		msg->buflen += 2;
	}
	else if (type == ARRAY)
	{
		msg->buf += 2;
	}
	// else critical error!!
}
// parameter object only.
void SRecProtGetTxDataLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = paramtxlen;
	msg->buflen = 1;
}

void SRecProtSetTxDataLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	// if the length is 0, set the use parameter delimeter algorithm variable to 1
	// else set it to 0
	else if ((msg->buf[0] > TX_MAX_STRING_LEN))
	{
		g_status = INVALID_ATTRIBUTE_DATA;
		return;
	}

	ParamUseTxDelimAlgor = (!(paramtxlen = msg->buf[0]));
	msg->buflen			 = 1;
}
// end parameter object only
void SRecProtGetTxStrType (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = type;
	msg->buflen = 1;
}

void SRecProtGetSerHwInst (MSG *msg)
{
	msg->buf[0] = msg->buflen = 1;
}

void SRecProtGetMode (MSG *msg)
{
	msg->buf[0] = SRecMode;
	msg->buflen = 1;
}

void SRecProtGetSwap (MSG *msg)
{
	msg->buf[0] = SRecSwap;
	msg->buflen = 1;
}

// this routine is called when you want to send the string.  We hold off the parse
// until here, and then we only have to parse once per send, instead of once per set
// I made this routine in the small memory model because of the disable interrupts
// i want this routine to go FAST.
extern void my_putc (unsigned char port, char c);

extern void ToggleAndLockSyncBits (void);
extern void UnlockAndUpdateSyncBits (void);

void ParseAndSendTxStr (unsigned char a, unsigned char UseDelim)
{
	unsigned char *src		 = txrecstr + 2;
	unsigned char  swaptemp	 = SRecSwap;
	unsigned char  flipstate = SRecSwap;
	unsigned char  c;				   // temp stroage to speed up algorithm
	unsigned char  delim = TxStrDelim; // move this var from to idata for this routine
	// accelerator variables, these conditions are static for the duration of this
	// algorithm, we do not have to evaluiate them every iteration of the loop
	// i wish i could use f0, f1 and f2.  that would free memory
	bool UseExcludeAlgor = UseDelim && TxStrDelimMode == EXCLUDE_DELIMITER;
	bool UseIncludeAlgor = UseDelim && TxStrDelimMode == INCLUDE_DELIMITER;

	if (a)
		ToggleAndLockSyncBits ();
	while (a--)
	{							  // a contains the number of characters in the buffer left to copy
		c = *(src + flipstate--); // get a byte out of the curent word
		// check for excluded end delimiter
		if (UseExcludeAlgor && c == delim)
			break;
		// add to the port
//		my_putc (0, (char)c); //TODO ASCII code so disabled it
		// check for included end delimiter
		if (UseIncludeAlgor && c == delim)
			break;

		// update flipstate
		if (flipstate == 0xFF)
			src += (flipstate = swaptemp) + 1;
	}
//	UnlockAndUpdateSyncBits (); //TODO Ascii code so disabled it

	// adjust the stored string length in case we have encountered the delimiter
}


void SRecProtSetTxRec (MSG *msg)
{
	if(!DnCheckAttrLen(msg,1,1))
		return;
	if(txrecnum == msg->buf[0])
		return;
	else
		return;

	txrecnum=msg->buf[0];

}

void SRecProtSetTxStr (MSG *msg)
{
	unsigned char a;
	if (msg->class == 0x0f)
	{
		// I am a short string
		if (!msg->buflen)
		{
			g_status = 0x13;
			return;
		}
		msg->buflen--;
		if (!DnCheckAttrLen (msg, msg->buf[0], msg->buf[0]))
			return;
		a = paramtxlen;
		msg->buf++;
	}
	else if (type == ARRAY)
	{
		a = 0;
	}
	else
	{
		if (!msg->buflen)
		{
			g_status = 0x13;
			return;
		}
		a = msg->buf[0];
		msg->buf++;
		msg->buflen--;
		if (type == STRING)
		{
			if (!msg->buflen)
			{
				g_status = 0x13;
				return;
			}
			if (msg->buf[0])
			{
				g_status = TOO_MUCH_DATA_2;
				return;
			}
			msg->buf++;
			msg->buflen--;
		}
	}
	if (msg->buflen < a)
		a = msg->buflen;
	if (a > TX_MAX_STRING_LEN)
	{
		g_status = TOO_MUCH_DATA_2;
		return;
	}
	if (a == 0 /*&& TxStrDelimMode!=NO_DELIMITER*/)
	{
		a				= msg->buflen;
		UseTxDelimAlgor = 1;
	}
	txrecstr[0] = a;
	txrecstr[1] = 0;
	// now, just copy the string to the buffer
	xdata_memcpy (&txrecstr[2], msg->buf, a);
	/*
	{//scope
	unsigned char * src = msg->buf;
	unsigned char * dest = txrecstr+2;
	while(a--){
		*(dest++)=*(src++);
		}
	}
	*/
	if (SRecMode)
	{
		if (msg->class == 0x0f)
			ParseAndSendTxStr (((ParamUseTxDelimAlgor || txrecstr[0] < paramtxlen) ? (txrecstr[0]) : (paramtxlen)),
							   (unsigned char)ParamUseTxDelimAlgor);
		else
			ParseAndSendTxStr (txrecstr[0], (unsigned char)UseTxDelimAlgor);
	} // send the string if immediate send
	msg->buflen = 0;
}

void SRecProtSetTxStrType (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 2)
	{
		g_status = INVALID_ATTRIB_VALUE;
		return;
	}
	type = msg->buf[0];
	Write_EE_Byte (EE_TX_DATA_STR_TYPE, type);
}

void SRecProtSetSerHwInst (MSG *msg)
{
	msg		 = msg;
	g_status = ATTRIBUTE_NOT_SETTABLE;
}

void SRecProtSetMode (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 1)
	{
		g_status = INVALID_ATTRIB_VALUE;
		return;
	}
	SRecMode = msg->buf[0];
	Write_EE_Byte (EE_TX_REC_HEADER_MODE, msg->buf[0]);
	msg->buflen = 0;
}

void SRecProtSetSwap (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 3)
	{
		g_status = 0x09;
		return;
	}
	SRecSwap = msg->buf[0];
	Write_EE_Byte (EE_TX_SWAP_MODE, SRecSwap);
	msg->buflen = 0;
}

void SRecProtSetTxStrMaxLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if ((CNXN_ESTABLISHED == ConnectionObjectRAM[POLL_CNXN].state)
		|| (CNXN_ESTABLISHED == ConnectionObjectRAM[STROBE_CNXN].state)
		|| (CNXN_ESTABLISHED == ConnectionObjectRAM[COS_CNXN].state))
	{
		g_status  = 0x10;
		g_addCode = 0xff;
	}
	else if ((msg->buf[0] > TX_MAX_STRING_LEN) || (!(msg->buf[0])))
	{
		g_status = INVALID_ATTRIBUTE_DATA;
	}
	else
	{
		TxStrLen = msg->buf[0];
		Write_EE_Byte (EE_MAX_NUMBER_TX_CHARS, TxStrLen);
		msg->buflen = 0;
	}
}

void SRecProtSetTxStrDelimiter (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	TxStrDelim = msg->buf[0];
	Write_EE_Byte (EE_TX_END_DELIM, TxStrDelim);
	msg->buflen = 0;
}

void SRecProtSetTxStrDelimiterMode (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 2)
	{
		g_status = INVALID_ATTRIB_VALUE;
		return;
	}
	TxStrDelimMode = msg->buf[0];
	Write_EE_Byte (EE_TX_END_DELIM_MODE, TxStrDelimMode);
	msg->buflen = 0;
}

void SRecProtGetTxStrMaxLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = TxStrLen;
	msg->buflen = 1;
}

void SRecProtGetTxStrDelimiter (MSG *msg)
{
	msg->buf[0] = TxStrDelim;
	msg->buflen = 1;
}

void SRecProtGetTxStrDelimiterMode (MSG *msg)
{
	msg->buf[0] = TxStrDelimMode;
	msg->buflen = 1;
}

void SRecProtGetStatus (MSG *msg)
{
	msg->buf[0] = TxSts;
	msg->buflen = 1;
}

void SRecProtSetStatus (MSG *msg)
{
	TxSts		= 0;
	msg->buflen = 0;
}

/*****
 *
 *We use DNGenericGetSetFill in dnutils.c (library function)
 *
 *
 ***********/
flist SRecGetFuncList[] = {
	SRecProtGetSerHwInst,		   // Instance of serhw
	SRecProtGetTxRec,			   // Tx Record Number
	SRecProtGetTxStr,			   // TxString
	SRecProtGetTxStrType,		   // SHORT_STRING,STRING,ARRAY
	SRecProtGetTxStrMaxLen,		   // 0-TXMAXSTRLEN
	SRecProtGetTxStrDelimiter,	   // 0-255 ascii char.
	SRecProtGetTxStrDelimiterMode, // Include Delimiter, exclude delimiter
	SRecProtGetMode,			   // Immediate tx, wait for record
	SRecProtGetSwap,			   // 1-txStrMaxLen ---- Must be multiple
	SRecProtGetStatus			   // 1-txStrMaxLen ---- Must be multiple
};								   // of string len-1

flist SRecSetFuncList[] = {
	SRecProtSetSerHwInst,		   // Instance of serhw
	SRecProtSetTxRec,			   // Tx Record Number
	SRecProtSetTxStr,			   // TxString
	SRecProtSetTxStrType,		   // SHORT_STRING,STRING,ARRAY
	SRecProtSetTxStrMaxLen,		   // 0-TXMAXSTRLEN
	SRecProtSetTxStrDelimiter,	   // 0-255 ascii char.
	SRecProtSetTxStrDelimiterMode, // Include Delimiter, exclude delimiter
	SRecProtSetMode,			   // Immediate tx, wait for record
	SRecProtSetSwap,			   // 1-txStrMaxLen ---- Must be multiple
	SRecProtSetStatus			   // 1-txStrMaxLen ---- Must be multiple
};								   // of string len-1

void *SRecProtFunc (MSG *msg)
{
	g_addCode = 0xff;
	if (msg->instance == 0)
		;
	else if (msg->instance == 1)
	{
		if (msg->service == GET_REQ)
		{
			return DNChkAndRetAttrNum (SRecGetFuncList, 10, msg->attribute);
		}
		else if (msg->service == SET_REQ)
		{
			return DNChkAndRetAttrNum (SRecSetFuncList, 10, msg->attribute);
		}
	}
	else
	{
		g_status = OBJECT_DOES_NOT_EXIST;
		return NULL;
	}
	g_status = SERVICE_NOT_SUPPORTED;
	return NULL;
}

/*****************************************************************************************************/

// Recieve Record Protection!!!!!1
#define MAX_MAX_RX_LEN BYTES_OF_SER_DATA

unsigned char  StartMode					  = NO_DELIMITER;
unsigned char  EndMode						  = INCLUDE_DELIMITER;
unsigned char  RRecStart					  = ':';
unsigned char  RRecEnd						  = 0x0d;
unsigned char  RRecNum						  = 0;
unsigned char  RRecRxStr1[MAX_MAX_RX_LEN + 2] = { 0, 0 };
unsigned char  RRecRxStr2[MAX_MAX_RX_LEN + 2] = { 0, 0 };
unsigned char *RxAddPoint					  = RRecRxStr1 + 2;
unsigned char *RxLength						  = RRecRxStr1;
unsigned char *RRecRxStr					  = RRecRxStr2;
unsigned char  RRecRxStrType				  = SHORT_STRING;
unsigned char  RRecRxStrMaxLen				  = 20;
unsigned char  RRecPadMode					  = 0;
unsigned char  RRecPadChar					  = 0;
unsigned char  RRecSwap						  = 0;
unsigned char  RRecSwapState				  = 0;
unsigned char  RRecRcvMode					  = 1; // Auto INcrement rec num on recieve, Man Increment Rec Num
unsigned char  RRecStatus					  = 0;
#define NEW_DATA_AVAILIBLE	  0x80
#define NEW_DATA_TOGGLE		  0x20
#define RECORD_STATE_CONFLICT 0x40
void RxSwapAndIncrement (unsigned char newrecnum);

#ifdef GMM
// the first time received data is transferred by module,
//  it appeared that the pad char was not yet applied.  To be
//  extra safe, we'll just load them both with the pad char.
void GMM_pad_buffers (void);
void GMM_pad_buffers (void)
{
	unsigned char *zero;
	unsigned char  zerolen;

	zerolen = RRecRxStrMaxLen;
	zero	= RRecRxStr1 + 2;
	while (zerolen--)
		*(zero++) = RRecPadChar;

	zerolen = RRecRxStrMaxLen;
	zero	= RRecRxStr2 + 2;
	while (zerolen--)
		*(zero++) = RRecPadChar;
}
#endif // GMM

void RRecProtGetRxRec (MSG *msg)
{
	msg->buf[0] = RRecNum;
	msg->buflen = 1;
}

void RRecProtSetRxRec (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] != RRecNum)
	{
		if (!(RRecStatus & NEW_DATA_AVAILIBLE))
		{
			RRecStatus |= RECORD_STATE_CONFLICT;
			g_status = 0x0c;
			return;
		}
		else
			RRecStatus &= ~RECORD_STATE_CONFLICT;
		RxSwapAndIncrement (msg->buf[0]);
		RRecStatus = 0; // save a little space.  (I'm Desperate!)
	}
	msg->buflen = 0;
}

void RRecProtGetRxStr (MSG *msg)
{
	unsigned char  a;
	unsigned char *dest = msg->buf;
	unsigned char *src	= RRecRxStr;
	if (RRecRxStrType == SHORT_STRING || msg->class == 0x0f)
	{
		src++;
		a			 = 1;
		RRecRxStr[1] = RRecRxStr[0];
	}
	else if (RRecRxStrType == STRING)
	{
		RRecRxStr[1] = 0;
		a			 = 2;
	}
	else
	{
		src += 2;
		a = 0;
	}
	if (RRecPadMode)
	{
		a += RRecRxStrMaxLen;
	}
	else
	{
		a += ((RRecRxStr[0] + RRecSwap) / (RRecSwap + 1)) * (RRecSwap + 1);
	}
	msg->buflen = a;
	while (a--)
	{
		*(dest++) = *(src++);
	}
	RRecRxStr[1] = 0;
}

// special parameter object entry point
void RRecProtGetRxDataLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = RRecRxStr[0];
	msg->buflen = 1;
}

void RRecProtGetSerHwInst (MSG *msg)
{
	msg->buf[0] = 1;
	msg->buflen = 1;
}

void RRecProtGetRxStrType (MSG *msg)
{
	msg->buf[0] = RRecRxStrType;
	msg->buflen = 1;
}

void RRecProtGetPadMode (MSG *msg)
{
	msg->buf[0] = RRecPadMode;
	msg->buflen = 1;
}

void RRecProtGetPadChar (MSG *msg)
{
	msg->buf[0] = RRecPadChar;
	msg->buflen = 1;
}

void RRecProtGetRxStartChar (MSG *msg)
{
	msg->buf[0] = RRecStart;
	msg->buflen = 1;
}

void RRecProtGetRxEndChar (MSG *msg)
{
	msg->buf[0] = RRecEnd;
	msg->buflen = 1;
}

void RRecProtGetRxStartFormMode (MSG *msg)
{
	msg->buf[0] = StartMode;
	msg->buflen = 1;
}

void RRecProtGetRxEndFormMode (MSG *msg)
{
	msg->buf[0] = EndMode;
	msg->buflen = 1;
}

void RRecProtGetSwap (MSG *msg)
{
	msg->buf[0] = RRecSwap;
	msg->buflen = 1;
}

void RRecProtGetAutoInc (MSG *msg)
{
	msg->buf[0] = RRecRcvMode;
	msg->buflen = 1;
}

void RRecProtGetStatus (MSG *msg)
{
	msg->buf[0] = RRecStatus;
	msg->buflen = 1;
}

void RRecProtGetRxLen (MSG *msg)
{
	msg->buf[0] = RRecRxStrMaxLen;
	msg->buflen = 1;
}

void RRecProtSetRxStr (MSG *msg)
{
	msg = msg;
	// no
}

void RRecProtSetSerHwInst (MSG *msg)
{
	msg = msg;
	// no
}

void RRecProtSetRxStrType (MSG *msg)
{
	unsigned char temp = msg->buf[0];
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (temp > 2)
	{
		g_status = INVALID_ATTRIBUTE_DATA;
		return;
	}
	RRecRxStrType = temp;
	Write_EE_Byte (EE_RCV_DATA_STR_TYPE, temp);
	msg->buflen = 0;
}
/////////////////////////////////////////////////yo!
void RRecProtSetPadMode (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 1)
	{
		g_status = 0x09;
		return;
	}
	RRecPadMode = msg->buf[0];
	Write_EE_Byte (EE_PAD_MODE, RRecPadMode);
	msg->buflen = 0;
}

void RRecProtSetPadChar (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	RRecPadChar = msg->buf[0];
	Write_EE_Byte (EE_PAD_CHAR, RRecPadChar);
	msg->buflen = 0;
}

void RRecProtSetRxLen (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if ((CNXN_ESTABLISHED == ConnectionObjectRAM[POLL_CNXN].state)
		|| (CNXN_ESTABLISHED == ConnectionObjectRAM[STROBE_CNXN].state)
		|| (CNXN_ESTABLISHED == ConnectionObjectRAM[COS_CNXN].state))
	{
		g_status  = 0x10;
		g_addCode = 0xff;
	}
	else if ((msg->buf[0] > MAX_MAX_RX_LEN) || (!(msg->buf[0])))
	{
		g_status = INVALID_ATTRIBUTE_DATA;
	}
	else
	{
		RRecRxStrMaxLen = msg->buf[0];
		Write_EE_Byte (EE_MAX_NUMBER_RCV_CHARS, RRecRxStrMaxLen);
		msg->buflen = 0;
	}
}

void RRecProtSetRxStartChar (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	RRecStart = msg->buf[0];
	Write_EE_Byte (EE_RCV_DELIM_START_CHAR, RRecStart);
	msg->buflen = 0;
}

void RRecProtSetRxEndChar (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	RRecEnd = msg->buf[0];
	Write_EE_Byte (EE_RCV_DELIM_END_CHAR, RRecEnd);
	msg->buflen = 0;
}

void RRecProtSetRxStartFormMode (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 2)
	{
		g_status = 0x09;
		return;
	}
	StartMode = msg->buf[0];
	Write_EE_Byte (EE_RCV_DELIM_START_MODE, StartMode);
	msg->buflen = 0;
}

void RRecProtSetRxEndFormMode (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 2)
	{
		g_status = 0x09;
		return;
	}
	EndMode = msg->buf[0];
	Write_EE_Byte (EE_RCV_DELIM_END_MODE, EndMode);
	msg->buflen = 0;
}

void RRecProtSetSwap (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 3)
	{
		g_status = 0x09;
		return;
	}

	RRecSwap = msg->buf[0];
	Write_EE_Byte (EE_RCV_SWAP_MODE, RRecSwap);
	msg->buflen = 0;
}
extern bool AssyCRxRecNum;

void RRecProtSetAutoInc (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 1, 1))
		return;
	if (msg->buf[0] > 1)
	{
		g_status = 0x09;
		return;
	}
	RRecRcvMode	  = msg->buf[0];
	AssyCRxRecNum = !(RRecRcvMode && 1);
	Write_EE_Byte (EE_RCV_REC_AUTO_INC, RRecRcvMode);
	msg->buflen = 1;
}

void RRecProtSetStatus (MSG *msg)
{
	msg->buflen = 0;
}

/*****
 *
 *We use DNGenericGetSetFill in dnutils.c (library function)
 *		 SAVES A LOT OF SPACE, YOU SHOULD USE THIS TOO (if you can)!!!!
 *
 ***********/

flist RRecGetFuncs[]
	= { RRecProtGetSerHwInst, RRecProtGetRxRec,			  RRecProtGetRxStr,			RRecProtGetRxStrType,
		RRecProtGetPadMode,	  RRecProtGetPadChar,		  RRecProtGetRxLen,			RRecProtGetRxStartChar,
		RRecProtGetRxEndChar, RRecProtGetRxStartFormMode, RRecProtGetRxEndFormMode, RRecProtGetSwap,
		RRecProtGetAutoInc,	  RRecProtGetStatus };

flist RRecSetFuncs[]
	= { RRecProtSetSerHwInst, RRecProtSetRxRec,			  RRecProtSetRxStr,			RRecProtSetRxStrType,
		RRecProtSetPadMode,	  RRecProtSetPadChar,		  RRecProtSetRxLen,			RRecProtSetRxStartChar,
		RRecProtSetRxEndChar, RRecProtSetRxStartFormMode, RRecProtSetRxEndFormMode, RRecProtSetSwap,
		RRecProtSetAutoInc,	  RRecProtSetStatus };

void *RRecProtFunc (MSG *msg)
{
	g_addCode = 0xff;
	if (msg->instance == 0)
		;
	else if (msg->instance == 1)
	{
		if (msg->service == GET_REQ)
		{
			return DNChkAndRetAttrNum (RRecGetFuncs, 13, msg->attribute);
		}
		else if (msg->service == SET_REQ)
		{
			return DNChkAndRetAttrNum (RRecSetFuncs, 13, msg->attribute);
		}
	}
	else
	{
		g_status = OBJECT_DOES_NOT_EXIST;
		return NULL;
	}
	g_status = SERVICE_NOT_SUPPORTED;
	return NULL;
}

/*******
 *
 *Receive Main Routines
 *		Main Loop Functions
 *
 *******/

unsigned char debugbuffer[20];
unsigned char debugcount;
unsigned char debugerrorcount = 0;

extern void TriggerCOS (void);

bool rxbuffertouse = 1;
void RxSwapAndIncrement (unsigned char newRecNum)
{
	TriggerCOS ();
#ifdef GMM
	if (GMMRAM.active)
	{
		RRecStatus |= NEW_DATA_AVAILIBLE;
	}
#endif // GMM
	rxbuffertouse = !rxbuffertouse;
	if (rxbuffertouse)
	{
		RxAddPoint	  = RRecRxStr1 + 2;
		RxLength	  = RRecRxStr1;
		RRecRxStr1[0] = 0; // zero the length bytes of the string
		RRecRxStr1[1] = 0;
		RRecRxStr	  = RRecRxStr2;
	}
	else
	{
		RxAddPoint	  = RRecRxStr2 + 2;
		RxLength	  = RRecRxStr2;
		RRecRxStr2[0] = 0;
		RRecRxStr2[1] = 0;
		RRecRxStr	  = RRecRxStr1;
	}
	/*
		for(debugcount=0;debugcount<sizeof(debugbuffer);debugcount++){
			if(RRecRxStr[2+debugcount]!=debugbuffer[debugcount]){
				debugbuffer[debugcount]=RRecRxStr[2+debugcount];
				debugerrorcount++;
				if(debugerrorcount>0x80){
					debugerrorcount=0;
					}
				}
			}
	*/
	{ // Pad Me Baby!
		unsigned char *zero	   = RxAddPoint;
		unsigned char  zerolen = RRecRxStrMaxLen;
		while (zerolen--)
			*(zero++) = RRecPadChar;
	}
	RRecNum = newRecNum;

	if (0) // TODO ASCII Function so disabled it (isparityerror (0))
	{
		clearparityerror (0);
		RRecStatus |= 0x04;
	}
	else
	{
		RRecStatus &= ~0x04;
	}
}

bool		  isportempty (unsigned char port); // ascii.c
char		  getc (unsigned char portnum);
bool		  aquire   = 0;
unsigned char oldrxsts = 0;

void RRecMain (void)
{
	unsigned char c;

	if (oldrxsts != RRecStatus)
	{
		TriggerCOS ();
		oldrxsts = RRecStatus;
	}

	if (RRecStatus & NEW_DATA_AVAILIBLE)
		return;
	if (isportempty (0))
		return;
	c = getc (0);
	if (StartMode == NO_DELIMITER)
		aquire = 1;

	if (!aquire && StartMode == INCLUDE_DELIMITER && c == RRecStart)
	{
		aquire = 1;
	}
	else if (aquire && EndMode == EXCLUDE_DELIMITER && c == RRecEnd)
	{
		aquire		  = 0;
		RRecSwapState = 0;
		if (RRecRcvMode)
			RxSwapAndIncrement (RRecNum + 1);
		else
		{
			RRecStatus |= NEW_DATA_AVAILIBLE;
			RRecStatus ^= NEW_DATA_TOGGLE;
		}
	}
	if (aquire)
	{
		if (RRecSwapState % (RRecSwap + 1) == 0)
		{
			// pad RxAddPoint with the pad char for rxswap+1
			RRecSwapState = 0;
			while (RRecSwapState++ != (RRecSwap + 1))
			{
				*(RxAddPoint++) = RRecPadChar;
			}
			RRecSwapState = 0;
		}

		*((RxAddPoint) - (++RRecSwapState)) = c;
		*(RxLength) += 1;
		if (*RxLength == RRecRxStrMaxLen)
		{
			// tell the world that we are at or limt
			aquire		  = 0;
			RRecSwapState = 0;
			if (RRecRcvMode)
				RxSwapAndIncrement (RRecNum + 1);
			else
			{
				RRecStatus |= NEW_DATA_AVAILIBLE;
				RRecStatus ^= NEW_DATA_TOGGLE;
			}

			return;
		}
	}
	if (!aquire && StartMode == EXCLUDE_DELIMITER && c == RRecStart)
	{
		aquire = 1;
	}
	else if (aquire && EndMode == INCLUDE_DELIMITER && c == RRecEnd)
	{
		aquire		  = 0;
		RRecSwapState = 0;

		if (RRecRcvMode)
			RxSwapAndIncrement (RRecNum + 1);
		else
		{
			RRecStatus |= NEW_DATA_AVAILIBLE;
			RRecStatus ^= NEW_DATA_TOGGLE;
		}
	}
}
extern bool AssyCRxRecNum;
void		InitRxTxAssy (void)
{
	SRecMode		= Read_EE_Byte (EE_TX_REC_HEADER_MODE);
	type			= Read_EE_Byte (EE_TX_DATA_STR_TYPE);
	SRecSwap		= Read_EE_Byte (EE_TX_SWAP_MODE);
	TxStrLen		= Read_EE_Byte (EE_MAX_NUMBER_TX_CHARS);
	TxStrDelim		= Read_EE_Byte (EE_TX_END_DELIM);
	TxStrDelimMode	= Read_EE_Byte (EE_TX_END_DELIM_MODE);
	RRecRxStrType	= Read_EE_Byte (EE_RCV_DATA_STR_TYPE);
	RRecPadMode		= Read_EE_Byte (EE_PAD_MODE);
	RRecPadChar		= Read_EE_Byte (EE_PAD_CHAR);
	RRecRxStrMaxLen = Read_EE_Byte (EE_MAX_NUMBER_RCV_CHARS);
	RRecStart		= Read_EE_Byte (EE_RCV_DELIM_START_CHAR);
	RRecEnd			= Read_EE_Byte (EE_RCV_DELIM_END_CHAR);
	StartMode		= Read_EE_Byte (EE_RCV_DELIM_START_MODE);
	EndMode			= Read_EE_Byte (EE_RCV_DELIM_END_MODE);
	RRecSwap		= Read_EE_Byte (EE_RCV_SWAP_MODE);
	RRecRcvMode		= Read_EE_Byte (EE_RCV_REC_AUTO_INC);
	AssyCRxRecNum	= (RRecRcvMode == 0);
}

unsigned char GetSizeSize (unsigned char khgh)
{
	if (khgh == ARRAY)
		return (0);
	if (khgh == STRING)
		return (2);
	if (khgh == SHORT_STRING)
		return (1);
	return 0;
}
