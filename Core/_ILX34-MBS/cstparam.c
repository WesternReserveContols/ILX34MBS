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
#include "mbport.h"
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
unsigned char USINT_2[]						= { 0x02 };
unsigned char USINT_4[]						= { 0x04 };
unsigned char DESCRIPTOR_MONITOR[]			= { 0x00, 0x00 };
unsigned char DESCRIPTOR_READONLY[]			= { 0x10, 0x00 };
unsigned char DESCRIPTOR_MONITOR_READONLY[] = { 0x30, 0x00 };
//unsigned char DATA_TYPE_USINT[]				= { 8 };
unsigned char DATA_TYPE_SHORT_STRING[]		= { 23 };
unsigned char DATA_TYPE_SINT[]				= { 0xC2 };
unsigned char DATA_TYPE_INT[]				= { 0xC3 };
unsigned char DATA_TYPE_DINT[]				= { 0xC4 };
unsigned char DATA_TYPE_USINT[]				= { 0xC6 };


//#define USINT_0 0
//#define USINT_1 1
//#define DATA_TYPE_SINT  0xC2
//#define DATA_TYPE_USINT 0xC6
//#define DATA_TYPE_INT   0xC3
//#define DATA_TYPE_DINT  0xC4
//#define DESCRIPTOR_MONITOR 0x0000
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

extern void GetProduceAssyNum(MSG * msg);
extern void GetConsumeAssyNum(MSG  * msg);
extern void SetConsumeAssyNum(MSG * msg);
extern void SetProduceAssyNum(MSG * msg);

/* list of the parameters */
PARAMROM paramlist[16]=
{
	{//instance 1
	  GetFraming,//get function for parameter attribute 1
	  SetFraming,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_1,//USINT_0,//val
			  1		    //length
			},
		//attribute 3
			{
				USINT_2,//NULL,	//val
			  1		  //length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,  //val
			  1					       //length
			},
		//attribute 6
			{
				USINT_1,	//val
				1		//length
			}
		}//end romattrs[5]
  },//end instance 1

	{//instance 2
	  MB_GetProtocol,//get function for parameter attribute 1
	  MB_SetProtocol,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,	//val
			  1					      //length
			},
		//attribute 6
			{
			  USINT_1,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 2

	{//instance 3
  	GetConsumeAssyNum,
	  SetConsumeAssyNum,
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,	//val
			  1					      //length
			},
		//attribute 6
			{
			  USINT_1,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 3

  {//instance 4
	  GetBaudRate,//get function for parameter attribute 1
	  SetBaudRate,//set function for parameter attribute 1
	  {//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,	//val
			  1					      //length
			},
		//attribute 6
			{
					USINT_1,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 4

 	{//instance 5
  	MB_GetType,
	  MB_SetType,
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,	//val
			  1					      //length
			},
		//attribute 6
			{
			  USINT_1,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 5

	{//instance 6
	  GetProduceAssyNum,
	  SetProduceAssyNum,
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_SINT,	//val
			  1					      //length
			},
		//attribute 6
			{
			  USINT_1,	//val
			  1		//length
			}
		}//end romattrs[5]
   	},//end instance 6

	{//instance 7
	  MB_GetTimeout,
	  MB_SetTimeout,
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_DINT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_4,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 7

	{//instance 8
  	MB_GetSlaveID,//get function for parameter attribute 1
	  MB_SetSlaveID,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1			          //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 8

	{//instance 9
  	MB_GetCoil_StartAddr,//get function for parameter attribute 1
	  MB_SetCoil_StartAddr,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 9

	{//instance 10
  	MB_GetCoil_Count,//get function for parameter attribute 1
	  MB_SetCoil_Count,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								//length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1			          //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 10

	{//instance 11
  	MB_GetDiscInput_StartAddr,//get function for parameter attribute 1
	  MB_SetDiscInput_StartAddr,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 11

	{//instance 12
  	MB_GetDiscInput_Count,//get function for parameter attribute 1
	  MB_SetDiscInput_Count,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 12

	{//instance 13
  	MB_GetInReg_StartAddr,//get function for parameter attribute 1
	  MB_SetInReg_StartAddr,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 13

	{//instance 14
  	MB_GetInReg_Count,//get function for parameter attribute 1
	  MB_SetInReg_Count,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 14

	{//instance 15
  	MB_GetHoldReg_StartAddr,//get function for parameter attribute 1
	  MB_SetHoldReg_StartAddr,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  },//end instance 15

	{//instance 16
  	MB_GetHoldReg_Count,//get function for parameter attribute 1
	  MB_SetHoldReg_Count,//set function for parameter attribute 1
		{//romattrs[5]
		//attribute 2
			{
			  USINT_0,//val
			  1		//length
			},
		//attribute 3
			{
			  NULL,	//val
			  1		//length
			},
		//attribute 4
			{
			  DESCRIPTOR_MONITOR,	//val
			  2								    //length
			},
		//attribute 5
			{
			  DATA_TYPE_INT,	//val
			  1					      //length
			},
		//attribute 6
			{
				USINT_2,	//val
			  1		//length
			}
		}//end romattrs[5]
  }//end instance 16

};//end PARAM_ROM


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
	InitMbParam();
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
	if (msg->instance > 27)
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
