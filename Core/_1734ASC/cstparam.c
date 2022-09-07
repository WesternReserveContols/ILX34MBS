/*************
 *
 *cstparam.c
 *
 *  Custom Parameter Object
 *         It does not use the default parameter
 *         Object
 *
 *
 *
 **************/
#include <stddef.h>
#include "ee_adr.h"
#include "msg.h"
#include "serial_config.h"

/**************************************

******************************************/

/*********
 *
 *We use the standard device net fill function
 *
 *
 *
 **********/

/////////// we are only going to implement the device net stub!

///////// Because most of the data is hard coded, I am going to make a structure
void Write_EE_Byte (unsigned int addr, unsigned char val)
{
	BumpCCV ();
	EEPROMObjectWriteAndUpdate (addr, val);
}

unsigned char Read_EE_Byte (unsigned int addr)
{
	return EEPROMObjectRead (addr);
}

typedef struct
{
	unsigned char *val;
	unsigned char  len;
} ROM_ATTRIBUTE;

typedef struct
{
	void *		  attr1get;
	void *		  attr1set;
	ROM_ATTRIBUTE romattrs[5];
} PARAMROM;

unsigned char USINT_0[]						= { 0x00 };
unsigned char USINT_1[]						= { 0x01 };
unsigned char DESCRIPTOR_MONITOR[]			= { 0x20, 0x00 };
unsigned char DESCRIPTOR_READONLY[]			= { 0x10, 0x00 };
unsigned char DESCRIPTOR_MONITOR_READONLY[] = { 0x30, 0x00 };
unsigned char DATA_TYPE_USINT[]				= { 8 };
unsigned char DATA_TYPE_SHORT_STRING[]		= { 23 };

/**********
  prototypes for used functions;
  *********/

void AsciiGetDPS (MSG *msg);
void AsciiGetBaudrate (MSG *msg);
void RRecProtGetRxLen (MSG *msg);
void RRecProtGetRxStrType (MSG *msg);
void RRecProtGetRxStartFormMode (MSG *msg);
void RRecProtGetRxEndFormMode (MSG *msg);
void RRecProtGetRxStartChar (MSG *msg);
void RRecProtGetRxEndChar (MSG *msg);
void RRecProtGetPadMode (MSG *msg);
void RRecProtGetPadChar (MSG *msg);
void RRecProtGetSwap (MSG *msg);
void RRecProtGetAutoInc (MSG *msg);
void AssyGetProduceSize (MSG *msg);
void RRecProtGetRxStr (MSG *msg);
void RRecProtGetRxDataLen (MSG *msg);
void RRecProtGetRxRec (MSG *msg);
void SRecProtGetTxStrMaxLen (MSG *msg);
void SRecProtGetTxStrType (MSG *msg);
void SRecProtGetTxStrDelimiter (MSG *msg);
void SRecProtGetTxStrDelimiterMode (MSG *msg);
void SRecProtGetSwap (MSG *msg);
void SRecProtGetMode (MSG *msg);
void AssyGetConsumeSize (MSG *msg);
void SRecProtGetTxStr (MSG *msg);
void SRecProtGetTxDataLen (MSG *msg);
void SRecProtGetTxRec (MSG *msg);
void AssyGetComboStatus (MSG *msg);
void AsciiSetDPS (MSG *msg);
void AsciiSetBaudrate (MSG *msg);
void RRecProtSetRxLen (MSG *msg);
void RRecProtSetRxStrType (MSG *msg);
void RRecProtSetRxStartFormMode (MSG *msg);
void RRecProtSetRxEndFormMode (MSG *msg);
void RRecProtSetRxStartChar (MSG *msg);
void RRecProtSetRxEndChar (MSG *msg);
void RRecProtSetPadMode (MSG *msg);
void RRecProtSetPadChar (MSG *msg);
void RRecProtSetSwap (MSG *msg);
void RRecProtSetAutoInc (MSG *msg);
void RRecProtSetRxRec (MSG *msg);
void SRecProtSetTxStrMaxLen (MSG *msg);
void SRecProtSetTxStrType (MSG *msg);
void SRecProtSetTxStrDelimiter (MSG *msg);
void SRecProtSetTxStrDelimiterMode (MSG *msg);
void SRecProtSetSwap (MSG *msg);
void SRecProtSetMode (MSG *msg);
void SRecProtSetTxStr (MSG *msg);
void SRecProtSetTxDataLen (MSG *msg);
void SRecProtSetTxRec (MSG *msg);
void AssySetComboStatus (MSG *msg);

PARAMROM paramlist[28] = {
	{
		// instance 1
		AsciiGetDPS, // get function for parameter attribute 1
		AsciiSetDPS, // set function for parameter attribute 1
		{			 // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  1		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  1		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 1
	{
		// instance 2
		AsciiGetBaudrate, // get function for parameter attribute 1
		AsciiSetBaudrate, // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 2
	{
		// instance 3
		RRecProtGetRxLen, // get function for parameter attribute 1
		RRecProtSetRxLen, // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 3
	{
		// instance 4
		RRecProtGetRxStartFormMode, // get function for parameter attribute 1
		RRecProtSetRxStartFormMode, // set function for parameter attribute 1
		{							// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 4
	{
		// instance 5
		RRecProtGetRxStartChar, // get function for parameter attribute 1
		RRecProtSetRxStartChar, // set function for parameter attribute 1
		{						// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 5
	{
		// instance 6
		RRecProtGetRxEndFormMode, // get function for parameter attribute 1
		RRecProtSetRxEndFormMode, // set function for parameter attribute 1
		{						  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 6
	{
		// instance 7
		RRecProtGetRxEndChar, // get function for parameter attribute 1
		RRecProtSetRxEndChar, // set function for parameter attribute 1
		{					  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 7
	{
		// instance 8
		RRecProtGetRxStrType, // get function for parameter attribute 1
		RRecProtSetRxStrType, // set function for parameter attribute 1
		{					  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 8
	{
		// instance 9
		RRecProtGetPadMode, // get function for parameter attribute 1
		RRecProtSetPadMode, // set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 9
	{
		// instance 10
		RRecProtGetPadChar, // get function for parameter attribute 1
		RRecProtSetPadChar, // set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 10
	{
		// instance 11
		RRecProtGetSwap, // get function for parameter attribute 1
		RRecProtSetSwap, // set function for parameter attribute 1
		{				 // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 11
	{
		// instance 12
		RRecProtGetAutoInc, // get function for parameter attribute 1
		RRecProtSetAutoInc, // set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 12
	{
		// instance 13
		AssyGetProduceSize, // get function for parameter attribute 1
		NULL,				// set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR_READONLY, // val
			  2							   // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 13
	{
		// instance 14
		RRecProtGetRxStr, // get function for parameter attribute 1
		NULL,			  // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR_READONLY, // val
			  2							   // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_SHORT_STRING, // val
			  1						  // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 14
	{
		// instance 15
		RRecProtGetRxDataLen, // get function for parameter attribute 1
		NULL,				  // set function for parameter attribute 1
		{					  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR_READONLY, // val
			  2							   // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 15
	{
		// instance 16
		RRecProtGetRxRec, // get function for parameter attribute 1
		RRecProtSetRxRec, // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 16
	{
		// instance 17
		SRecProtGetTxStrMaxLen, // get function for parameter attribute 1
		SRecProtSetTxStrMaxLen, // set function for parameter attribute 1
		{						// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 17
	{
		// instance 18
		SRecProtGetTxStrDelimiterMode, // get function for parameter attribute 1
		SRecProtSetTxStrDelimiterMode, // set function for parameter attribute 1
		{							   // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 18
	{
		// instance 19
		SRecProtGetTxStrDelimiter, // get function for parameter attribute 1
		SRecProtSetTxStrDelimiter, // set function for parameter attribute 1
		{						   // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 19
	{
		// instance 20
		SRecProtGetTxStrType, // get function for parameter attribute 1
		SRecProtSetTxStrType, // set function for parameter attribute 1
		{					  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 20
	{
		// instance 21
		SRecProtGetSwap, // get function for parameter attribute 1
		SRecProtSetSwap, // set function for parameter attribute 1
		{				 // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 21
	{
		// instance 22
		SRecProtGetMode, // get function for parameter attribute 1
		SRecProtSetMode, // set function for parameter attribute 1
		{				 // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 22
	{
		// instance 23
		AssyGetConsumeSize, // get function for parameter attribute 1
		NULL,				// set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR_READONLY, // val
			  2							   // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 23
	{
		// instance 24
		SRecProtGetTxStr, // get function for parameter attribute 1
		SRecProtSetTxStr, // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_SHORT_STRING, // val
			  1						  // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 24
	{
		// instance 25
		SRecProtGetTxDataLen, // get function for parameter attribute 1
		SRecProtSetTxDataLen, // set function for parameter attribute 1
		{					  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 25
	{
		// instance 26
		SRecProtGetTxRec, // get function for parameter attribute 1
		SRecProtSetTxRec, // set function for parameter attribute 1
		{				  // romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR, // val
			  2					  // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	},				   // end instance 26
	{
		// instance 27
		AssyGetComboStatus, // get function for parameter attribute 1
		NULL,				// set function for parameter attribute 1
		{					// romattrs[5]
		  // attribute 2
		  {
			  USINT_0, // val
			  1		   // length
		  },
		  // attribute 3
		  {
			  NULL, // val
			  0		// length
		  },
		  // attribute 4
		  {
			  DESCRIPTOR_MONITOR_READONLY, // val
			  2							   // length
		  },
		  // attribute 5
		  {
			  DATA_TYPE_USINT, // val
			  1				   // length
		  },
		  // attribute 6
		  {
			  USINT_1, // val
			  0		   // length
		  } }		   // end romattrs[5]
	}				   // end instance 27
};					   // end PARAM_ROM

void ParamClassInstance (MSG *msg)
{
	if (msg->service != GET_REQ)
	{
		g_status = SERVICE_NOT_SUPP;
		return;
	}
	if (msg->buflen)
	{
		g_status = 0x15;
		return;
	}
	msg->buf[1] = 0;
	msg->buflen = 2;
	switch (msg->attribute)
	{
	case 2:
		msg->buf[0] = 27;
		break;
	case 8: // PARAMETER_CLASS_DESCRIPTOR:
		msg->buf[0] = 0x09;
		break;
	case 9: // CONFIG_ASSY_INSTANCE:
		msg->buf[0] = 0;
		break;
	default:
		g_status = 0x14;
	}
}

void ParamGetRom (MSG *msg)
{
	unsigned char  count   = paramlist[msg->instance].romattrs[msg->attribute - 2].len;
	unsigned char *listptr = paramlist[msg->instance].romattrs[msg->attribute - 2].val + count;
	msg->buflen			   = count;
	msg->buf += count;
	while (count--)
	{
		*(--msg->buf) = *(--listptr);
	}
}

void InitRxTxAssy (void);
void RestoreSerialFromEE (void);
void CustParamInit (void);
void ParamClassReset (void)
{
	Write_EE_Byte (EE_CUST_PARAM_INIT_VAR, 0);
	CustParamInit ();
	InitRxTxAssy ();
	RestoreSerialFromEE ();
}

// New function called from the EE reset state
// same as the ParamClassReset above but
//  does not call Init on our Serial Port
void ParamEEReset (void)
{
	Write_EE_Byte (EE_CUST_PARAM_INIT_VAR, 0);
	CustParamInit ();
}

void *ParamFuncRom (MSG *msg)
{
	if (msg->instance > 16) // Changed from 27 to 16 to match Legacy code.
	{
		g_status = OBJECT_DOES_NOT_EXIST;
		return NULL;
	}
	if (!msg->instance--)
	{ // to keep from having to do subsequent -1's on the attribute
		if (msg->service == 0x0e)
			return ParamClassInstance;
		else if (msg->service == 0x05)
			return ParamClassReset;
		g_status = SERVICE_NOT_SUPPORTED;
		return NULL;
	}
	if (msg->service == 0x10)
	{
		if (msg->attribute == 1)
		{
			if (!paramlist[msg->instance].attr1set)
			{
				g_status = ATTRIBUTE_NOT_SETTABLE;
				return NULL;
			}
			else
				return paramlist[msg->instance].attr1set;
		}
		else if (msg->attribute && msg->attribute <= 6)
		{
			g_status = ATTRIBUTE_NOT_SETTABLE;
			return NULL;
		}
	}
	else if (msg->service == 0x0e)
	{
		if (msg->attribute == 1)
			return paramlist[msg->instance].attr1get;
		else if (msg->attribute && msg->attribute <= 6)
			return ParamGetRom;
	}
	else
	{
		g_status = SERVICE_NOT_SUPPORTED;
		return NULL;
	}
	g_status = ATTRIBUTE_NOT_SUPPORTED;
	return NULL;
}

void CustParamInit (void)
{
	if (Read_EE_Byte (EE_CUST_PARAM_INIT_VAR) != 0x55)
	{
		Write_EE_Byte (EE_CUST_PARAM_INIT_VAR, 0x55);
		Write_EE_Byte (EE_TX_REC_HEADER_MODE, 0);
		Write_EE_Byte (EE_TX_DATA_STR_TYPE, 0x1);
		Write_EE_Byte (EE_TX_SWAP_MODE, 0);
		Write_EE_Byte (EE_MAX_NUMBER_TX_CHARS, 20);
		Write_EE_Byte (EE_TX_END_DELIM, 0x0d);
		Write_EE_Byte (EE_TX_END_DELIM_MODE, 2);
		Write_EE_Byte (EE_RCV_DATA_STR_TYPE, 0x1);
		Write_EE_Byte (EE_PAD_MODE, 1);
		Write_EE_Byte (EE_PAD_CHAR, 0);
		Write_EE_Byte (EE_MAX_NUMBER_RCV_CHARS, 20);
		Write_EE_Byte (EE_RCV_DELIM_START_CHAR, 0x3a);
		Write_EE_Byte (EE_RCV_DELIM_END_CHAR, 0x0d);
		Write_EE_Byte (EE_RCV_DELIM_START_MODE, 0);
		Write_EE_Byte (EE_RCV_DELIM_END_MODE, 2);
		Write_EE_Byte (EE_RCV_SWAP_MODE, 0);
		Write_EE_Byte (EE_RCV_REC_AUTO_INC, 1);
		Write_EE_Byte (EE_SERIAL_CHARACTER_FORMAT, 0);
		Write_EE_Byte (EE_SERIAL_BAUDRATE, 0);
	}
}
