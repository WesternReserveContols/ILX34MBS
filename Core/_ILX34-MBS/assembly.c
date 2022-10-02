/**
 * @file assembly.c
 * @author Jignesh
 * @date 25 Jul 2022
 * @brief This file contains assembly level handling related functions and variables.
 *
 */

// Assembly.c
#include <dn_def.h>
#include <fifo.h>
#include <msg.h>
#include <serial_config.h>
#include <stdbool.h>
#include "ee_adr.h"
#include <string.h>
#include "mbport.h"
#include "dn_cnobj.h"

extern uchar P_InMsgBuffer[]; // Rick_TEST This shoudl be include in the App_Objects.h

#define ASSY_PINST		101
#define ASSY_CINST		102
#define ASSY_CONFIGINST 103

bool				 AssyCRxRecNum		   = 0;
bool				 AssyCTxRecNum_ws	   = 0;
bool				 AssyCRxRecNum_ws	   = 0;
bool				 AssyCFlowControlFlags = 0;
extern unsigned char TxStrLen;
extern unsigned char type;
bool				 AssyProdRecCounter = 1;
bool				 AssyProdStatus		= 1;
extern unsigned char SRecMode;
extern unsigned char RRecNum;
extern unsigned char RRecRxStrMaxLen;
extern unsigned char RRecRxStrType;
extern unsigned char RRecStatus;
extern unsigned char TxSts;
MSG					 AssyPMsg;
void				 RRecProtGetRxStr (MSG *msg); // returns a pointer, we need to copy it
void				 SRecProtSetTxStr (MSG *msg);
void				 SRecProtGetTxStr (MSG *msg);
void				 SRecProtSetTxRec (MSG *msg);
void				 RRecProtSetRxRec (MSG *msg);

//typedef struct
//{
//	unsigned char baudrate;
//	unsigned char flowcontrol;
//	unsigned char DataBits;
//	unsigned char Framing;
//	unsigned char Parity;
//	unsigned char status;
//	FIFO_CONTEXT  RxFifo;
//	FIFO_CONTEXT  TxFifo;
//} ASCIISTRUCT;

void SetProduceAssyNum(MSG * msg);
unsigned char ComputeIOConsumeSize(void);
unsigned char ComputeIOProduceSize(void);
void SetConsumeAssyNum(MSG * msg);
void GetProduceAssyNum(MSG * msg);
void GetConsumeAssyNum(MSG  * msg);


unsigned char ConsumeAssyNum,ProduceAssyNum;
extern unsigned char MaxRxSize;


/**
 * @brief AssyPFunc() Function to get the assembly produce message based parameters.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void AssyPFunc (MSG *msg)
{
	// Produce data for PLC Input
	// Rick_TEST Bug33 Added this section to perform GET and SET
	// If Implicit Poll message, then Class is not one of 101,105 or 106
	unsigned char AssyLength;
	unsigned char IsExplicit = TRUE;
	switch(msg->instance)
	{
	case 101:
		 AssyLength=30;
		 break;
	case 105:
		 AssyLength=62;
		 break;
	case 107:
		AssyLength=86;
		break;
	default:
		IsExplicit = FALSE;
		break;  // Stops here on Implicit Poll Reply
	}

	  unsigned char * prodbuf=msg->buf;

     if (IsExplicit)
     {
     	if (msg->service = 0xe)  //Service is GET
     	{
     		if (msg->attribute  == 3 )  // Rick_TEST
     		{
     			g_status = ATTR_NOT_SETTABLE;
     			return;
     		}

     		if (msg->attribute  == 4 )  // Rick_TEST
     		{
     			g_status = ATTR_NOT_SETTABLE;
     			return;
     		}
     	}

     	else // Service is SET
     	{
     		if (msg->attribute  == 3 )  // Rick_TEST
     		{
     			g_status = ATTR_NOT_SETTABLE;
     			return;
     		}

     		if (msg->attribute  == 4 )  // Rick_TEST
     		{
     			g_status = ATTR_NOT_SETTABLE;
     			return;
     		}
     	}
     }//  end of Explicit


	else // If Implicit Poll Reply  This was the orriginal code
     {
    	 *(prodbuf)= 0;
    	 *(prodbuf+1)= RRecStatus|TxSts|Ascii.Status;
    	 //produce the data
    	 AssyPMsg.class=0x04;
    	 AssyPMsg.buf=prodbuf;


    	 // add the record counter if we want to
    	 msg->buflen = 0;
    	 // if(AssyProdRecCounter)
    	 *(prodbuf++) = RRecNum;
    	 msg->buflen++;

    	 *(prodbuf++) = RRecStatus | TxSts | Ascii.Status;

    	 msg->buflen++;
    	 // pad for logix complient crap!
    	 if ((RRecRxStrType == SHORT_STRING) || (RRecRxStrType == ARRAY))
    	 {
    		 *(prodbuf++) = 0;
    		 msg->buflen++;
    	 }
    	 if (RRecRxStrType == ARRAY)
    	 {
    		 *(prodbuf++) = 0;
    		 msg->buflen++;
    	 }
    	 // produce the data
    	 AssyPMsg.class	= 0x04;
    	 AssyPMsg.buf	= prodbuf;
    	 AssyPMsg.buflen = 0;
    	 RRecProtGetRxStr (&AssyPMsg); // returns a pointer, we need to copy it
    	 msg->buflen += AssyPMsg.buflen;
     }

}
// these need to be because they are passed in the msgbuf!
unsigned char		 new_txrec_num;
unsigned char		 new_rxrec_num;
extern unsigned char RRecPadMode;
unsigned char		 CompAssyCSize (void);
/**
 * @brief AssyCFunc() Function to get the assembly consume message based parameters.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void AssyCFunc (MSG *msg)
{

	// Consume data From PLC Output
	unsigned char AssyLength;
	switch(msg->instance)
	{
	case 102:
		 AssyLength=26;
		 break;
	case 106:
		 AssyLength=58;
		 break;
	case 108:
		AssyLength=82;
		break;
	default:
		break;
	}

	if (msg->service == 0x0e)
	{

		unsigned char *outbuffer = msg->buf;
		unsigned char  difflen	 = 0;  // = 2;

		// replaces the msg->buf pointer.
		{
			unsigned char *dest = msg->buf;
			unsigned char *src;
			unsigned char  copylen;
			if (msg->attribute  == 3 )  // Rick_TEST Bug24 Read and write all Assembly attributes
			{
				//  Rick_TEST SRecProtGetTxStr (msg);
				src	= &P_InMsgBuffer[0];
				difflen += copylen = AssyLength;
				while (copylen--)
					*(dest++) = *(src++);

				msg->buflen= difflen;
				msg->buf	= outbuffer;
			}
			else // Attribute 4 -- Length
			{
				*dest 	= AssyLength;
				*(dest+1) = 0;
				msg->buflen  = 2;
				msg->buf	= dest;
			}

		}
	}
	else
	{

		if(!DnCheckAttrLen(msg,AssyLength,AssyLength))
			return;

		if (msg->attribute  == 4 )  // Rick_TEST
		{
			g_status = ATTR_NOT_SETTABLE;
			return;
		}

		/*  This is all carry over 1734ASC code that is not required

		if (AssyCFlowControlFlags && msg->buflen)
		{
			// I dont know what this will look like yet.
		}
		if (msg->buflen)
		{
			if (AssyCRxRecNum)
			{
				AssyCRxRecNum_ws = 1;
				new_rxrec_num	 = *(msg->buf);
			}
			msg->buf++;
			msg->buflen--;
		}

		if (msg->buflen)
		{
			if (!SRecMode)
			{
				AssyCTxRecNum_ws = 1;
				new_txrec_num	 = *(msg->buf);
			}
			msg->buf++;
			msg->buflen--;
		}
		if (msg->buflen)
		{
			if (type == SHORT_STRING || type == ARRAY)
			{
				msg->buf++;
				msg->buflen--;
			}
		}
		if (msg->buflen)
		{
			if (type == ARRAY)
			{
				msg->buf++;
				msg->buflen--;
			}
		}

		// copy over the tx data
		if (msg->buflen)
			SRecProtSetTxStr (msg);
		// tell the system that we have consumed the record numbers
		if (AssyCTxRecNum_ws)
		{
			AssyCTxRecNum_ws = 0;
			msg->buflen		 = 1;
			msg->buf		 = &new_txrec_num;
			SRecProtSetTxRec (msg);
		}
		if (AssyCRxRecNum_ws)
		{
			AssyCRxRecNum_ws = 0;
			msg->buflen		 = 1;
			msg->buf		 = &new_rxrec_num;
			RRecProtSetRxRec (msg);
		}
		*/

		// Rick_TEST Do you want to write this buffer to the MBPort?
		//MBM_QueMbTxMsg(msg->buf);   // Rick_TEST bug31
		msg->buflen	 = 0;
		*(msg->buf) = 0x90;
		*(msg->buf) = 01;
	}
}


void MessageObjectFormatSuccessMessage(void)
{
   // Fill in data values into transmit buffer
   //  Byte  0     : message header
   //  Byte  1   : RSP Bit set(Bit 7 ) and Set Response
   // TODO CurrFragObj.numbytes = 2;
   // TODO CurrFragObj.buffer[1] = RSP_BIT + 0x10;
}

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
void SRecProtGetTxStrMaxLen (MSG *msg);
void SRecProtGetTxStrType (MSG *msg);
void SRecProtGetTxStrDelimiter (MSG *msg);
void SRecProtGetTxStrDelimiterMode (MSG *msg);
void SRecProtGetSwap (MSG *msg);
void SRecProtGetMode (MSG *msg);

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
void SRecProtSetTxStrMaxLen (MSG *msg);
void SRecProtSetTxStrType (MSG *msg);
void SRecProtSetTxStrDelimiter (MSG *msg);
void SRecProtSetTxStrDelimiterMode (MSG *msg);
void SRecProtSetSwap (MSG *msg);
void SRecProtSetMode (MSG *msg);

// Bug21  Copied following defines from Legacy code

#define SETUP_CONFIG_MESSAGE \
	ConfigMsg.service=0x0e; \
	ConfigMsg.buflen=0;

#define SETUP_CONFIG_MESSAGE_1 \
	if(!(len--) || g_status)return;\
	ConfigMsg.service=0x10;ConfigMsg.buflen=1;\
	ConfigMsg.buf[0]=*ptr++;      // ILX-9

#define SETUP_CONFIG_MESSAGE_2 \
	if(!len || g_status)return;len=len-2;\
	ConfigMsg.service=0x10;ConfigMsg.buflen=2;\
	ConfigMsg.buf[0]=*ptr++;ConfigMsg.buf[1]=*ptr++;   // ILX-9

#define SETUP_CONFIG_MESSAGE_4 \
	if(!len || g_status)return;len=len-4;\
	ConfigMsg.service=0x10;ConfigMsg.buflen=4;\
	ConfigMsg.buf[0]=*ptr++;ConfigMsg.buf[1]=*ptr++;\
    ConfigMsg.buf[2]=*ptr++;ConfigMsg.buf[3]=*ptr++;

#define AFTER_CONFIG_MESSAGE_1 \
	if(g_status)return; \
	*ptr++=buf[0]; \
	len++;

#define AFTER_CONFIG_MESSAGE_2 \
	if(g_status)return; \
	*ptr++=buf[0]; \
	*ptr++=buf[1]; \
	len=len+2;

#define AFTER_CONFIG_MESSAGE_4 \
	if(g_status)return; \
	*ptr++=buf[0]; \
	*ptr++=buf[1]; \
	*ptr++=buf[2]; \
	*ptr++=buf[3]; \
	len=len+4;
// end bug 21


/**
 * @brief AssyConfigFunc() Function to get and set the assembly configurations.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void AssyConfigFunc (MSG *msg)
{
	MSG ConfigMsg;
	unsigned char *ptr = msg->buf;
	unsigned char len=msg->buflen;
	unsigned char buf[2],idx;
	ConfigMsg.buf = &buf[0];
	if((msg->attribute!=3) && (msg->attribute!=4))
	{
		g_status=0x14;
		 return;
	}
	// get the record number
	g_status=0;
	  if((msg->service==0x0e) && (msg->attribute==4))//  Rick_TEST Add the Get Attr 4
	  {
		  *(msg->buf) = 0x22;  //Rick_TEST this needs to be defined in a header file.
		  *(msg->buf+1) = 0;
		  msg->buflen=2;
		  return;
	  }
	  if(msg->service==0x0e)//get
	  {
		  len=6;
			for(idx=0;idx<len;idx++)
				*ptr++=0;//zero RA Reserved & Reserved1 bytes(4+2 bytes) ALSO points the ptr to the 1st valid config position (Framing)
	    ConfigMsg.buflen=len;
	    SETUP_CONFIG_MESSAGE   //Reserved DINT jtm 09/19/13 inserted 4 byte space as per new spec
		  AFTER_CONFIG_MESSAGE_4

	    SETUP_CONFIG_MESSAGE   //Reserved INT  jtm 09/19/13 inserted 2 byte space as per new spec
		  AFTER_CONFIG_MESSAGE_2

		  SETUP_CONFIG_MESSAGE
		  GetFraming(&ConfigMsg);       //Framing
		  AFTER_CONFIG_MESSAGE_1

		  SETUP_CONFIG_MESSAGE
		  MB_GetProtocol(&ConfigMsg);   //Protocol ASCII/RTU
		  AFTER_CONFIG_MESSAGE_1

		SETUP_CONFIG_MESSAGE
		  GetConsumeAssyNum(&ConfigMsg);//Consume Assembly number
		  AFTER_CONFIG_MESSAGE_1

		SETUP_CONFIG_MESSAGE
	  	  GetBaudRate(&ConfigMsg);      //Baudrate
		  AFTER_CONFIG_MESSAGE_1

	  	SETUP_CONFIG_MESSAGE
		  MB_GetType(&ConfigMsg);		    //Type Master/Slave
		  AFTER_CONFIG_MESSAGE_1

		SETUP_CONFIG_MESSAGE
	  	  GetProduceAssyNum(&ConfigMsg);//Produce Assembly number
		  AFTER_CONFIG_MESSAGE_1

	    SETUP_CONFIG_MESSAGE
		  MB_GetTimeout(&ConfigMsg);//Timeout 4 bytes allocated, but using 2 least significant bytes
		  AFTER_CONFIG_MESSAGE_4    //jtm 09-18-13 Changed to 4 bytes as per new spec

		SETUP_CONFIG_MESSAGE
		  MB_GetSlaveID(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2    //jtm 02-25-2013 changed from 1 byte to 2 bytes

	    SETUP_CONFIG_MESSAGE
		  MB_GetCoil_StartAddr(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetCoil_Count(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetDiscInput_StartAddr(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetDiscInput_Count(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetInReg_StartAddr(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetInReg_Count(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetHoldReg_StartAddr(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    SETUP_CONFIG_MESSAGE
		  MB_GetHoldReg_Count(&ConfigMsg);
		  AFTER_CONFIG_MESSAGE_2

	    msg->buflen=len;
	  }
	  else if(msg->service==0x10)//set
	  {
			// Rick_TEST Bug 32 make sure data is correct write size
			if(!DnCheckAttrLen(msg,34,34))return;

			if((msg->attribute==4))
			{
				g_status=0xe;
				 return;
			}

		    if(msg->buflen>34)
		    {
		      g_status=0x15;
		      return;
		    }
		    ConfigMsg.buflen=1;
			  msg->buflen=0;

		    SETUP_CONFIG_MESSAGE_4//Reserved DINT  jtm 09/19/13 inserted 4 byte space as per new spec
		    SETUP_CONFIG_MESSAGE_2//Reserved INT   jtm 09/19/13 inserted 2 byte space as per new spec

		    SETUP_CONFIG_MESSAGE_1
		    SetFraming(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_1
		    MB_SetProtocol(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_1
		    SetConsumeAssyNum(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_1
		    SetBaudRate(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_1
		    MB_SetType(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_1
		    SetProduceAssyNum(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_4    //Timeout 4 bytes allocated, but using 2 least significant bytes
		    MB_SetTimeout(&ConfigMsg);//jtm 09-18-13 Changed to 4 bytes as per new spec

		    SETUP_CONFIG_MESSAGE_2    //jtm 02-25-2013 changed from 1 byte to 2 bytes
		    MB_SetSlaveID(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetCoil_StartAddr(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetCoil_Count(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetDiscInput_StartAddr(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetDiscInput_Count(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetInReg_StartAddr(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetInReg_Count(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetHoldReg_StartAddr(&ConfigMsg);

		    SETUP_CONFIG_MESSAGE_2
		    MB_SetHoldReg_Count(&ConfigMsg);
	  }
	  InitSerialIO();
}

/**
 * @brief CheckMsgLen() Function to check the assembly message length with respect to high and low lengggth.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return bool 					- Return 1 success OR 0 fail
 *
 */
bool CheckMsgLen (MSG *msg, unsigned char lo, unsigned char hi)
{
	if (msg->buflen < lo)
	{
		g_status = 0x13;
		return 0;
	}
	if (msg->buflen > hi)
	{
		g_status = 0x15;
		return 0;
	}
	return 1;
}

/**
 * @brief AssyCheck() Function to check the assembly message type and retrun accordingly.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return unsigned charr 					- Return the type of assembly message type
 *
 */
unsigned char AssyCheck (MSG *msg)
{
	register unsigned char tmp;
	g_status = 0;
	if (msg->instance == 0)
	{
		g_status = SERVICE_NOT_SUPP;
	}
	else if ((msg->service != 0x0e) && (msg->service != 0x10))
	{
		g_status = SERVICE_NOT_SUPP;
	}
	else
		switch(msg->instance)   //Rick_TEST  add the other instances to selection.
		{
		case 101:     //  Produce Assemblies -- PLC Input
		case 105:
		case 107:
			tmp = 1;
			break;
		case 102:     //  Consume Assemblies -- PLC Output
		case 106:
		case 108:
			tmp = 2;
			break;
		case 103:
			tmp = 3;
			break;
		default:
			g_status = OBJECT_DOES_NOT_EXIST;
		}
	/*
	else if (msg->instance == ASSY_PINST) //  Rick_TEST swapped PINST & CINST
	{
		//if (msg->service != 0x0e)
		 //	g_status = SERVICE_NOT_SUPP;  // Rick_TEST Bug24 Legacy Module supports R & W To all instances.
		tmp = 1;
	}
	else if (msg->instance == ASSY_CINST)
	{
		tmp = 2;
	}
	else if (msg->instance == ASSY_CONFIGINST)
	{
		tmp = 3;
	}
	else
	{
		g_status = OBJECT_DOES_NOT_EXIST;
	}  */
	if (g_status)
		return 0;
	return tmp;
}

/**
 * @brief InitAssembly() Function to init the assembly function including getting EEPROM stored configurations.
 *
 *
 * @param void  							- None
 * @return void 							- None
 *
 */
void InitAssembly(void)
{
  /* DRC 2/12/2015 Replaced the above lines with the following */
  ConsumeAssyNum = Read_EE_Byte(EE_Consume_Path_Id);
  Ascii.ReceiveSize = Read_EE_Byte(EE_RECBUFFER_ADDR);
  ProduceAssyNum = Read_EE_Byte(EE_Produce_Path_Id);
  Ascii.TransmitSize = Read_EE_Byte(EE_XMITBUFFER_ADDR);
	/* End of changes */

  IOCnxnSize[CSI_P_CONS] = Ascii.ReceiveSize;
  IOCnxnSize[CSI_P_PROD] = Ascii.TransmitSize;
  IOCnxnSize[CSI_C_PROD] = Ascii.TransmitSize;

}

/**
 * @brief SetConsumeAssyNum() Function to set consume assembly receiver size based on assy config type.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void SetConsumeAssyNum(MSG * msg)
{
  if(!DnCheckAttrLen(msg,1,1))return;
  ConsumeAssyNum = msg->buf[0];
	switch(ConsumeAssyNum)
	{
		case 102:
			Ascii.ReceiveSize = 26;//10+16 bytes
		  break;

		case 106:
			Ascii.ReceiveSize = 58;//10+48 bytes
		  break;

		case 108:
			Ascii.ReceiveSize = 82;//10+72 bytes
		  break;

		default:
			g_status = INVALID_PARAMETER;
	}

	if ( g_status != INVALID_PARAMETER )
	{
		 Write_EE_Byte(EE_Consume_Path_Id, ConsumeAssyNum);
		 Write_EE_Byte(EE_RECBUFFER_ADDR, Ascii.ReceiveSize);
	}

  msg->buflen=0;
}

/**
 * @brief SetProduceAssyNum() Function to set produce assembly transmit size based on assy config type.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void SetProduceAssyNum(MSG * msg)
{
  if(!DnCheckAttrLen(msg,1,1))return;
  ProduceAssyNum = msg->buf[0];
	switch(ProduceAssyNum)
	{
		case 101:
			Ascii.TransmitSize = 30; //14+16 bytes
		  MaxRxSize = 16;
		  break;

    case 105:
  		Ascii.TransmitSize = 62;//14+48 bytes
		  MaxRxSize = 48;
		  break;

		case 107:
			Ascii.TransmitSize = 86;//14+72 bytes
		  MaxRxSize = 72;
		  break;

    default:
			g_status = INVALID_PARAMETER;
	}

	if ( g_status != INVALID_PARAMETER )
	{
     Write_EE_Byte(EE_Produce_Path_Id, ProduceAssyNum);
     Write_EE_Byte(EE_XMITBUFFER_ADDR, Ascii.TransmitSize);
  }

  msg->buflen=0;
}

/**
 * @brief CompAssyCSize() Function to get the consume message size.
 *
 *
 * @param void 		 						- None
 * @return unsigned char 		 			- value of the consume size
 *
 */
unsigned char CompAssyCSize (void)
{
	return ComputeIOConsumeSize();   // Rick_TEEST 8/3/2022
	//return TxStrLen + 4;
}

/**
 * @brief CompAssyPSize() Function to get the produce message size.
 *
 *
 * @param void 		 						- None
 * @return unsigned char 		 			- value of the produce size
 *
 */
unsigned char CompAssyPSize (void)
{
	return ComputeIOProduceSize();   // Rick_TEEST 8/3/2022
	//return 4 + RRecRxStrMaxLen;
}

/**
 * @brief AssemblyFill() Function to fill the assembly message.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void AssemblyFill (MSG *msg)
{
	// I have half a mind to pop crap off the stack
	// here so we return straight from the redirected fill
	// function
	if (AssyCheck (msg))
	{
		msg->attribute = 0xff;
		return;
	}
}

//void AssemblyObject (void)
void AssemblyInstPC(MSG *msg)
{

	  uchar Service=msg->service;
	  uchar Class=msg->class;
	  uchar Instance=msg->instance;
	  uchar Attribute=msg->attribute;


	  if(Instance == 0) g_status = SERVICE_NOT_SUPP;
		else if(msg->service == GET_REQ)
		{
		  switch(Instance)
			{
				case 101:
					switch(Attribute)
				  {
						case 1:
							g_status = ATTRIBUTE_NOT_SUPPORTED;
						  break;
						case 3:
	          	//xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
			    //    CurrFragObj.numbytes = GetAssySize(Instance)+2;
	            //FragMsg=FALSE;
							break;
						case 4:
					//		CurrFragObj.buffer[2] = GetAssySize(Instance);
					//	  CurrFragObj.buffer[3] = 0;
	            // CurrFragObj.numbytes = 4;
	            //FragMsg=FALSE;
							break;
						default: g_status = ATTRIBUTE_NOT_SUPPORTED;
					}
	        break;
			}
		}
}
#ifdef Rick_TEST_NO


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





  // uchar mbuf[50];
  uchar Service=CurrFragObj.buffer[INDX_SRVC];
  uchar Class=(CurrFragObj.buffer[INDX_CL_HI]<<8)+CurrFragObj.buffer[INDX_CL_LO];
  uchar Instance=CurrFragObj.buffer[INDX_INST];
  uchar Attribute=CurrFragObj.buffer[INDX_ATTR];
  //MSG xdata * msg;

	msg->buf = &mbuf[0];

	g_status=0;

  if(Instance == 0) g_status = SERVICE_NOT_SUPP;
	else if(Service == GET_REQ)
	{
	  switch(Instance)
		{
			case 101:
				switch(Attribute)
			  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
					case 3:
          	//xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            //FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
        break;

			case 102:
				switch(Attribute)
			  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
					case 3:
          	xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
        break;

			case 103:
				switch(Attribute)
			  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
					case 3:
    	  		msg->attribute=Attribute;
		      	msg->service=Service;
					  msg->buflen = GetAssySize(Instance);
            AssyConfigFunc(msg);
      	    xdata_memcpy(&CurrFragObj.buffer[2],msg->buf,msg->buflen-6);
		        CurrFragObj.numbytes = msg->buflen-4;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
			  break;

			case 105:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
  				case 3:
          	xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 106:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
  				case 3:
          	xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 107:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
  				case 3:
          	xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 108:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
  				case 3:
          	xdata_memcpy(&CurrFragObj.buffer[2],AssyInBuf,GetAssySize(Instance));
		        CurrFragObj.numbytes = GetAssySize(Instance)+2;
            FragMsg=FALSE;
						break;
					case 4:
						CurrFragObj.buffer[2] = GetAssySize(Instance);
					  CurrFragObj.buffer[3] = 0;
            CurrFragObj.numbytes = 4;
            FragMsg=FALSE;
						break;
					default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			default:
				g_status = OBJECT_DOES_NOT_EXIST;
  			break;
		}
	}
	else if(Service == SET_REQ)
	{
	  switch(Instance)
		{
			case 101:
 				switch(Attribute)
  		  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
						break;
	  			case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
				//		xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
        	//	Assembly_Produce(Prod_Out_Buf,GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
		  			break;
				  case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
			  }
			  break;

			case 102:
 				switch(Attribute)
  		  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
						break;
	  			case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
						xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
        		Assembly_Produce(Prod_Out_Buf,GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
		  			break;
				  case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
			  }
			  break;

			case 103:
 				switch(Attribute)
  		  {
					case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
						break;
	  			case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
    				msg->buflen=GetAssySize(Instance);
		    	  msg->attribute=Attribute;
			      msg->service=Service;
       	    xdata_memcpy(msg->buf,&CurrFragObj.buffer[INDX_DATA],GetAssySize(Instance));
            AssyConfigFunc(msg);
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
		  			break;
				  case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
			  }
				break;

			case 105:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
          case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
						xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
					  break;
          case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 106:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
          case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
						xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
        		Assembly_Produce(Prod_Out_Buf,GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
					  break;
          case 4:
						g_status = ATTR_NOT_SETTABLE;
            FragMsg=FALSE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 107:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
          case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
						xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
        		Assembly_Produce(Prod_Out_Buf,GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
					  break;
          case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			case 108:
        switch(Attribute)
			  {
			    case 1:
						g_status = ATTRIBUTE_NOT_SUPPORTED;
					  break;
          case 3:
						if(CurrFragObj.buffindx-6 > GetAssySize(Instance))
						{
							g_status = TOO_MUCH_DATA;
							break;
						}
						else if(CurrFragObj.buffindx-6 < GetAssySize(Instance))
						{
							g_status = NOT_ENOUGH_DATA;
							break;
						}
						xdata_memcpy(AssyOutBuf,&CurrFragObj.buffer[7],GetAssySize(Instance));
        		Assembly_Produce(Prod_Out_Buf,GetAssySize(Instance));
            MessageObjectFormatSuccessMessage();
            FragMsg=FALSE;
					  break;
          case 4:
						g_status = ATTR_NOT_SETTABLE;
					  break;
				  default: g_status = ATTRIBUTE_NOT_SUPPORTED;
				}
				break;

			default:
			  g_status = OBJECT_DOES_NOT_EXIST;
				break;
		}
	}
	else
	{
		g_status = SERVICE_NOT_SUPP;
	}
  if (g_status)
  {
    MessageObjectFormatErrorMessage(g_status, g_addCode);
    FragMsg=FALSE;
    return;
  }
}


#else
/**
 * @brief AssemblyFunc() Function to check assembly message and execute producer & consumer assembly message.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void *		 					- Pointer to message updated buffer
 *
 */
extern unsigned char TestBuf[];  //Rick_TEST Use teh teste buffer can delete when done.

void *AssemblyFunc (MSG *msg)
{
	unsigned char tmp;
	if (!g_status && (tmp = AssyCheck (msg)))
	{
		void *retval;
		if (tmp == 0x01)
			retval = AssyPFunc;
		else if (tmp == 0x03)
		{
			retval =  AssyConfigFunc;
		}
		else
			retval = AssyCFunc;

		if ((msg->attribute != 3) && (msg->attribute != 4))  // Rick_TEST needs to do both Data and Len Attributes.
					g_status = ATTRIBUTE_NOT_SUPP;

		else if ((msg->service == 0x10) && (msg->attribute == 4))
				g_status = ATTR_NOT_SETTABLE;
/* Bug32  the size chaeck is moved to the AssyFunc() (CheckMsgLen (msg, CompAssyCSize () - TxStrLen, CompAssyCSize ()))
				return retval;
		else if (!CheckMsgLen (msg, 0, 100))  // Rick_TEST arbitrarialy set to 100/  Not used in Legacy Module.
			; */
		else
			return retval;
	}
	return NULL;
}
/**
 * @brief GetConsumeAssyNum() Function to get the consume assembly number.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void GetConsumeAssyNum(MSG  * msg)
{
	msg->buf[0] = ConsumeAssyNum;
	msg->buflen++;
}
/**
 * @brief GetProduceAssyNum() Function to get the produce assembly number.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void GetProduceAssyNum(MSG * msg)
{
	msg->buf[0] = ProduceAssyNum;
	msg->buflen++;
}
/**
 * @brief AssyGetConsumeSize() Function to get the  consume assembly message size.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void AssyGetConsumeSize (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = CompAssyCSize ();
	msg->buflen = 1;
}
/**
 * @brief AssyGetProduceSize() Function to get the produce assembly message size.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void AssyGetProduceSize (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = CompAssyPSize ();
	msg->buflen = 1;
}

/**
 * @brief AssyGetComboStatus() Function to get the combo assembly status.
 *
 *
 * @param MSG * 							- Pointer to MSG struct data
 * @return void 		 					- None
 *
 */
void AssyGetComboStatus (MSG *msg)
{
	if (!DnCheckAttrLen (msg, 0, 0))
		return;
	msg->buf[0] = RRecStatus | TxSts;
	msg->buflen = 1;
}


#endif
