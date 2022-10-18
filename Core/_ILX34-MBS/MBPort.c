/**
 * @file MBPort.c
 * @author Jignesh
 * @date 25 Jul 2022
 * @brief This file contains MBPort modbus Master & slave role configurations and related functionalities.
 *
 */


#include <dn_cnobj.h>
#include <dn_dnobj.h>
#include <dn_eeprm.h>
#include <dn_eeprm.h>
#include <dn_init.h>
#include <dn_tmobj.h>
#include <dn_uiobj.h>
#include <fifo.h>
#include <memopt.h>
#include <msg.h>
#include <serial_config.h>
#include "mbport.h"

#include "ee_adr.h"
#include "io_peripheral.h"
#include "serial_hal.h"
#include "tim.h"

#include <stdio.h>
#include <xdatacpy.h>

/**
 * Common Macros
 */
#define FLT_SWAP_NONE        0
#define FLT_SWAP_WORD_PAIRS  1
#define FLT_SWAP_BYTES_WORDS 2
#define FLT_SWAP_BYTES       3

// Device Net Output (DNO) Assembly defines for location of fields
#define DNO_CONTROLBITS 0
#define DNO_RESERVED    1
#define DNO_TX_ID       2
#define DNO_STATION_ID  3
#define DNO_FUNC_   4
#define DNO_ADDR_L      5
#define DNO_ADDR_H      6
#define DNO_QUANT_L     7  // same as count lo in spec
#define DNO_QUANT_H     8  // same as count hi in spec
#define DNO_BYTE_COUNT  9
#define DNO_DATA_STRT  10

// Device Net Input (DNI) Assembly defines for location of fields
#define DNI_FAULT_STRT  0
#define DNI_MOD_STATUS  0
#define DNI_ERROR_  	1
#define DNI_RX_ID       2
#define DNI_STATION_ID  3
#define DNI_FUNC_   	4
#define DNI_ADDR_L      5
#define DNI_ADDR_H      6
#define DNI_QUANT_L     7  // same as count lo in spec
#define DNI_QUANT_H     8  // same as count hi in spec
#define DNI_BYTE_COUNT  9
#define DNI_DATA_STRT  	10
#define DNI_ERROR_CODE  11
#define DNI_FUNC_CODE 	12

// 9/11/2013 DRC - Added defines for MB Messages recieved in ilx slave mode
// these represent the location of information from the beginning, 0th location
// of the normalized message in the mb_normalized_rcv_buffer.
#define MB_STATION_ID     0  // location for command and response messages
#define MB_FUNC_LOC       1  // location for command and response messages
#define MBC_ADDR_HI_LOC   2  // location for command message Coil, Discrete, registers, or holding register Hi Address
#define MBC_ADDR_LO_LOC   3  // location for command message Coil, Discrete, registers, or holding register Lo Address
#define MBC_QUANT_HI_LOC  4  // location for command message Coil, Discrete, registers, or holding quantiy  Hi value
#define MBC_QUANT_LO_LOC  5  // location for command message Coil, Discrete, registers, or holding quantiy  Lo value
#define MBC_BYTE_CNT_LOC  6  // location for command message Byte Count for functions 15 and 16
#define MBC_DATA_STRT_LOC 7  // location for command message data start for functions 15 and 16
// end of MB defines added 9/11/2013 DRC

// 9/13/2013 DRC - Added defines for MB Messages recieved in ilx master mode
// these represent the location of information from the beginning, 0th location
// of the normalized message in the mb_normalized_rcv_buffer.
//#define MB_STATION_ID     0  // defined above since location of station id is the same for command and response messages
//#define MB_FUNC_LOC       1  // defined above since location of function id is the same for command and response messages
#define MBR_ERROR_LOC       2  // location for error response message error code
#define MBR_F1TO4_BYTE_CNT  2
#define MBR_F5_6_15_16_DATA 2
#define MBR_F1TO4_DATA_STRT 3
#define MBR_F5_6_15_16_LEN  6

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned int
#endif

#ifndef TX_FIFO_OVERFLOW
#define TX_FIFO_OVERFLOW 0x01
#endif

#ifndef RX_FIFO_OVERFLOW
#define RX_FIFO_OVERFLOW 0x02
#endif

///////////////////////////// Modbus stuff
#define FAULT_IDLE_STRING_SIZE      1
#define BUFFER_OVERFLOW_ERR_BIT		1
/* Modbus byte locations for Slave Mode input */
#define MB_SLAVEDNETSTAT	0
#define MB_SLAVEDNETTXID	1
#define MB_SLAVEDNETERR		2
#define MB_SLAVEDNETNODE	3
#define MB_SLAVEDNETFUNC	4
#define MB_SLAVEDNETADDR	5
#define MB_SLAVEDNETREG		7
#define MB_SLAVEDNETCOUNT	9

static uint32_t TimerVal = 0;
uchar NoByteSwap;
uchar FloatSwap;
unsigned char MaxRxSize = 120;  // Max N bytes of data in Assy
                                 // 10/24/2013 DRC - This now also means Max Byte Count (2*max Int data size of an assembly)
unsigned char MaxRxBufSize = 0;
unsigned char FragMsg=FALSE; //Jignesh TODO

static unsigned char input_txid;  // Rick_TEST changed to Static


unsigned char xmitDataLen;

unsigned char transaction_id =0;
unsigned char error_status;
unsigned char dest_addr; //=0;
unsigned char function_code; //=0;
unsigned char  produce_buffer[MAX_MODBUS_MESSAGE_SIZE];
unsigned char produce_buffer_len;
unsigned char mb_data_buffer_len;
unsigned char mb_data_buffer_temp_len;
unsigned char  mb_data_buffer[MB_DATA_BUFFER_SIZE];
unsigned char mb_normalized_rcv_buffer_len;
unsigned char  mb_normalized_rcv_buffer[MB_DATA_BUFFER_SIZE];
unsigned char  mb_data_buffer_temp[MB_DATA_BUFFER_SIZE];
unsigned char  mb_data_buffer_out[MB_DATA_BUFFER_SIZE];
unsigned char mb_data_buffer_out_len;
unsigned long mb_timeoutcounter;
unsigned char mb_messagesent;
unsigned long mb_timer;
unsigned char  MB_Status=0, MB_Exception=0;

int waiting=0;
int Transmitting=0;
int ProcessMbMessage=0;
MB_CONFIG	ModbusConfig;
unsigned char parityChkRslt;
unsigned int BaudDiv[8] = { BAUD19, BAUD12, BAUD24, BAUD48, BAUD96, BAUD38 };  // Rick_TEEST 8/3/2022
//unsigned int BaudDiv[8] = { BAUD96, BAUD12, BAUD24, BAUD48, BAUD19, BAUD38 };

BYTE recieve_status;//=0;
BYTE transmit_record_counter;//=0;
BYTE recieve_record_counter;//=0;
BYTE ser_predelim_buf_len;
char track=0;
unsigned int timeout;
unsigned int timeout_reload_value;
unsigned int Ascii_Mode_InterChar_Time = 0;        // millisec
unsigned char ASCII_Mode_InterChar_TO_flg = FALSE; // set to true whenever a timeout occurs
unsigned char ASCII_Mode_InterChar_TO_ON = FALSE;  // set to true when ASCII_MODE to turn on timer
unsigned char next_recieve_status; //=0;

extern unsigned char   num_bytes_in_buf;
//#define NO_MASTER  //debugging ONLY!
extern uchar P_OutMsgBuffer[];  // store IO data to be produced here
extern uchar P_InMsgBuffer[];
extern char			IOCnxnIsPOLLED; // Rick 403
extern unsigned char g_status;
extern unsigned char g_addCode;
extern unsigned char TxEmpty;
extern unsigned char TxInProgress;
extern unsigned char mainloopassydata[BYTES_OF_SER_DATA+6];
extern unsigned char size_of_mainloopassydata;
extern unsigned int fifo_used_mem;



void Mb_FactoryDefaults(void);
void CommandFlip(unsigned char *mb_msg);
void InitMbParam(void);
unsigned char MB_LoadProduceBuffer(unsigned char error);
int ComputeSlaveResponseLength(unsigned char  * src,unsigned char * l);
int ComputeMasterRecLength(unsigned char  * src,unsigned char * l);
unsigned char Check_For_Valid_MB_Msg(unsigned char *buf, unsigned char len);
void InitSerialIO(void);
void Serial_RX_ISR(void);
void Serial_TX_ISR(void);

extern void UIObjectLEDRefresh(void);
extern void MessageObjectSendPollMessage(void);
extern void TriggerCOS(void);
extern void MessageObjectFormatSuccessMessage(void);

MODBUS_ATTRIB ModAttrib = {
   0,
   ASCII_MODE,
   1,
};

ASCIISTRUCT Ascii = {
   3, //chnaged from 0 to 3 for testing MODBUS//Framing - 0=7N2, 1=7E1, 2=7O1, 3=8N1, 4=8N2, 5=8E1, 6=8O1
   7, //data bits
   0,	//baud rate index to BaudDiv[]
  10,	//delimiter
   0,	//parity 0=NONE / 1 = ODD / 2 = EVEN
   0,	//BlockSize
   0,	//Mode
   //                       0    //Framing
};
const DATA_PARITY MBport_DataParity[9] = {
	{ 7, NONE }, // 7N2
	{ 7, EVEN }, // 7E1
	{ 7, ODD },	 // 7O1
	{ 7, NONE }, // 8N1 - NOTE: needs the 8 bit mode
	{ 8, NONE }, // 8N2
	{ 8, EVEN }, // 8E1
	{ 8, ODD },	 // 8O1
	{ 8, EVEN }, // 7E2 - NOTE: needs the 9 bit mode
	{ 8, ODD },	 // 7O2 - NOTE: needs the 9 bit mode
};

/*------------------------------------------------*/
uchar CheckLimitParameters(unsigned char  *buf, unsigned char);
void InitRtuTimeout(void);


/**
 * @brief RestoreSerialFromEE() read serial framing (ex. 8N1, 7N2, etc) and baud rate value
 *        stored in EEPROM memory. It will call serila initialize fuction also.
 *
 *
 * @param void 					- none
 * @return void 				- none
 *
 */
void RestoreSerialFromEE (void)
{
	Ascii.Framing  = Read_EE_Byte (EE_SERIAL_CHARACTER_FORMAT);
	Ascii.BaudRate = Read_EE_Byte (EE_SERIAL_BAUDRATE); //@9600
	InitSerialIO ();
}

/**
 * @brief InitSerialIO() It will initialize serial configuration with various parameters
 *        eith respoect to MODBUS mode configuration.
 *
 *
 * @param void 					- none
 * @return void 				- none
 *
 */
void InitSerialIO(void)
{

   mb_timer = 0;
   mb_data_buffer_out_len = 0;
   error_status = 0;
   input_txid = 0;
   P_OutMsgBuffer[DNI_RX_ID] = input_txid; // Set to initialize Rx id to 0 on reset or power up - DRC 11/1/2013
   /* set for 1 sec */
   mb_timeoutcounter = ModbusConfig.timeout;
   mb_messagesent = 0;
   transaction_id =0;  //Rick_TEST added Bug43



   Ascii.DataBits = MBport_DataParity[Ascii.Framing].DataBits;
   Ascii.Parity = MBport_DataParity[Ascii.Framing].Parity;
   IO_SET_SerialTxRx (TxRx_RECV);    //Rick_TEST 8/3/2022
   // TxRx = TxRx_RECV;
   mb_data_buffer_len=0;			     //er-- experiment

   // Calculate the MaxRxBufSize
   if (ModbusConfig.type == MB_MASTERMODE) {
      if (ModAttrib.Mode == ASCII_MODE) {
         MaxRxBufSize = MaxRxSize * 2 + 11;
      }
      else { // ModAttrib.Mode == RTU_MODE
    	   MaxRxBufSize = MaxRxSize + 5;
      }
   }
   else { // ModbusConfig.type == MB_SLAVEMODE
      if (ModAttrib.Mode == ASCII_MODE) {
         MaxRxBufSize = MaxRxSize * 2 + 19;
      }
      else { // ModAttrib.Mode == RTU_MODE
         MaxRxBufSize = MaxRxSize + 9;
      }
   }
   // end of calculating MaxRxBufSize
   if(ModAttrib.Mode == RTU_MODE) InitRtuTimeout();
   MB_Status = READY_FOR_COMMAND;
   MB_Exception = 0;
	//
	// rs485 TX/RX to receive
	IO_SET_SerialTxRx (TxRx_RECV);
	//
	// Init our hardware.
	SH_Set_Parameters ();
	SH_Init ();

//Rick_TEST Bug27 this send a explicit produce before connection is made probabley bad	TriggerCOS(); //Bug5  TODO Jignesh to avoid zero to PLC data
}

/**
 * @brief XmitChar() Function to send the byte to UART function.
 *
 *
 * @param BYTE 					- Character byte
 * @return void 				- None
 *
 */
void XmitChar(BYTE chr)
{
	SH_Put_Char(chr);
}

//Application intterface functions needed for the group 2 allocate
// ILX-8  This is where the Procuce and Consume conneciton size are set
// This is size of PLC_Header plus the MB_TXRX_Size
/**
 * @brief ComputeIOConsumeSize() It will return the IO consume size.
 *
 *
 * @param void 						- none
 * @return unsigned char 			- Consume size
 *
 */
unsigned char ComputeIOConsumeSize(void)
{
   return (Ascii.ReceiveSize); //jtm 02-28-2013
}
/**
 * @brief ComputeIOProduceSize() It will return the IO produce size.
 *
 *
 * @param void 					-	- none
 * @return unsigned char 		   	- Produce size
 *
 */
unsigned char ComputeIOProduceSize(void)
{
   return (Ascii.TransmitSize); //jtm 02-28-2013
}

/**
 * @brief UpdateCRC() It will calculate the CRC value for new data.
 *
 *
 * @param BYTE 						- data
 * @param WORD 						- CRC value
 * @return WORD 					- final CRC value
 *
 */
WORD UpdateCRC(BYTE mydata,WORD crc)
{
   BYTE j;

   crc ^= mydata;
   for(j = 0; j < 8; j++)
   {
      if(crc & 1) crc = (crc >> 1)^0xA001;
      else crc >>= 1;
   }
   return crc;
}
/**
 * @brief bin_char() It will convert the ASCII characters to binary value.
 *
 *
 * @param BYTE 							- ASCII higher value
 * @param BYTE 							- ASCII lower value
 * @return BYTE 						- binary value
 *
 */
BYTE bin_char(BYTE char_hi,BYTE char_lo)  //Convert 2 ASCII characters to binary
{
   BYTE bin1, bin2;

   if(char_hi < 0x3A) bin1 = (BYTE)((char_hi - 0x30) << 4);
   else
   {
      char_hi &= 0xDF;                        //make upper case
      bin1 = (BYTE)((char_hi - 0x37) << 4);
   }
   if(char_lo < 0x3A) bin2 = (BYTE)(char_lo - 0x30);
   else
   {
      char_lo &= 0xDF;                        //make upper case
      bin2 = (BYTE)(char_lo - 0x37);
   }
   return(bin1 | bin2);
}
/**
 * @brief HexToBin() It will calculate and return hex value to bin value.
 *
 *
 * @param BYTE * 						- data pointer
 * @return BYTE 						- final CRC value
 *
 */
BYTE HexToBin(BYTE *Bytestr)
{
   return(bin_char(Bytestr[0],Bytestr[1]));
}

// Method below Normalizes the MB ASCII message received
// into a common binary message buffer and checks the lrc

/**
 * @brief NormalizeMBAsciiRcvMsg()  Method below Normalizes the MB ASCII message received
 * 									into a common binary message buffer and checks the lrc
 * @param unsigned char 		- MB_Exception
 * @param unsigned char * 		- Pointer to receive buffer
 * @param unsigned char 		- mb_rcv_buf_len
 * @param unsigned char * 		- mb_normalized_rcv_buf Normalized buffer
 * @param unsigned char * 		- mb_normalized_rcv_buf_len
 * @return unsigned char 		-  CRC error or success.
 *
 */
unsigned char NormalizeMBAsciiRcvMsg(unsigned char MB_Exception, unsigned char  * mb_rcv_buf, unsigned char mb_rcv_buf_len, unsigned char  * mb_normalized_rcv_buf, unsigned char * mb_normalized_rcv_buf_len)
{
   int i = 0;
   unsigned char LRC = 0;
   unsigned char bytestr[3] = {0,0,0};
   unsigned char byte = 0;

   *mb_normalized_rcv_buf_len = 0;

   for ( i = 1; i < (mb_rcv_buf_len - 4); i += 2 ) {
      bytestr[0] = mb_rcv_buf[i];
      bytestr[1] = mb_rcv_buf[i + 1];
      mb_normalized_rcv_buf[*mb_normalized_rcv_buf_len] = HexToBin(bytestr);
      LRC += mb_normalized_rcv_buf[*mb_normalized_rcv_buf_len];
      (*mb_normalized_rcv_buf_len)++;
   }
   LRC = (BYTE)-LRC;
   bytestr[0] = mb_rcv_buf[i];          //check LRC
   bytestr[1] = mb_rcv_buf[i+1];
   byte = HexToBin(bytestr);
   if (!MB_Exception)
      if ( LRC != byte ) {
         return CHECKSUM_ERROR;
      }
      else
      {
         return 0;
      }
   else {
      return MB_Exception;
   }
}

// 9/10/2013 DRC - method below to normalized the MB RTU message received
// into a common binary message buffer and checks the crc

/**
 * @brief NormalizeMB_RTU_RcvMsg()  method below to normalized the MB RTU message received
 *									into a common binary message buffer and checks the crc
 *
 *
 * @param unsigned char 		- MB_Exception
 * @param unsigned char * 		- Pointer to receive buffer
 * @param unsigned char 		- mb_rcv_buf_len
 * @param unsigned char * 		- mb_normalized_rcv_buf Normalized buffer
 * @param unsigned char * 		- mb_normalized_rcv_buf_len
 * @return unsigned char 		- return CRC error or success.
 *
 */
unsigned char NormalizeMB_RTU_RcvMsg(unsigned char MB_Exception, unsigned char  * mb_Rcv_Buf, unsigned char mb_Rcv_Buf_Len, unsigned char  * mb_normalized_rcv_buf, unsigned char * mb_normalized_rcv_buf_len)
{
   WORD crc = 0xFFFF;
   WORD recv_crc = 0x0000;
   int i = 0;

   *mb_normalized_rcv_buf_len = 0;

   for (i = 0; i < (mb_Rcv_Buf_Len - 2); i++) {
      crc = UpdateCRC(mb_Rcv_Buf[i],crc);
      mb_normalized_rcv_buf[*mb_normalized_rcv_buf_len] = mb_Rcv_Buf[i];
      (*mb_normalized_rcv_buf_len)++;
   }
   recv_crc = (mb_Rcv_Buf[i] + (mb_Rcv_Buf[i+1] << 8));
   if (!MB_Exception) {
      if (crc != recv_crc) {
        return CHECKSUM_ERROR;
      }
      else
      {
         return 0;
      }
   }
   else {
      return MB_Exception;
   }
}

/**
 * @brief mak_ASCII() Function below to convert binary nibble to ASCII character.
 *
 *
 * @param BYTE						- data nibble of byte
 * @return BYTE			 			- Return to ASCII value
 *
 */
BYTE mak_ASCII(BYTE datum) // Convert binary nibble to Hex ASCII character
{
   BYTE ASC_char;

   if(datum < 10)
   {
      ASC_char = (BYTE)(datum + 0x30);
   }
   else
   {
      ASC_char = (BYTE)(datum + 0x37);
   }
   return(ASC_char);
}

/**
 * @brief BinToHex() Function below to convert binary data to HEX value.
 *
 *
 * @param BYTE *					- Pointer to data array
 * @return void			 			- Returnil.
 *
 */
void BinToHex(BYTE Byte, BYTE *Bytestr)
{
   Bytestr[0] = mak_ASCII((BYTE)(Byte >> 4));
   Bytestr[1] = mak_ASCII((BYTE)(Byte & 0x0F));
}

/**
 * @brief ComputeCommandLength() Function below to calculate command size
 *
 *
 * @param unsigned char * src 		- Pointe to command
 * @param unsigned char * l 		- Pointer to command length
 * @return unsigned char 			- Return to success or fail.
 *
 */
int ComputeCommandLength(unsigned char  * src,unsigned char * l)
{
   switch(src[1])
   {
   case 1:
   case 2:
   case 3:
   case 4:
   case 5:
   case 6:
      *l=6;
      break;
   case 7:
      *l=2;
      break;
   case 8:
      *l=6;
      break;
   case 9:
   case 10:
      return 1;
   case 11:
   case 12:
      *l=2;
      break;
   case 13:
   case 14:
      return 1;
   case 15:
   case 16:
      *l=7+src[6];
      break;
   case 17:
      *l=2;
      break;
   case 18:
   case 19:
      return 1;
   case 20:
   case 21:
      *l=3+src[2];
      break;
   case 22:
      *l=8;
      break;
   case 23:
      *l=11+src[10];
      break;
   case 24:
      *l=4;
      break;
   default:
      return 1;
   }
   return 0;
}

/**
 * @brief ComputeSlaveResponseLength() Function below to calculate slave response length size
 *
 *
 * @param unsigned char * src 		- Pointe to command
 * @param unsigned char * l 		- Pointer to command length
 * @return unsigned char 			- Return to success or fail.
 *
 */
int ComputeSlaveResponseLength(unsigned char  * src,unsigned char * l)
{
   switch(src[1] & 0x7F) //ignore error bit
   {
   case 1:
   case 2:
   case 3:
   case 4:
      *l=7+src[6];
      break;
   case 5:
   case 6:
   case 15:
   case 16:
      *l=6;			    //  id, funccode, staddrH staddrL, QuantH, QuantL
      break;
   default:
      return 1;
   }
   return 0;
}

/**
 * @brief ComputeMasterRecLength() Function below to calculate Master response length size
 *
 *
 * @param unsigned char * src 		- Pointe to command
 * @param unsigned char * l 		- Pointer to command length
 * @return unsigned char 			- Return to success or fail.
 *
 */
int ComputeMasterRecLength(unsigned char  * src,unsigned char * l)
{
   switch(src[1])
   {
   case 1:
   case 2:
   case 3:
   case 4:
   case 5:
   case 6:
      *l=(6+src[2]);
      break;
   case 15:
   case 16:
      *l=(7+src[2]);
      break;

   default:
      return 1;
   }
   return 0;
}

/**
 * @brief Word_Flip() Function below to flip the word and uupdate the same buffer
 *
 *
 * @param unsigned char * src 		- Pointe to source word
 * @return void 					- None
 *
 */
void Word_Flip(unsigned char  * src)
{
   unsigned char tmpbyte;
   tmpbyte=(*src);
   (*src)=(*(src+1));
   (*(src+1))=tmpbyte;
}

//!!! mb_msg must be pointing at the modbus address !!!
/**
 * @brief CommandFlip() Function to flip the modbus address command using word flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void CommandFlip(unsigned char  *mb_msg)
{
   unsigned char i;//, i2;
   //unsigned char bytecnt;
   switch(*(mb_msg+1))
   {
   case 1:
   case 2:
   case 4:
   case 3:
   case 5:
   case 6:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   case 15:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);

      //bytecnt = *( mb_msg + 6 );  // get Byte Count
      break;
   case 16:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      i=7;
      while((*(mb_msg+6))+7>i)
      {
         Word_Flip(mb_msg+i);
         i+=2;
      }
      break;
   default:
      return;
   }
}

//!!! mb_msg must be pointing at the modbus address !!!
/**
 * @brief CommandFlipSlave() Function to flip the modbus slave address command using word flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void CommandFlipSlave(unsigned char  *mb_msg)
{
   unsigned char i;//, i2;
   //modbus command conversion......
   switch(*(mb_msg+1))
   {
   case 1:
   case 2:
   case 3:
   case 4:
   case 5:
   case 6:
      Word_Flip(mb_msg+6);
      Word_Flip(mb_msg+8);
      break;
   case 15:
      Word_Flip(mb_msg+6);
      Word_Flip(mb_msg+8);
      break;
   case 16:
      Word_Flip(mb_msg+6);
      Word_Flip(mb_msg+8);
      i=7;
      while((*(mb_msg+10))+7>i)
      {
         Word_Flip(mb_msg+i);
         i+=2;
      }
      break;
   default:
      return;
   }
}

/**
 * @brief ResponseFlipSlave() Function to flip the modbus slave response using word flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @param uchar 					- Flag to decide swap or no swap
 * @return void 					- None
 *
 */
void ResponseFlipSlave(unsigned char  * mb_msg, uchar noSwap)
{
   unsigned char i;
   switch(*(mb_msg+1))
   {
   case 3:
   case 4:
      mb_msg[2] = mb_msg[6];                // ILX-17  Move the byte count to right after func code.
      for (i=0; i< mb_msg[2];i+=2)          // ILX-17 Loop over byte count which is at msg[6]  accounting for skipped ADDR[2,3] And QAUNT[4,5]
      {
         if (!noSwap)
         {
            Word_Flip(mb_msg+7+i);
         }
      }
      break;
   case 5:
   case 6:
      if (noSwap) break;
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   case 12:
      if (noSwap) break;
      Word_Flip(mb_msg+3);
      Word_Flip(mb_msg+5);
      Word_Flip(mb_msg+7);
      break;
   case 15:
   case 16:
      if (noSwap) break;
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   default:
      return;
   }
}

//I need work still as well.
/**
 * @brief ResponseFlip() Function to flip the modbus response using word flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void ResponseFlip(unsigned char  * mb_msg)
{
   unsigned char i;
   switch(*(mb_msg+1))
   {
   case 3:
   case 4:
      for (i = 0; i < *(mb_msg+2); i += 2) { // for the number of words in Byte Count
         Word_Flip ( mb_msg + 3 + i );  // flip Words starting at word after Byte Count field
      }
      break;
   case 5:
   case 6:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   case 12:
      Word_Flip(mb_msg+3);
      Word_Flip(mb_msg+5);
      Word_Flip(mb_msg+7);
      break;
   case 15:
   case 16:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   default:
      return;
   }
}

/**
 * @brief ResponseMasterFlip() Function to flip the modbus master response using word flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void ResponseMasterFlip(unsigned char  * mb_msg)
{
   unsigned char i;
   switch(*(mb_msg+1))
   {
   case 3:
   case 4:
      i=7;
      while((*(mb_msg+6))+6>i)
      {
         Word_Flip(mb_msg+i);
         i+=2;
      }
      break;
   case 5:
   case 6:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   case 12:
      Word_Flip(mb_msg+3);
      Word_Flip(mb_msg+5);
      Word_Flip(mb_msg+7);
      break;
   case 15:
   case 16:
      Word_Flip(mb_msg+2);
      Word_Flip(mb_msg+4);
      break;
   default:
      return;
   }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/*************            Device net stuff             **********************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define SERVICE      explicit_msg_data[SERVICE_CODE_INDEX]
#define INSTANCE     explicit_msg_data[INSTANCE_INDEX]

/**
 * @brief Long_Flip() Function to flip pair of words bytes of the buffer.
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void Long_Flip(unsigned char  * ptr) // swap words pairs
{
   unsigned char tmpWrd[2];

   tmpWrd[0] = *ptr;
   tmpWrd[1] = *(ptr + 1);
   *ptr = *(ptr + 2);
   *(ptr + 1) = *(ptr + 3);
   *(ptr + 2) = tmpWrd[0];
   *(ptr + 3) = tmpWrd[1];
}

/**
 * @brief ByteWord_Flip() Function to flip the complete 4 bytes using word flip & long flip function.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void ByteWord_Flip(unsigned char  * src)
{
   Word_Flip(src);
   Word_Flip(src+2);
   Long_Flip(src);
}
/**
 * @brief Bytes_Flip() Function to flip the bytes using word flip.
 *
 *
 * @param unsigned char * src 		- Pointe to source message.
 * @return void 					- None
 *
 */
void Bytes_Flip(unsigned char  * src)
{
   Word_Flip(src);
   Word_Flip(src+2);
}

/**
 * @brief FltSwap_Command_Flip() Function to filter flip the modbus master & slave command.
 *
 *
 * @param unsigned char * msg 			- Pointer to source message.
 * @param unsigned char mmmmsg_len 		- Message length
 * @param unsigned char mmmmsg_len 		- mstr_slve_type value
 * @return unsigned char 				- Return 0 for success
 *
 */
unsigned char FltSwap_Command_Flip( unsigned char  * msg, unsigned char msg_len, unsigned char mstr_slve_type )
{
   unsigned char i;
   unsigned char func_loc;
   unsigned char byte_Cnt;
   unsigned int  intReg_Cnt;
   unsigned char reg_data_start;

   // setup function and data location depending on whether message is
   // from DN or normalized MB message.
   if ( mstr_slve_type == MB_MASTERMODE ) {
      func_loc = DNO_FUNC_ - DNO_STATION_ID;       // Subtract DNO_STATION_ID location because message pointer starts at station id
      byte_Cnt = DNO_BYTE_COUNT - DNO_STATION_ID;      // Subtract DNO_STATION_ID            "               "
      reg_data_start = DNO_DATA_STRT - DNO_STATION_ID; // Subtract DNO_STATION_ID            "               "
   }
   else { // mstr_slve_type = MB_SLAVEMODE
      func_loc = MB_FUNC_LOC;
      byte_Cnt = MBC_BYTE_CNT_LOC;
      reg_data_start = MBC_BYTE_CNT_LOC + 1;
   }

   switch ( msg [ func_loc ] ) {
      case  3:
         // check of slave func 3 command has the proper number of registers for word swap operations if needed
         if ( ( mstr_slve_type == MB_SLAVEMODE ) && ( ( FloatSwap == FLT_SWAP_WORD_PAIRS ) || ( FloatSwap == FLT_SWAP_BYTES_WORDS ) ) ) {
            intReg_Cnt = ( msg [ MBC_QUANT_HI_LOC ] << 8 ) + msg [ MBC_QUANT_LO_LOC ];
            if ( intReg_Cnt % 2 ) {
               return FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR;
            }
         }
         else if ( ( mstr_slve_type == MB_MASTERMODE ) && ( ( FloatSwap == FLT_SWAP_WORD_PAIRS ) || ( FloatSwap == FLT_SWAP_BYTES_WORDS ) ) ) {
            intReg_Cnt = ( msg [ DNO_QUANT_H ] << 8 ) + msg [ DNO_QUANT_L ];
            if ( intReg_Cnt % 2 ) {
               return FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR;
            }
         }
         break;

      case 16:

         // Test number of registers to make sure word swaps can take place
         if ( ( FloatSwap == FLT_SWAP_WORD_PAIRS ) || ( FloatSwap == FLT_SWAP_BYTES_WORDS ) ) {
            if ( ( msg [ byte_Cnt ]/2 ) % 2 ) {  // divide reg count by 2 to get reg count then determine if even or odd
               return FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR;
            }
         }

         for ( i = reg_data_start; i < msg_len; i += 4 ) {

            switch ( FloatSwap ) {

               case FLT_SWAP_WORD_PAIRS:
                  Long_Flip ( msg + i );
                  break;

               case FLT_SWAP_BYTES_WORDS:
                  ByteWord_Flip ( msg + i );
                  break;

               case FLT_SWAP_BYTES:
                  Bytes_Flip ( msg + i );
                  break;
            }
         }
         break;
   }

   return 0;
}
/**
 * @brief FltSwap_Response_Flip() Function to filter flip the modbus master & slave response.
 *
 *
 * @param unsigned char * msg 			- Pointer to source message.
 * @param unsigned char mmmmsg_len 		- Message length
 * @param unsigned char mmmmsg_len 		- mstr_slve_type value
 * @return unsigned char 				- Return 0 for success
 *
 */
unsigned char FltSwap_Response_Flip ( unsigned char  * msg, unsigned char msg_len, unsigned char mstr_slve_type )
{
   unsigned char i;
   unsigned char func_loc;
   unsigned char byte_Cnt;
   unsigned char reg_data_start;

   // setup function and data location depending on whether message is
   // from DN or normalized MB message.
   if ( mstr_slve_type == MB_MASTERMODE ) {
      func_loc = MB_FUNC_LOC;
      byte_Cnt = MBR_F1TO4_BYTE_CNT;
      reg_data_start = MBR_F1TO4_DATA_STRT;
   }
   else { // mstr_slve_type = MB_SLAVEMODE
      func_loc = DNO_FUNC_ - DNO_STATION_ID;       // Subtract DNO_STATION_ID location because message pointer starts at station id
      byte_Cnt = DNO_BYTE_COUNT - DNO_STATION_ID;      // Subtract DNO_STATION_ID location because message pointer starts at station id
      reg_data_start = DNO_DATA_STRT - DNO_STATION_ID; // Subtract DNO_STATION_ID location because message pointer starts at station id
   }

   switch ( msg [func_loc] ) {
      case 3:

         // Test number of registers to make sure word swaps can take place
         if ( ( FloatSwap == FLT_SWAP_WORD_PAIRS ) || ( FloatSwap == FLT_SWAP_BYTES_WORDS ) ) {
            if ( ( msg [ byte_Cnt ]/2 ) % 2 ) {
               if ( mstr_slve_type == MB_MASTERMODE ) {
                  return FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR;
               }
               else {
                  // if slave mode do nothing but return and let the message
                  // go through without trying to swap words
                  return 0;
               }
            }
         }

         for ( i = reg_data_start; i < msg_len; i += 4 ) {
            switch ( FloatSwap ) {
               case FLT_SWAP_WORD_PAIRS:
                 Long_Flip ( msg + i );
                 break;

               case FLT_SWAP_BYTES_WORDS:
                 ByteWord_Flip ( msg + i );
                 break;

               case FLT_SWAP_BYTES:
                 Bytes_Flip ( msg + i );
                 break;
            }
         }
         break;
   }
   return 0;
}

/**
 * @brief Start_Timeout() Initialized timeout value to start ASCII timer.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void Start_Timeout(void)
{
   timeout=timeout_reload_value;
}
/**
 * @brief StopTimeout() Functions to stop timer for ASCII timer.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void StopTimeout(void)
{
   timeout=0;
}

/********************* RTU TIMEOUT SYSTEM ************************/
BYTE TimerL,TimerH;
// 1200>0.029166667, 2400>0.014583333, 4800>0.007291667, 9600>0.003645833, 19200>0.001822917, 38400>0.000911458
//unsigned int BaudDiv[8] = { BAUD19, BAUD12, BAUD24, BAUD48, BAUD96, BAUD38 };
const unsigned int RTU_Timeout[6] = { 1823, 29166, 14583, 7292, 3646, 1750};
/**
 * @brief InitRtuTimeout() Initialized RTU timer with buad rate based timeout.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void InitRtuTimeout(void) //called from InitSerialIO()
{
	TimerVal = RTU_Timeout[Ascii.BaudRate];
	MX_TIM15_Init (TimerVal);
}

/**
 * @brief MB_StartRtuTimeout() Start a timer for RTU byte gets started receving.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void MB_StartRtuTimeout(void)// a 3.5 CHARACTER timeout
{
	MX_TIM15_Stop();
	MX_TIM15_Start();
}


/**
 * @brief MB_StopRtuTimeout() Stop a timer for R.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void MB_StopRtuTimeout(void)
{
	MX_TIM15_Stop();
}

/**
 * @brief MB_Rtu_TimedOut() Function gets called when RTU baud rate based timeout gets over.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void MB_Rtu_TimedOut(void) //interrupt 1  // using 2
{
   MX_TIM15_Stop();
   if (mb_data_buffer_len < 4)
   {
	   return;
   }
   if (ModbusConfig.type == MB_MASTERMODE) {
      MB_Status = PROCESSING_RESPONSE;
   }
   else { // Slave Mode
      MB_Status = PROCESSING_COMMAND;
   }
   // TriggerCOS();   //Rick_TEST Bug5 TODO Jignesh to avoid zero to PLC data
   ProcessMbMessage=1;  //process the message
}

/**
 * @brief MB_Strt_Ascii_Mde_IntChr_Timeout() Function to start timer when inter char starts receiving.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void MB_Strt_Ascii_Mde_IntChr_Timeout(void)
{
   Ascii_Mode_InterChar_Time = 0;
   ASCII_Mode_InterChar_TO_ON = TRUE;
}

/**
 * @brief MB_Stp_Ascii_Mde_IntChr_Timeout() Function to stop timer when inter char starts receiving.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void MB_Stp_Ascii_Mde_IntChr_Timeout(void)
{
   ASCII_Mode_InterChar_TO_ON = FALSE;
}

unsigned char  * mb_data_ptr;

/**
 * @brief Serial_TX_ISR() It will transmit the byte by byte until last byte on serial TX interrupt.
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void Serial_TX_ISR(void)
{
   //Jignesh TI=0;
   if(mb_data_buffer_out_len)
   {
      mb_data_buffer_out_len--;
      IO_SET_SerialTxRx (TxRx_XMIT); //set 485 chip to transmit Rick_TEST
      //  TxRx = TxRx_RECV;

      XmitChar(*(mb_data_ptr++));

   }
   else
   {
      Transmitting=0;
      mb_messagesent = 1;  //tell system that message has been sent
      mb_data_buffer_len = 0;//make sure the input buffer is ready to go
      if (ModbusConfig.type == MB_MASTERMODE) {
         // start wait timer to wait for MB response only in MB_MASTERMODE here
         waiting=1;           //tell system we are waiting for a response (timeout active)
         MB_Status = WAITING_FOR_RESPONSE;
      }
      else { // ModbusConfig.type == MB_SLAVEMODE
         // On successfull transmit, kill the wait timer to prevent an error response from
         // being sent back to the MB device
         waiting=0;  // tell the system we are not waiting for a response anymore.
         MB_Status = READY_FOR_COMMAND;
      }
      IO_SET_SerialTxRx (TxRx_RECV); //set 485 chip to receive Rick_TEST 8/3/2022
      // TxRx = TxRx_RECV;
      //we are done transmiting the message,
      if(!dest_addr) waiting=0;//broadcast message has no response
      else Start_Timeout();
      TriggerCOS(); //Bug5 TODO Jignesh to avoid zerto PLC
   }
}

#ifdef RICK_SIM
static unsigned char rtubyte=0;
static unsigned char rtucnt=0;
#define TEST_RTU
#endif


unsigned char StIn,CrIn;
unsigned char err;
char recChar;
unsigned char RiCount = 0;

/**
 * @brief Serial_RX_ISR() It will receive and store the byte by byte until MODBUS last byte
 * 			gets receive and flag gets set for success case or fail case.
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void Serial_RX_ISR(void)
{
   err = 0;

   // if the receive buffer is clear and ready to receive first character
   // reset MB_Exception to 0
   if ( ( ModAttrib.Mode != ASCII ) && ( !mb_data_buffer_len ) ) {
      MB_Exception = 0;
   }

   //recChar = RecvChar();
    SH_Get_Char_ISR(&recChar);


   if ( parityChkRslt == PARITY_ERROR ) {
      MB_Exception = PARITY_ERROR;
   }

   if ( Transmitting ||
        ProcessMbMessage ||
        ( ( ModbusConfig.type == MB_MASTERMODE )  && ( MB_Status != WAITING_FOR_RESPONSE ) )
      )
   {
      return;
   }
//   MB_StopRtuTimeout();
   // Test if number of bytes received is within buffer range, MaxRxBufSize
   if ( mb_data_buffer_len < MB_DATA_BUFFER_SIZE )
   {
      if(ModAttrib.Mode==ASCII)
      {
         MB_Stp_Ascii_Mde_IntChr_Timeout();

         switch(recChar)
         {
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
         case ':':
         case 0x0d:
         case 0x0a:
            err=0;
            break;

         default:
            err=1;
         }

         if(err)
         {
            if ( ( !MB_Exception ) && ( StIn == TRUE ) ) {
               // flag invalid messag only if start chr ':' has been received
               MB_Exception = MODBUS_INVALID_MESSAGE;
            }
            StIn = CrIn = FALSE;         //clear flags
            mb_data_buffer_len = 0;      //clear buffer
            //Jignesh RI=0;
            return;
         }

         if ( mb_data_buffer_len == 0 ) {
            // make sure StIn and CrIn flags are reset in case an inter char timeout occurred just before this char
            // was received.
            StIn = CrIn = FALSE;
            if ( recChar == ':' ) {
               StIn = TRUE;
               MB_Exception = 0;
            }
            else {
               // if first character received is not the ASCII start character then assume
               // junk is on the line and exit without flagging an error or anything
               //Jignesh RI=0;
               return;
            }
         }

         if(StIn==TRUE)
        	 mb_data_buffer[mb_data_buffer_len++]=recChar;
#ifdef RICK_SIM
         if(mb_data_buffer[mb_data_buffer_len-1]==0x0d) //for simulation
         {
            StIn = CrIn = FALSE;//clear flags
            if (ModbusConfig.type == MB_MASTERMODE)
            {
               MB_Status = PROCESSING_RESPONSE;
            }
            else
            { // Slave Mode
               MB_Status = PROCESSING_COMMAND;
            }
            ProcessMbMessage=1;
            RI=0;
            TriggerCOS;
            return;
         }

#else
         if ( ( StIn == TRUE ) && ( CrIn == FALSE ) && ( recChar == 0x0d ) )
         {
            CrIn = TRUE;
         }
         else if (( StIn==TRUE ) && ( CrIn==TRUE ))
         {
            if ( recChar == 0x0a )
            {
               StIn = CrIn = FALSE;//clear flags
               if (ModbusConfig.type == MB_MASTERMODE)
               {
                  MB_Status = PROCESSING_RESPONSE;
               }
               else
               {  // Slave Mode
                  MB_Status = PROCESSING_COMMAND;
               }
               ProcessMbMessage=1;
               //Jignesh RI=0;  new_produce_data_avail = 1;
               // TriggerCOS();  //Rick_TEST Bug5 TODO Jignesh to avoid zero to PLC data
               return;
            }
            else
            {
               // if the start chr and the carriage return has been received
               // then this character should be the line feed chr. Exit if it is not.
               if ( !MB_Exception ) {
                  MB_Exception = MODBUS_INVALID_MESSAGE;
               }
               StIn = CrIn = FALSE;         //clear flags
               mb_data_buffer_len = 0;      //clear buffer
               //Jignesh RI=0;
               return;
            }
         }
         MB_Strt_Ascii_Mde_IntChr_Timeout();
#endif
      }
      else
      { // RTU mode
         mb_data_buffer[mb_data_buffer_len++]=recChar;
#ifndef TEST_RTU
         MB_StartRtuTimeout();
#endif
      }
   }
   else
   {
      // Process what we have with a MB_Exception = INVALID_DATALENGTH_ERROR
      if(ModAttrib.Mode==ASCII) {
         // Make sure bogus LCR, CR, and LF is added to end of message so that it is processed
         // properly into the DN buffer
         if ( ( CrIn == FALSE ) && ( recChar!=0x0d) ) {
            // assume there is not an LCR in the message even though we are not sure
            mb_data_buffer[mb_data_buffer_len] = 'F';      // bogus LCR high character
            mb_data_buffer[mb_data_buffer_len + 1] = 'F';  // bogus LCR low character
            mb_data_buffer[mb_data_buffer_len + 2] = 0x0d; // CR
            mb_data_buffer[mb_data_buffer_len + 3] = 0x0a; // LF
            // increment mb_data_buffer_len appropriately
            mb_data_buffer_len += 5;

         }
         else if ( ( CrIn == FALSE ) && ( recChar == 0x0d ) ) {
            // LCR has bee received just add CR LF
            mb_data_buffer[mb_data_buffer_len] = 0x0d;
            mb_data_buffer[mb_data_buffer_len + 1] = 0x0a;
            // increment mb_data_buffer_len appropriately
            mb_data_buffer_len += 3;
         }
         else if ( ( CrIn == TRUE ) && ( recChar = 0x0a ) ) {
            // LCR and CR already received just add LF
            mb_data_buffer[mb_data_buffer_len] = 0x0a;
            // increment mb_data_buffer_len appropriately
            mb_data_buffer_len += 2;
         }
         StIn = CrIn = FALSE; //clear flags
      }
      else { // ModAttrib.Mode==RTU
         //
         // add bogus CRC at end but continue getting all bytes from the UART without
         // adding them to the buffer
         mb_data_buffer[mb_data_buffer_len] = 0xFF;
         mb_data_buffer[mb_data_buffer_len + 1] = 0xFF;
         // increment mb_data_buffer_len appropriately
         mb_data_buffer_len += 3;
//         MB_StartRtuTimeout();
     }
     if ( !MB_Exception ) {
        MB_Exception = INVALID_DATALENGTH_ERROR;
     }
     if (ModbusConfig.type == MB_MASTERMODE) {
        MB_Status = PROCESSING_RESPONSE;
     }
     else { // Slave Mode
        MB_Status = PROCESSING_COMMAND;
     }
     ProcessMbMessage=1;
     //TriggerCOS();  //Rick_TEST Bug5 TODO Jignesh to avoid zero to PLC data
   }
   //Jignesh RI=0;
}

/**
 * @brief MBLoad() It will parse and reverse the POLL request from DNET and create the MB PDU for
 * 					MODBUS mode configuration in mb_data_buffer_out. This is the message goes out
 * 					the UART as Modbus message on the wire. mainloopassydata  contains the reply
 * 					sent to the Producer for PLC Input consumption.
 *
 *
 * @param unsigned char  *src)  	- Pointer to source buffer
 * @return void 					- None
 *
 */
int MBLoad(unsigned char  *src)
{
   unsigned char srclen = 0;
   unsigned char  *to = mb_data_buffer_out;
   unsigned char byte;
   unsigned char indx = 0;
   //------------------------------------------------------------------------
   // src contains the DNET receved POLL and is pointing at the Station ID
   // so Parse DNET Poll into MB_PDU and calc the MB message length
   //------------------------------------------------------------------------

   if (ModbusConfig.type == MB_MASTERMODE)
   {
      if (ComputeCommandLength(src,&srclen))
      {
         MB_Exception = MODBUS_INVALID_MESSAGE;
         return 0;	//error, function is illegal code
      }

      if (NoByteSwap == FALSE)
      {
         CommandFlip(src);
      }

      // Check if Command 16 (hex 10) needs register data swapped
      if ( FloatSwap ) {
         MB_Exception = FltSwap_Command_Flip( src, srclen, ModbusConfig.type );
         if ( MB_Exception ) {
            mainloopassydata[DNI_ERROR_CODE] = MB_Exception;
            return 0;  // Return, If error do not continue to send message to MB device
         }
      }
   }
   else    // MB_SLAVEMODE
   {
      if (!(src[1] & 0x80)) { // do not do anything if this is an exception message

         if (ComputeSlaveResponseLength(src,&srclen))
         {
            MB_Exception = MODBUS_INVALID_MESSAGE;
            return 0;
         }

         ResponseFlipSlave(src,NoByteSwap);
         // Also Strips ADDR And QUANT words in func  3 & 4.

         // Check if Response to func 3 needs register data swapped
         if ( FloatSwap ) {
            MB_Exception = FltSwap_Response_Flip ( src, srclen, ModbusConfig.type );
            if ( MB_Exception ) {
               mainloopassydata[DNI_ERROR_CODE] = MB_Exception;
               return 0;  // Return, If error do not continue to send message to MB device
            }
         }
      }
      else
      {
         srclen = 3;
      }
   }
   //------------------------------------------------------------------------------------
   // srclen contains the length of the MB_PDU to send, add check and delimiters
   //   ie MB message to be built is  <START> {--MB_PDU--} <CHK_H><CHK_L><END>
   // Word values after func or byte count are swapped per configuration in above flip()
   // Just need to format and send the data based on ASCII or RTU selection
   //------------------------------------------------------------------------------------
   dest_addr=src[0];
   function_code=src[1];

   switch(ModAttrib.Mode)
   {
   case ASCII_MODE: //ASCII_MODE scope
      {
         unsigned char lrc = 0;
         unsigned char bytestr[3];

         bytestr[2] = 0;
         *(to++) = ':';
         for( indx = 0; indx < srclen; indx++ )
         {
            byte = (*(src++));
            // if statement skip pad bytes
            // in functions 1-4 in MB_SLAVEMODE

            if (0) //Jignesh ((ModbusConfig.type == MB_SLAVEMODE) && (function_ > 0) && (function_ < 5) && (indx > 1) && (indx < 6))
			{
               continue;
            }
            else
            {
               lrc += byte;
               BinToHex(byte,bytestr);
               (*(to++)) = bytestr[0];
               (*(to++)) = bytestr[1];
            }
         }

         lrc = (BYTE)-lrc;
         BinToHex(lrc,bytestr);
         (*(to++)) = bytestr[0];
         (*(to++)) = bytestr[1];
         (*(to++)) = '\r';                 //add termination
         (*(to++)) = '\n';

      } // end ASCII_MODE Scope
      break;

   case RTU_MODE:
      { //RTU_MODE Variable Scope
         WORD crc = 0xFFFF;
         for( indx = 0; indx < srclen; indx++ )
         {
            byte =(*(src++));
            if (0)//Jignesh ((ModbusConfig.type == MB_SLAVEMODE) && (function_ > 0) && (function_ < 5) && (indx > 1) && (indx < 6))
			{
               continue;
            }
            else
            {
               crc = UpdateCRC(byte,crc);
               (*(to++)) = byte;
            }
         }

         (*(to++)) = (BYTE) crc;
         (*(to++)) = (BYTE)(crc >> 8);
      } //End RTU_MODE Variable Scope
   } //end switch(ModAttrib.Mode)

   //length computation... a novel solution by ed.
   //the destination pointer less the origional value of the destination pointer must equal the length
   mb_data_buffer_out_len=(unsigned char)((unsigned int) ( ((unsigned int)to)-((unsigned int)mb_data_buffer_out) ));
   return 1;
}
/**
 * @brief StartMbSend() It will send the MODBUS data tp serial interface.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void StartMbSend(void)
{
   if(mb_data_buffer_out_len)
   {
      error_status=0; // on a new send, clear the error status
      mb_data_ptr=mb_data_buffer_out;
      mb_data_buffer_out_len--;
      Transmitting=1;
      IO_SET_SerialTxRx (TxRx_XMIT); //set 485 chip to transmit  Rick_TEST 8/3/2022
      // TxRx = TxRx_XMIT;
      XmitChar(*(mb_data_ptr++));
   }
   else Transmitting=0;
}

/**************************************************************************************************
**  MBM_QueMbTxMsg(MSG  *msg)
**
**  Takes the msg created when a POLL message containing OUTPUT ASSY 102 data is received
**  and queues a message to the serial interface if the transaction ID has changed
**  Called from:
**         2253.dn_msgobj>MessageObjectISR()
**	Returns void
**
**  Brute forced with P_InBuf for right now, should be passed a msg struct.
**  P_InBuf definitions:
**    0  Config_L
**    1  Config_H            if MBM_CFGHDR_BYTES is defined then add 2 to each below offest.
**    >>>>>>  Add any  to deal with configuration header then start processing at TX_ID
**    0  Tranaction ID       Need to put in message config word  consistiong 3 bits  Swap, Rst Ctrs, Cold Boot
**    1  MB_PDU Length
**    2  Station ID						Start of MB message must insert start mode Delim
**    3  Function							Start of MB_PDU
**    4  Addr_H
**    5  Addr_L
**    6  Quant_H
**    7  Quant_L
**    8  Optional data[0]
**    n  optional data[N]    End of MB_PDU  add CRC and end Delim to finish MB Message
**
**
**  3-3-13   Rick Ales
***************************************************************************************************/
/**
 * @brief WriteExceptionToQue() Function will add the exception to message to pass main loop data.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void WriteExceptionToQue(void)
{
   mainloopassydata[1] = MB_Exception;
}

/**
 * @brief MBM_QueMbTxMsg()
 *  Takes the msg created when a POLL message containing OUTPUT ASSY 102 data is received
 *  and queues a message to the serial interface if the transaction ID has changed
 *  Called from:
 *         2253.dn_msgobj>MessageObjectISR()
 *
 * @param Pointer *P_InBuf 			- Input Buffer to get the data
 * @return void 					- None
 *
 */
void MBM_QueMbTxMsg(unsigned char  *P_InBuf)
{
	 if(P_InBuf[MBM_CMDS] == 0xFF) goto TESTSKIP;
   //  Processing for ILX Command Byte   // ILX-13
   if(P_InBuf[MBM_CMDS] & MBM_CMDS_BOOT)   //Perform Cold Boot
   {

	   SoftReset = SOFT_RESET_ACTIVE; // Bug9
	   //TODO SoftReset = SOFT_RESET_ACTIVE;
   }
   if (P_InBuf[MBM_CMDS] & MBM_CMDS_RSTCTR)
   {
      // (ResetCounters);
   }
   if (P_InBuf[MBM_CMDS] & MBM_CMDS_SWAP)
   {
      NoByteSwap = TRUE; // this has global scope to swap the RX bytes
   }
   else
   {
      NoByteSwap = FALSE;
   }
TESTSKIP:
   switch (P_InBuf[MBM_CMDS] & MBM_FLOAT_SWAP)
   {
   case MBM_FLOAT_SWAP_WORD_PAIRS:
      FloatSwap = FLT_SWAP_WORD_PAIRS;
      break;

   case MBM_FLOAT_SWAP_BYTES_WORDS:
      FloatSwap = FLT_SWAP_BYTES_WORDS;
      break;

   case MBM_FLOAT_SWAP_BYTES:
      FloatSwap = FLT_SWAP_BYTES;
      break;

   default:
      FloatSwap = FLT_SWAP_NONE;
   }

   //  Process Configuration header here
#ifdef RICK_SIM
   if(1)
#else
   if((P_InBuf[DNO_TX_ID] != transaction_id) && !waiting && !Transmitting )
#endif
   {
      if ( ModbusConfig.type == MB_MASTERMODE ) {
         MB_Status = PROCESSING_COMMAND;
      }
      else { // MB_SLAVEMODE
         MB_Status = PROCESSING_RESPONSE;
      }
      //TriggerCOS(); //Bug5 Bug30 TODO Jignesh to avoid zerto PLC
      // reset MB_Exception to 0 every time a message comes in from DN
      MB_Exception = 0;


      if ( MBLoad( &P_InBuf[DNO_STATION_ID] ) ) //load the modbus buffer system, verify the buffer
      {
         StartMbSend();
         // note after message is transmitted the MB_Status is set to READY_FOR_COMMAND (SLAVEMODE)
         // or WAITING_FOR_RESPONSE (MASTERMODE) at the end of MB_Tx_Interrupt()
      }
      else { // MBLoad returned 0, an exception occurred
         // if an exception occurred always set MB_Status = READY_FOR_COMMAND
         MB_Status = READY_FOR_COMMAND;
      }

      if ( MB_Exception ) {
         // if by chance a 1 was returned but MB_Exception != 0 set MB_Status = READY_FOR_COMMAND
         MB_Status = READY_FOR_COMMAND;
      }

      transaction_id = P_InBuf[DNO_TX_ID];
     // Rick_TEST Bug43 TriggerCOS();   //Bug5 TODO Jignesh to avoid zero to PLC data
   }
   return;
}

/**
 * @brief Producing() Function to prepare the producing buffer.
 *
 *
 * @param MSG * 					- Pointer to message struct data
 * @return void 					- None
 *
 */
void Producing(MSG  *msg)
{
   unsigned char  * from=produce_buffer;
   unsigned char  * to=msg->buf;
   unsigned char i=produce_buffer_len;
   (*(to++))=transaction_id;
   (*(to))=(((unsigned char)(waiting))<<7);
   (*(to++))|=error_status ;
   (*(to++))=i;
   while(i)
   {
      (*(to++))=(*(from++));
      i--;
   }
   // i = our transmit size
   i=msg->buflen=ComputeIOProduceSize();
   //subtract what we have already added to the transmit buffer
   i-=(unsigned char) (((unsigned int)(to)) -((unsigned int)(msg->buf)));
   //pad i zeros
   while(i){
      (*(to++))=0;
      i--;
   }
}

/**
 * @brief MB_GetProtocol() Functions is used to get the protocol mode.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetProtocol(MSG  * msg)
{
   msg->buflen=1;
   msg->buf[0]=ModAttrib.Mode;
}

/**
 * @brief MB_SetProtocol() Functions is used to set the protocol mode in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetProtocol(MSG  * msg)
{
  if(!DnCheckAttrLen(msg,1,1))return;
  ModAttrib.Mode = msg->buf[0];
  EEPROMObjectWriteAndUpdate(EE_MODBUSMODE_ADDR,msg->buf[0]);
  msg->buflen=0;
}
/**
 * @brief GetReceiveSize() Functions is used to get the receive size.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void GetReceiveSize(MSG  * msg)
{
   if(msg->buflen>0) g_status=TOO_MUCH_DATA_2;
   msg->buflen=1;
   msg->buf[0]=Ascii.ReceiveSize;
}

/**
 * @brief GetTransmitSize() Functions is used to get the transmit size.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void GetTransmitSize(MSG  * msg)
{
   if(msg->buflen>0) g_status=TOO_MUCH_DATA_2;
   msg->buflen=1;
   msg->buf[0]=Ascii.TransmitSize;
}

/**
 * @brief SetTransmitSize() Functions is used to set the transmit size in EEPROM struct..
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void SetTransmitSize(MSG  * msg)
{
   if(msg->buf[0] > MAX_MODBUS_MESSAGE_SIZE)
   {
      g_status = INVALID_ATTRIB_VALUE;
      return;
   }
   Ascii.TransmitSize=msg->buf[0];  //jtm 02-27-2013
   EEPROMObjectWriteAndUpdate(EE_XMITBUFFER_ADDR,msg->buf[0]);
   //InitSerialIO();
   return;
}

/**
 * @brief SetReceiveSize() Functions is used to set the receive size in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void SetReceiveSize(MSG  * msg)
{
   if(msg->buf[0] > MAX_MODBUS_MESSAGE_SIZE)
   {
      g_status = INVALID_ATTRIB_VALUE;
      return;
   }
   Ascii.ReceiveSize=msg->buf[0];  //jtm 02-27-2013
   EEPROMObjectWriteAndUpdate(EE_RECBUFFER_ADDR,msg->buf[0]);
   //InitSerialIO();
   return;
}
/**
 * @brief GetFraming() Functions is used to get the framing of UART.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void GetFraming(MSG  * msg)
{
   if(msg->buflen>0) g_status=TOO_MUCH_DATA_2;
   msg->buflen=1;
   msg->buf[0]=Ascii.Framing;
}

/**
 * @brief SetFraming() Functions is used to set the framing of UART in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void SetFraming(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,1,1))
	 {
	   	 FragMsg=FALSE;
		 return;
	 }
   else if(msg->buf[0]>8)
   {
      g_status=INVALID_ATTRIBUTE_DATA;
      return;
   }
   Ascii.Framing = msg->buf[0];
   EEPROMObjectWriteAndUpdate(EE_SERIAL_CHARACTER_FORMAT,msg->buf[0]);
   msg->buflen=0;
}

/**
 * @brief GetBaudRate() Functions to get the baud rate of UART.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void GetBaudRate(MSG  * msg)
{
   if(msg->buflen>0) g_status=TOO_MUCH_DATA_2;
   msg->buflen = 1;
   msg->buf[0]= Ascii.BaudRate;
}

/**
 * @brief MB_GetType() Functions to get the type of modbus. .
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetType(MSG  * msg)
{
   msg->buflen=1;
   msg->buf[0]=ModbusConfig.type;
}

/**
 * @brief MB_GetTimeout() Functions to get the timeout of modbus.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetTimeout(MSG  * msg)
{
   msg->buflen=4;
   // note: PLC is little endian, LSB first.   c505 processor is big endian, MSB first
   msg->buf[0]=ModbusConfig.timeout & 0xFF;
   msg->buf[1]=ModbusConfig.timeout >> 8;
   msg->buf[2]=ModbusConfig.timeout >> 16;
   msg->buf[3]=ModbusConfig.timeout >> 24;
}

/**
 * @brief MB_GetSlaveID() Functions to get the slave ID.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetSlaveID(MSG  * msg)
{
   msg->buflen = 2;
   msg->buf[0] = ModbusConfig.slaveID;
   msg->buf[1] = 0;                    //jtm 02-27-2013  zero top byte (always 0)
}

/**
 * @brief MB_GetCoil_StartAddr() Functions to get the coil start address.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetCoil_StartAddr(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.Coil_StartAddr & 0xFF;
   msg->buf[1]=ModbusConfig.Coil_StartAddr >> 8;
}

/**
 * @brief MB_GetCoil_Count() Functions to get the coil count.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetCoil_Count(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.Coil_Count & 0xFF;
   msg->buf[1]=ModbusConfig.Coil_Count >> 8;
}

/**
 * @brief MB_GetDiscInput_Count() Functions to get the descrete input start address.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetDiscInput_StartAddr(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.DiscInput_StartAddr & 0xFF;
   msg->buf[1]=ModbusConfig.DiscInput_StartAddr >> 8;
}

/**
 * @brief MB_GetDiscInput_Count() Functions to get the descrete input count.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetDiscInput_Count(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.DiscInput_Count & 0xFF;
   msg->buf[1]=ModbusConfig.DiscInput_Count >> 8;
}

/**
 * @brief MB_GetInReg_StartAddr() Functions to get the input register start address.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetInReg_StartAddr(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.InReg_StartAddr & 0xFF;
   msg->buf[1]=ModbusConfig.InReg_StartAddr >> 8;
}

/**
 * @brief MB_GetInReg_Count() Functions to get the input register count.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetInReg_Count(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.InReg_Count & 0xFF;
   msg->buf[1]=ModbusConfig.InReg_Count >> 8;
}

/**
 * @brief MB_GetHoldReg_StartAddr() Functions to get the holding register start address.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetHoldReg_StartAddr(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.HoldReg_StartAddr & 0xFF;
   msg->buf[1]=ModbusConfig.HoldReg_StartAddr >> 8;
}

/**
 * @brief MB_GetHoldReg_Count() Functions to get the holding register count.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_GetHoldReg_Count(MSG  * msg)
{
   msg->buflen=2;
   msg->buf[0]=ModbusConfig.HoldReg_Count & 0xFF;
   msg->buf[1]=ModbusConfig.HoldReg_Count >> 8;
}

/**
 * @brief SetBaudRate() Functions to set the baud rate of UART in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void SetBaudRate(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,1,1))
	 {
	   FragMsg = FALSE;
	   return;
	 }

   if(msg->buf[0]>5)
   {
      g_status=INVALID_ATTRIBUTE_DATA;
      return;
   }
   Ascii.BaudRate = msg->buf[0];
   EEPROMObjectWriteAndUpdate(EE_SERIAL_BAUDRATE,msg->buf[0]);
   msg->buflen=0;
}

/**
 * @brief MB_SetType() Functions to set the type of modbus in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetType(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,1,1))
	 {
	   	 FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.type = msg->buf[0];
   EEPROMObjectWriteAndUpdate(MB_TYPE_NVRAM_ADDR,msg->buf[0]);
	msg->buflen=0;
}
/**
 * @brief MB_SetTimeout() Functions to set the timeout value in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetTimeout(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,4,4))
	 {
	     FragMsg=FALSE;
		 return;
	 }
   // note: PLC is little endian, LSB first.   c505 processor is big endian, MSB first
   ModbusConfig.timeout = (long) (msg->buf[0] + (msg->buf[1] << 8)+(msg->buf[2] << 16)+(msg->buf[3] << 24));
   msg->buf[3]=ModbusConfig.timeout >> 24;
   EEPROMObjectWriteAndUpdate(MB_TIMEOUT_NVRAM_ADDR,msg->buf[3]);
   msg->buf[2]=ModbusConfig.timeout >> 16;
   EEPROMObjectWriteAndUpdate(MB_TIMEOUT_NVRAM_ADDR,msg->buf[2]);
   msg->buf[1]=ModbusConfig.timeout >> 8;
   EEPROMObjectWriteAndUpdate(MB_TIMEOUT_NVRAM_ADDR,msg->buf[1]);    // note little endian PLC data stored as big endian, PLC MSB stored first
   EEPROMObjectWriteAndUpdate(MB_TIMEOUT_NVRAM_ADDR+1,msg->buf[0]);  // PLC LSB stored last in memory
	msg->buflen=0;
   MessageObjectFormatSuccessMessage();
   FragMsg=FALSE;
}
/**
 * @brief MB_SetSlaveID() Functions to set the slave ID of UART in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetSlaveID(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   	  FragMsg=FALSE;
		 return;
	 }
   //MSB
   ModbusConfig.slaveID = (unsigned int) msg->buf[0];
   EEPROMObjectWriteAndUpdate(MB_SLAVEID_NVRAM_ADDR,0);  //jtm 02-27-2013 added for 2 byte SlaveId (upper byte always 0)
   //LSB //TODO for 2 bytes
   EEPROMObjectWriteAndUpdate(MB_SLAVEID_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}
/**
 * @brief MB_SetCoil_StartAddr() Functions to set the coil start address in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetCoil_StartAddr(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	    FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.Coil_StartAddr = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_COILSTART_NVRAM_ADDR,msg->buf[1]);
   //LSB //TODO 2 bytes in memory
   EEPROMObjectWriteAndUpdate(MB_COILSTART_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}
/**
 * @brief MB_SetCoil_Count() Functions to set the coil count in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetCoil_Count(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	    FragMsg=FALSE;
		 return;
	 }

   ModbusConfig.Coil_Count = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_COILCOUNT_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_COILCOUNT_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}
/**
 * @brief MB_SetDiscInput_StartAddr() Functions to set the descrete input register start addressss in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetDiscInput_StartAddr(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.DiscInput_StartAddr = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_DISCINPUTSTART_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_DISCINPUTSTART_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}
/**
 * @brief MB_SetDiscInput_Count() Functions to set the descrete input count in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetDiscInput_Count(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   	 FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.DiscInput_Count = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_DISCINPUTCOUNT_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_DISCINPUTCOUNT_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}

/**
 * @brief MB_SetInReg_StartAddr() Functions to set the Input register start address in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetInReg_StartAddr(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   	 FragMsg=FALSE;
		 return;
	 }

   ModbusConfig.InReg_StartAddr = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_INREGSTART_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_INREGSTART_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}

/**
 * @brief MB_SetInReg_Count() Functions is used to set the input register count in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetInReg_Count(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   	   FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.InReg_Count = (msg->buf[0] +( msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_INREGCOUNT_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_INREGCOUNT_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}

/**
 * @brief MB_SetHoldReg_StartAddr() Functions is used to set the holding register start address in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetHoldReg_StartAddr(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	   	  FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.HoldReg_StartAddr = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_HOLDREGSTART_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_HOLDREGSTART_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}

/**
 * @brief MB_SetHoldReg_Count() Functions is used to set the holding register count value in EEPROM struct.
 *
 *
 * @param MSG * 					- Pointer to MSG struct data
 * @return void 					- None
 *
 */
void MB_SetHoldReg_Count(MSG  * msg)
{
   if(!DnCheckAttrLen(msg,2,2))
	 {
	    FragMsg=FALSE;
		 return;
	 }
   ModbusConfig.HoldReg_Count = (msg->buf[0] + (msg->buf[1] << 8));
   //MSB
   EEPROMObjectWriteAndUpdate(MB_HOLDREGCOUNT_NVRAM_ADDR,msg->buf[1]);
   //LSB
   EEPROMObjectWriteAndUpdate(MB_HOLDREGCOUNT_NVRAM_ADDR+1,msg->buf[0]);
   //EEPRFragMsgOMObjectWriteAndUpdate(MB_HOLDREGCOUNT_NVRAM_ADDR+1,msg->buf[0]);
	msg->buflen=0;
}


/**
 * @brief Mb_FactoryDefaults() Function will write the device EEPROM with default data when ever PLC requested it.
 *
 *
 * @param void 						- None
 * @return void 					- None
 *
 */
void Mb_FactoryDefaults(void)
{
   // default them and save to EEPROM
   ModAttrib.Mode = 0; 			  //ASCII mode
   Ascii.Framing = 0;		// 7 N 2
   Ascii.BaudRate = 0; 	//19200
   timeout_reload_value = 0;
   Ascii.ReceiveSize = 26;
   Ascii.TransmitSize = 30;
   DeviceNetObjectRAM.baudrate = 3;   //DRC 3/3/2015 set from 0 to 3 to match cstparam.c custParamInit()
   ModbusConfig.type =  MB_MASTERMODE;
   ModbusConfig.timeout = 2000;
   ModbusConfig.Coil_StartAddr = 0;
   ModbusConfig.Coil_Count = 0;
   ModbusConfig.DiscInput_StartAddr = 0;
   ModbusConfig.DiscInput_Count = 0;
   ModbusConfig.InReg_StartAddr = 0;
   ModbusConfig.InReg_Count = 0;
   ModbusConfig.HoldReg_StartAddr = 0;
   ModbusConfig.HoldReg_Count = 0;
   ModbusConfig.slaveID = 0;
   /* Now save to EEPROM */
   Write_EE_Byte(EE_SERIAL_CHARACTER_FORMAT, Ascii.Framing);

   Write_EE_Byte(EE_SERIAL_BAUDRATE, Ascii.BaudRate);
   Write_EE_Byte(EE_MODBUSMODE_ADDR, ModAttrib.Mode);
   //MSB
   Write_EE_Byte(EE_TIMEOUT_LOW_ADDR,timeout_reload_value & 0xFF);
   //LSB
   Write_EE_Byte(EE_TIMEOUT_HI_ADDR,timeout_reload_value >> 8);
   //	Write_EE_Byte(EE_FAULTACT_ADDR, FaultAction);
   //	Write_EE_Byte(EE_IDLEACT_ADDR, IdleAction);
   Write_EE_Byte(EE_XMITBUFFER_ADDR, Ascii.TransmitSize);
   Write_EE_Byte(EE_RECBUFFER_ADDR, Ascii.ReceiveSize);
   Write_EE_Byte(EE_DNETBAUD_ADDR, DeviceNetObjectRAM.baudrate);  // DRC 3/4/2015 should this be NVS_BAUD_RATE ?
   Write_EE_Byte(MB_TYPE_NVRAM_ADDR, ModbusConfig.type);
   Write_EE_Byte(MB_SLAVEID_NVRAM_ADDR, 0);                       // DRC 4/6/2015 MSB should always be 0 it only uses LSB
   Write_EE_Byte(MB_SLAVEID_NVRAM_ADDR+1, ModbusConfig.slaveID);  // DRC 4/6/2015 LSB is active that is set to default value
   //Jignesh WDOG_PIN ^= 1;

   //MSB first
   Write_EE_Byte( MB_TIMEOUT_NVRAM_ADDR, ( ModbusConfig.timeout >> 8 ) );
   //LSB last
   Write_EE_Byte( ( MB_TIMEOUT_NVRAM_ADDR + 1 ), ( ModbusConfig.timeout & 0x00FF ) );

   //MSB first
   Write_EE_Byte(MB_COILSTART_NVRAM_ADDR,ModbusConfig.Coil_StartAddr >> 8);
   //LSB last
   Write_EE_Byte(MB_COILSTART_NVRAM_ADDR+1,ModbusConfig.Coil_StartAddr & 0xFF);

   //MSB first
   Write_EE_Byte(MB_COILCOUNT_NVRAM_ADDR,ModbusConfig.Coil_Count >> 8);
   //LSB last
   Write_EE_Byte(MB_COILCOUNT_NVRAM_ADDR+1,ModbusConfig.Coil_Count & 0xFF);

   //MSB first
   Write_EE_Byte(MB_DISCINPUTSTART_NVRAM_ADDR,ModbusConfig.DiscInput_StartAddr >> 8);
   //LSB last
   Write_EE_Byte(MB_DISCINPUTSTART_NVRAM_ADDR+1,ModbusConfig.DiscInput_StartAddr & 0xFF);

   //MSB first
   Write_EE_Byte(MB_DISCINPUTCOUNT_NVRAM_ADDR,ModbusConfig.DiscInput_Count >> 8);
   //LSB last
   Write_EE_Byte(MB_DISCINPUTCOUNT_NVRAM_ADDR+1,ModbusConfig.DiscInput_Count & 0xFF);

   //MSB first
   Write_EE_Byte(MB_INREGSTART_NVRAM_ADDR,ModbusConfig.InReg_StartAddr >> 8);
   //LSB last
   Write_EE_Byte(MB_INREGSTART_NVRAM_ADDR+1,ModbusConfig.InReg_StartAddr & 0xFF);

   //MSB first
   Write_EE_Byte(MB_INREGCOUNT_NVRAM_ADDR,ModbusConfig.InReg_Count >> 8);
   //LSB last
   Write_EE_Byte(MB_INREGCOUNT_NVRAM_ADDR+1,ModbusConfig.InReg_Count & 0xFF);

   //MSB first
   Write_EE_Byte(MB_HOLDREGSTART_NVRAM_ADDR,ModbusConfig.HoldReg_StartAddr >> 8);
   //LSB last
   Write_EE_Byte(MB_HOLDREGSTART_NVRAM_ADDR+1,ModbusConfig.HoldReg_StartAddr & 0xFF);

   //MSB first
   Write_EE_Byte(MB_HOLDREGCOUNT_NVRAM_ADDR,ModbusConfig.HoldReg_Count >> 8);
   //LSB last
   Write_EE_Byte(MB_HOLDREGCOUNT_NVRAM_ADDR+1,ModbusConfig.HoldReg_Count & 0xFF);

   Write_EE_Byte(EE_Produce_Path_Id,0x65);  // decimal 101
   Write_EE_Byte(EE_Consume_Path_Id,0x66);  // decimal 102

}

/**
 * @brief InitMbParam() Functions to initialize modbus parameters by reading stored from EEPROM.
 *
 *
 * @return void 					- None
 * @return void 					- None
 *
 */
void InitMbParam(void)
{
	if (1)
	{
	   ModAttrib.Mode = Read_EE_Byte(EE_MODBUSMODE_ADDR); // Bug10
	   Ascii.Framing =  Read_EE_Byte(EE_SERIAL_CHARACTER_FORMAT);  //Bug10
	   Ascii.BaudRate = Read_EE_Byte(EE_SERIAL_BAUDRATE);

//	   Ascii.BaudRate = 2; // TODO to check the 2400 Baud rate value
//     Ascii.Framing = 3; // TODO hard coded for value 3 means 8N1 for UART framing setting.
//     ModAttrib.Mode = RTU_MODE; //TODO make it enable to test with RTU. Also to make RTU working make Framing 3 (8N1)

       timeout_reload_value  = Read_EE_Byte(EE_TIMEOUT_HI_ADDR);
	   timeout_reload_value = timeout_reload_value << 8;
	   timeout_reload_value += Read_EE_Byte(EE_TIMEOUT_LOW_ADDR);

	   Ascii.ReceiveSize= Read_EE_Byte(EE_RECBUFFER_ADDR);
	   Ascii.TransmitSize= Read_EE_Byte(EE_XMITBUFFER_ADDR);

	   ModbusConfig.type = Read_EE_Byte(MB_TYPE_NVRAM_ADDR);

	   // Get MSB first
	   ModbusConfig.timeout  = Read_EE_Byte(MB_TIMEOUT_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.timeout = ModbusConfig.timeout << 8;
	   // Add LSB
	   ModbusConfig.timeout += Read_EE_Byte(MB_TIMEOUT_NVRAM_ADDR+1);

	   // DRC 3/12/2015 make sure slave is retreived from LSB in memory "MB_SLAVEID_NVRAM + 1"
	   ModbusConfig.slaveID = Read_EE_Byte(MB_SLAVEID_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.Coil_StartAddr  = Read_EE_Byte(MB_COILSTART_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.Coil_StartAddr = ModbusConfig.Coil_StartAddr << 8;
	   // Add LSB
	   ModbusConfig.Coil_StartAddr += Read_EE_Byte(MB_COILSTART_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.Coil_Count  = Read_EE_Byte(MB_COILCOUNT_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.Coil_Count = ModbusConfig.Coil_Count << 8;
	   // Add LSB
	   ModbusConfig.Coil_Count += Read_EE_Byte(MB_COILCOUNT_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.DiscInput_StartAddr  = Read_EE_Byte(MB_DISCINPUTSTART_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.DiscInput_StartAddr = ModbusConfig.DiscInput_StartAddr << 8;
	   // Add LSB
	   ModbusConfig.DiscInput_StartAddr += Read_EE_Byte(MB_DISCINPUTSTART_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.DiscInput_Count  = Read_EE_Byte(MB_DISCINPUTCOUNT_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.DiscInput_Count = ModbusConfig.DiscInput_Count << 8;
	   ModbusConfig.DiscInput_Count += Read_EE_Byte(MB_DISCINPUTCOUNT_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.InReg_StartAddr  = Read_EE_Byte(MB_INREGSTART_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.InReg_StartAddr = ModbusConfig.InReg_StartAddr << 8;
	   // Add LSB
	   ModbusConfig.InReg_StartAddr += Read_EE_Byte(MB_INREGSTART_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.InReg_Count  = Read_EE_Byte(MB_INREGCOUNT_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.InReg_Count = ModbusConfig.InReg_Count << 8;
	   // Add LSB
	   ModbusConfig.InReg_Count += Read_EE_Byte(MB_INREGCOUNT_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.HoldReg_StartAddr  = Read_EE_Byte(MB_HOLDREGSTART_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.HoldReg_StartAddr = ModbusConfig.HoldReg_StartAddr << 8;
	   // Add LSB
	   ModbusConfig.HoldReg_StartAddr += Read_EE_Byte(MB_HOLDREGSTART_NVRAM_ADDR+1);

	   // Get MSB first
	   ModbusConfig.HoldReg_Count  = Read_EE_Byte(MB_HOLDREGCOUNT_NVRAM_ADDR);
	   // Make it the Hi byte
	   ModbusConfig.HoldReg_Count = ModbusConfig.HoldReg_Count << 8;
	   // Add LSB
	   ModbusConfig.HoldReg_Count += Read_EE_Byte(MB_HOLDREGCOUNT_NVRAM_ADDR+1);

	}

	InitAssembly();
	InitSerialIO ();
	 // DRC 2/19/2015 Added to bypass call to AssyConfigFunc that was taken
	 // out of the InitAssembly() routine.
   // DRC 3/10/2015 took out so MB buadrate change takes effect after reset as v1.13 does InitSerialIO();
}

/**
 * @brief main_port_serial() Functions to process the response of MODBUS slaves and Master frame to connected devices.
 *
 *
 * @return void 					- None
 * @return void 					- None
 *
 */
void main_port_serial (void)
{
   unsigned char  *to=mb_data_buffer_out;
   unsigned char byte;
   //BYTE inidx,outidx,idx;
   BYTE idx;
   //BYTE lrc;
   //BYTE bytestr[3] ={0};
   WORD crc;
   unsigned char checksum_Status = 0;

   if ( (MB_Exception == MODBUS_TIMEOUT_ERROR) && ( waiting ) )
   {
      // A timeout occurred.

      // If in MB_MASTERMODE a message was transmitted to the MB device and the timeout
      // occurred while waiting for a response from the MB device.
      if (ModbusConfig.type == MB_MASTERMODE) {
         mainloopassydata[DNI_ERROR_CODE] = MB_Exception;
      }
      // If in MB_SLAVEMODE a message was received and successfully processed into the
      // DN buffer and a timeout occurred while waiting for a response from the DN PLC.
      else { // MB_SLAVEMODE
         // build an error response message to be sent back to the MB device
         if (ModAttrib.Mode == ASCII_MODE) {
            // build an ascii error response message
            mb_data_buffer_temp[MB_STATION_ID] = mb_normalized_rcv_buffer[MB_STATION_ID];
            mb_data_buffer_temp[MB_FUNC_LOC] = (0x80 | mb_normalized_rcv_buffer[MB_FUNC_LOC]);

            mb_data_buffer_temp[MBR_ERROR_LOC] = GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND;
            mb_data_buffer_temp_len = 3;
            // byte count to high send error
            if(MBLoad(mb_data_buffer_temp)) //load the modbus buffer system, verify the buffer
            {
               StartMbSend();
            }
         }
         else { // RTU_MODE
            // build an RTU error response message
            mb_data_buffer_temp[0] = mb_data_buffer[0];
            mb_data_buffer_temp[1] = 0x80 | mb_data_buffer[1];
            mb_data_buffer_temp[2] = GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND;
            mb_data_buffer_temp_len = 3;
            crc = 0xFFFF;
            // calculate crc and load mb_data_buffer_temp into mb_data_buffer_out
            for( idx=0; idx < mb_data_buffer_temp_len; idx++ )       //starting @ zero
            {
               byte = mb_data_buffer_temp[idx]; //get byte
               crc = UpdateCRC(byte,crc);		   //do crc
               (*(to++)) = byte;	               //plop
            }
            (*(to++)) = (BYTE) crc;
            (*(to++)) = (BYTE)(crc >> 8);
            mb_data_buffer_out_len = 5;
            StartMbSend();
         }
      }
      waiting = 0;
      MB_Status = READY_FOR_COMMAND;
      TriggerCOS(); //Bug5 TODO Jignesh to avoid zerto PLC
   }

   if ( ( ModAttrib.Mode == ASCII_MODE ) && ( ASCII_Mode_InterChar_TO_flg ) ) {
      // timeout occurred since the last ASCII character received and a the new expected character.
      MB_Stp_Ascii_Mde_IntChr_Timeout();
      ASCII_Mode_InterChar_TO_flg = FALSE;
      MB_Exception = MODBUS_INVALID_MESSAGE;
      if ( mb_data_buffer_len > 4 ) { // if id and function  or just LRC is present then try to process the message
         ProcessMbMessage = 1;
      }
      else {
         // just drop the message and return
         mb_data_buffer_len = 0;
         if ( ModbusConfig.type == MB_SLAVEMODE ) {
            MB_Status = READY_FOR_COMMAND;
         }
         TriggerCOS(); //Bug5 TODO Jignesh to avoid zerto PLC
      }
   }

   if(ProcessMbMessage)  // ProcessMbMessage is set only when a MB message is received
   {
      //bytestr[2] = 0;
      //inidx = lrc = 0;
      //lrc = 0;
      //outidx = inidx = lrc = 0;
      StopTimeout();    //Immediatly Disable the timeout.
      if (!ProcessMbMessage) return;     //if we have timed out, return.  We may have already notified the poll response of a timeout.
      // Verify the response message in the buffer.
      //update the status bits.
      switch(ModAttrib.Mode)
      {
         case ASCII_MODE:
            checksum_Status = NormalizeMBAsciiRcvMsg(MB_Exception, mb_data_buffer, mb_data_buffer_len, mb_normalized_rcv_buffer, &mb_normalized_rcv_buffer_len);
            break;
         case RTU_MODE:
            // Normalize RTU Rcv message into a common
            // binary message and check crc
            checksum_Status = NormalizeMB_RTU_RcvMsg(MB_Exception, mb_data_buffer, mb_data_buffer_len, mb_normalized_rcv_buffer, &mb_normalized_rcv_buffer_len);
            break;
      }

      if ( ModbusConfig.type == MB_MASTERMODE ) {
         // check to see if function 3 response message registers need to be swapped
         if ( ( checksum_Status ) && ( !MB_Exception ) ) {
            MB_Exception = checksum_Status;
         }

         if ( ( !MB_Exception ) && ( FloatSwap ) && ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 0x03 ) ) {
            MB_Exception = FltSwap_Response_Flip ( mb_normalized_rcv_buffer, mb_normalized_rcv_buffer_len, ModbusConfig.type );
	      }

         MB_Exception = MB_LoadProduceBuffer(MB_Exception);
      }
      else { //////////////// slave mode //////////////////////////

         if ( ( checksum_Status ) && ( !MB_Exception ) && ( mb_normalized_rcv_buffer[MB_STATION_ID] == ModbusConfig.slaveID ) ) {
            MB_Exception = checksum_Status;
         }

         if ( mb_normalized_rcv_buffer[MB_STATION_ID] != ModbusConfig.slaveID ) {
            mb_data_buffer_len = 0;			// zero buffer size jtm 113012
            // if NOT this units address: stop further processing and exit
            ProcessMbMessage=0;
            MB_Exception = 0;
            MB_Status = READY_FOR_COMMAND;
            TriggerCOS();
            return;
         }
         else if ( ( !MB_Exception ) && ( ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 8 ) && ( !mb_normalized_rcv_buffer[MB_FUNC_LOC + 1] ) && ( !mb_normalized_rcv_buffer[MB_FUNC_LOC + 2] ) ) ) {
            // process valid function 8 by copying the contents of the
            // mb_data_buffer to the mb_data_out_buffer and sending it back to
            // the MB device
            int indx = 0;
            for (indx = 0; indx < mb_data_buffer_len; indx++)
            {
               mb_data_buffer_out[indx] = mb_data_buffer[indx];
            }
            // message should already have CRC or LRC in the original message
            mb_data_buffer_out_len = mb_data_buffer_len;
            StartMbSend();
            // no other message processing necessary
         }
         else {
            // check to see if function 16 command message registers need to be swapped
            // and if function 3 command has the proper number of registers to swap
            if ( ( !MB_Exception ) && ( FloatSwap ) && ( ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 0x10 ) || ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 0x03 ) ) ) {
               MB_Exception = FltSwap_Command_Flip ( mb_normalized_rcv_buffer, mb_normalized_rcv_buffer_len, ModbusConfig.type );
	         }

            if (!MB_Exception) {
               if(NoByteSwap == TRUE) CommandFlip(mb_normalized_rcv_buffer);
               MB_Exception = CheckLimitParameters(mb_normalized_rcv_buffer, mb_normalized_rcv_buffer_len);
            }

            MB_Exception = MB_LoadProduceBuffer(MB_Exception);

            if ( (MB_Exception > 0 ) && ( MB_Exception < CHECKSUM_ERROR ) )
            {
               mb_data_buffer_temp[MB_STATION_ID] = mb_normalized_rcv_buffer[MB_STATION_ID];
               mb_data_buffer_temp[MB_FUNC_LOC] = (0x80 | mb_normalized_rcv_buffer[MB_FUNC_LOC]);

               mb_data_buffer_temp[MBR_ERROR_LOC] = MB_Exception;
               mb_data_buffer_temp_len = 3;
               // byte count to high send error
               if(MBLoad(mb_data_buffer_temp)) //load the modbus buffer system, verify the buffer
               {
                  StartMbSend();
               }
               else
               {
                  // error occurred set MB_State to READY_FOR_COMMAND
                  MB_Status = READY_FOR_COMMAND;
               }

               if (MB_Exception)
               {
                  // error occurred set MB_State to READY_FOR_COMMAND
                  MB_Status = READY_FOR_COMMAND;
               }
            }
         }
      }

      ProcessMbMessage=0;

      if ( ( ModbusConfig.type == MB_MASTERMODE ) || ( MB_Exception ) ) {
         waiting=0;
         MB_Status = READY_FOR_COMMAND;
      }
      else { // ModbusConfig.type == MB_SLAVEMODE
         // set timeout to wait for response from DN side only if a successful message is
         // in DN buffer so the PLC will process and send a response
         if ( MB_Status != READY_FOR_COMMAND ) {
            MB_Status = WAITING_FOR_RESPONSE;
         }
      }
      TriggerCOS();
   }
}

//types
#define REQ  1
#define RSP  0

/**
 * @brief Check_For_Valid_MB_Msg() Functions to check for valid modbus message with buffer and length.  slaves and Master frame to connected devices.
 *
 *
 * @param unsigned char * 							- Pointer to buf of MODBUS Message
 * @param unsigned char 							- Length of modbus message
 * @return unsigned char 							- TRUE or FALSE based on error.
 *
 */
unsigned char Check_For_Valid_MB_Msg(unsigned char *buf, unsigned char len)
{
   uchar err = FALSE;

   if(ModAttrib.Mode !=  RTU_MODE) return err;

   switch(buf[1])  //check command
   {
   case 1:
   case 2:
   case 3:
   case 4:
      if(ModbusConfig.type == MB_MASTERMODE)//receives MB responses
      {
         if((len != (5 + buf[2]))) err = MODBUS_INVALID_MESSAGE;
      }
      else //MB_SLAVEMODE receives MB Requests
      {
         if((len != 8)) err = MODBUS_INVALID_MESSAGE;
      }
      break;

   case 5:
   case 6:
      if(ModbusConfig.type == MB_MASTERMODE)//receives MB responses
      {
         if((len != 8)) err = MODBUS_INVALID_MESSAGE;
      }
      else //MB_SLAVEMODE receives MB Requests
      {
         if((len != 8)) err = MODBUS_INVALID_MESSAGE;
      }
      break;

   case 15:
   case 16:
      if(ModbusConfig.type == MB_MASTERMODE)//receives MB responses
      {
         if((len != 8)) err = MODBUS_INVALID_MESSAGE;
      }
      else //MB_SLAVEMODE receives MB Requests
      {
         if(len != (9 + buf[6]))  err = MODBUS_INVALID_MESSAGE;
      }
      break;

   default: err = INVALID_FUNC_ERROR;
   }
   return err;
}


/**
 * @brief MB_LoadProduceBuffer() Function helps to load producer buffer with respective response for modbus request.
 *
 *
 * @param unsigned char * 							- MODBUS Error code
 * @return unsigned char 							- Error code as modbus exception.
 *
 */
unsigned char MB_LoadProduceBuffer(unsigned char error)
{
   int i,bufcount;
   BYTE errorcode =0;
   unsigned char idx,cnt;
   static unsigned char RxID_S;     // Rick_TEST this is a trial values used below.

   error = error;
   // received a message ok
   mb_messagesent = 0;
   mb_timer = 0;
   mb_timeoutcounter = ModbusConfig.timeout;

   idx = 0;

   if (ModbusConfig.type == MB_MASTERMODE) // in RTU master mode
   {
      if(NoByteSwap == FALSE) ResponseFlip(mb_normalized_rcv_buffer);
      switch(mb_normalized_rcv_buffer[MB_FUNC_LOC])
      {
        case 1:
        case 2:
        case 3:
        case 4:
        idx = 0;
#ifdef Rick_TEST
        if(MB_Status!=1)
        	MB_Status = 1;
#endif

        mainloopassydata[idx++] = MB_Status;
        mainloopassydata[idx++] = errorcode;
        if ( ( !error ) || ( error == FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR) ) {
            if( ++input_txid == 0 ) ++input_txid;
        }
        mainloopassydata[idx++] = input_txid;                                   //set transmit record #
        mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_STATION_ID];      //get station address
        mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_FUNC_LOC];        //get function code
        mainloopassydata[idx++] = 0;                                            //insert 4 byte space for L1y
        mainloopassydata[idx++] = 0;                                            //insert 4 byte space for L1y
        mainloopassydata[idx++] = 0;                                            //insert 4 byte space for L1y
        mainloopassydata[idx++] = 0;                                            //insert 4 byte space for L1y
        mainloopassydata[idx++] = mb_normalized_rcv_buffer[MBR_F1TO4_BYTE_CNT]; //get byte count
        // only allow MaxRxSize (Maximum Input Assembly Data field size) amount of data or less
        // to be transferred to to the Input Assembly Data Field
        if ( mb_normalized_rcv_buffer[MBR_F1TO4_BYTE_CNT] > MaxRxSize ) {
            // limit size of data transferred to size of Input Assembly Data field
            // cnt = size of Input Assembly Data field.
            cnt = MaxRxSize;
        }
        else {
            // allow full size of data in the message buffer to be transferred
            // to the Input Assembly Data field
            cnt = mb_normalized_rcv_buffer[MBR_F1TO4_BYTE_CNT];
        }

        for ( i = MBR_F1TO4_DATA_STRT; i < ( cnt + 3 ); i++)
        {
            mainloopassydata[idx++] = mb_normalized_rcv_buffer[i];  //get data bytes
        }

        size_of_mainloopassydata = cnt + 10;
        break;

        case 5:
        case 6:
        case 15:
        case 16:
        idx = 0;
        mainloopassydata[idx++] = MB_Status;
        mainloopassydata[idx++] = errorcode;
        if ( ( !error ) || ( error == FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR) ) {
            if( ++input_txid == 0 ) ++input_txid;
        }
        mainloopassydata[idx++] = input_txid;                     //set transmit record #
        mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_STATION_ID];  //get station address
        mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_FUNC_LOC];    //get function code

        for ( i = MBR_F5_6_15_16_DATA; i < MBR_F5_6_15_16_LEN; i++ )
        {
            mainloopassydata[idx++] = mb_normalized_rcv_buffer[i];  //get start adr (2 bytes) & qty (2 bytes)
        }
        size_of_mainloopassydata = mb_normalized_rcv_buffer_len + 3;
        break;

        default:
        // if it message is an exception message, process it here
        if ( mb_normalized_rcv_buffer[MB_FUNC_LOC] & 0x80 ) {
           idx = 0;
           mainloopassydata[idx++] = MB_Status;
           if ( !error ) {
              error = mb_normalized_rcv_buffer[MBR_ERROR_LOC];
           }
           mainloopassydata[idx++] = errorcode;
           // always increment rxid in this case
           if( ++input_txid == 0 ) ++input_txid;
           mainloopassydata[idx++] = input_txid;                              //set transmit record #
           mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_STATION_ID];           //get station address
           mainloopassydata[idx++] = mb_normalized_rcv_buffer[MB_FUNC_LOC];      //get function code
           // not sure if the error  goes anywhere else. Since it is
           // already put in the Input Assembly error  field above, I
           // will leave it out for now - 11/6/2013 DRC.
           size_of_mainloopassydata = mb_normalized_rcv_buffer_len + 3 - 1;   // size of incomming message + 3 for MB_Status, error code, and txid - 1 because error  of message is put in error  field.
        }
        break;
      }
      TriggerCOS();  //8-15-2013 DRC not needed for testing
      mb_data_buffer_len = 0;				// zero buffer size jtm 4-23-2013
   }
   else // in slave mode
   {
      if(NoByteSwap == FALSE) CommandFlip(mb_normalized_rcv_buffer);
      mainloopassydata[DNI_MOD_STATUS] = MB_Status;
      mainloopassydata[DNI_ERROR_CODE] = errorcode;

      if ( ( !error ) || ( error == FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR ) ) {
         if ( ++input_txid == 0 ) ++input_txid;
      }

      mainloopassydata[DNI_RX_ID] = ++RxID_S;
      //  Rick_TEST try incrementing RXIDmainloopassydata[DNI_RX_ID] = input_txid;
      if ( mb_normalized_rcv_buffer_len > MB_STATION_ID ) {
         mainloopassydata[DNI_STATION_ID] =  mb_normalized_rcv_buffer[MB_STATION_ID];
      }
      if ( mb_normalized_rcv_buffer_len > MB_FUNC_LOC ) {
         mainloopassydata[DNI_FUNC_] =  mb_normalized_rcv_buffer[MB_FUNC_LOC];
         // Rick_TEST This looks out of place  mainloopassydata[DNI_FUNC_CODE] =  mb_normalized_rcv_buffer[MB_FUNC_LOC];
      }
      // Note that address bytes in mb_normalized_rcv_buffer were swapped
      // above so that the normal place holder for the MBC_ADDR_HI_LOC has
      // the value for DNI_ADDR_L and MBC_ADDR_LO_LOC has value for DNI_ADDR_H
      if ( mb_normalized_rcv_buffer_len > MBC_ADDR_HI_LOC ) {
         mainloopassydata[DNI_ADDR_L] = mb_normalized_rcv_buffer[MBC_ADDR_HI_LOC];
      }
      if ( mb_normalized_rcv_buffer_len > MBC_ADDR_LO_LOC ) {
         mainloopassydata[DNI_ADDR_H] = mb_normalized_rcv_buffer[MBC_ADDR_LO_LOC];
      }
      // Note that quntity bytes in mb_normalized_rcv_buffer were swapped
      // above so that the normal place holder for the MBC_QUANT_HI_LOC has
      // the value for DNI_QUANT_L and MBC_QUANT_LO_LOC has value for DNI_QUANT_H
      if ( mb_normalized_rcv_buffer_len > MBC_QUANT_HI_LOC ) {
         mainloopassydata[DNI_QUANT_L] = mb_normalized_rcv_buffer[MBC_QUANT_HI_LOC];
      }
      if ( mb_normalized_rcv_buffer_len > MBC_QUANT_LO_LOC ) {
         mainloopassydata[DNI_QUANT_H] = mb_normalized_rcv_buffer[MBC_QUANT_LO_LOC];
      }

      if ( ( ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 15 ) || ( mb_normalized_rcv_buffer[MB_FUNC_LOC] == 16 ) ) && ( mb_normalized_rcv_buffer_len > MBC_BYTE_CNT_LOC ) ) {

         mainloopassydata[DNI_BYTE_COUNT] = mb_normalized_rcv_buffer[MBC_BYTE_CNT_LOC];

         if ( mb_normalized_rcv_buffer_len > MBC_DATA_STRT_LOC ) {

            // only allow MaxRxSize (Maximum Input Assembly Data fiels size) amount of data or less
            // to be transferred to to the Input Assembly Data Field
            if ( mb_normalized_rcv_buffer[MBC_BYTE_CNT_LOC] > MaxRxSize ) {
               // limit size of data transferred to size of Input Assembly Data field
               cnt = MaxRxSize;
            }
            else if ( ( mb_normalized_rcv_buffer_len - MBC_DATA_STRT_LOC ) == mb_normalized_rcv_buffer[MBC_BYTE_CNT_LOC] ) {
               // allow full size of data in the message buffer to be transferred
               // to the Input Assembly Data field
               cnt = mb_normalized_rcv_buffer[MBC_BYTE_CNT_LOC];
            }
            else {
               // if buffer len - 7 (i.e. 7 is number of non data fields) != Byte count
               // then set the count to buffer len - 7
               cnt =  mb_normalized_rcv_buffer_len - MBC_DATA_STRT_LOC;
            }

            for ( i = 0; i < cnt; i++ ) {
               mainloopassydata[DNI_DATA_STRT + i] = mb_normalized_rcv_buffer[MBC_DATA_STRT_LOC + i];
            }

            bufcount = cnt + 7; // = data size + station id + func + Addr Hi + Addr Lo + Quant Hi + Quant Lo + byte count
         }
      }
      else { // functions 1-6 have a fixed length and bad 15 and 16 messages
         // just in case extra data is there for invalid data error the ilx will
         // go ahead and transfer the data to mainloopassydata buffer until MaxRxSize
         // (Input Assembly Data limit in number of bytes).
         if ( ( mb_normalized_rcv_buffer_len > 6 ) && ( mb_normalized_rcv_buffer_len <= ( MaxRxSize + 1 ) ) ) {
            cnt = mb_normalized_rcv_buffer_len - 6;
            bufcount =  mb_normalized_rcv_buffer_len;
         }
         else if ( ( mb_normalized_rcv_buffer_len > 6 ) && ( mb_normalized_rcv_buffer_len > ( MaxRxSize + 1 ) ) ) {
           cnt = MaxRxSize + 1 - 6;
           bufcount =  MaxRxSize + 1;
         }
         else { // mb_normalized_rcv_buffer_len == 6 or less
           cnt = 0;
           bufcount =  mb_normalized_rcv_buffer_len;
         }
         // copy the rest of the data if necessary
         for ( i = 0; i < cnt; i++ ) {
            mainloopassydata[DNI_BYTE_COUNT + i] = mb_normalized_rcv_buffer[MBC_BYTE_CNT_LOC + i];
         }
      }

      size_of_mainloopassydata = bufcount + 3;  // add 3 for status, error code, and txid.
      TriggerCOS();		// trigger the produce  // not needed for test DRC
      mb_data_buffer_len = 0;				// zero buffer size jtm 4-23-2013
   }

   for ( i = size_of_mainloopassydata; i < Ascii.TransmitSize; i++ )
   {
      mainloopassydata[i] = 0x00;
   }
   // make the size the max
   mb_data_buffer_len = 0;				// zero buffer size jtm 4-23-2013
   return(errorcode);
}

unsigned int  startreg,regqty,regcount,DataQty,DataBytes;
/**
 * @brief CheckLimitParameters() function to check limit parameters of received message ILX slave mode.
 *
 *
 * @param unsigned char *  					- Pointer to buffer data of command message
 * @param unsigned char  					- Buf_len buffer length
 * @return uchar 							- Return the data validation based error.
 *
 */
uchar CheckLimitParameters(unsigned char  * buf, unsigned char buf_len)
{
   // Assumptions:
   //   1. This is only checking command messages from the MB device when the ilx device is in slave mode
   //   2. The function assumes message in buf uses default MB Big - Endian for address and two byte counts
   //   3. if NoByteSwap is enabled in the DN message header then the addresses and two byte counts need
   //      to be swapped to Big - Endian format before calling this function.  This means that
   //      swapping should only occurr if NoByteSwap == TRUE. It also means that it does not have to be swapped
   //      swapped again, ever, before calling MB_LoadProduceBuffer.
   //
   //   NOTE: Added input option for buf_len so that thne message length can be verified in the future
   //         if needed. It seems that this should be done to make sure everything is correct.

   uchar err = 0; // err is the response passed back on function exit, initialize to 0.

   unsigned int intAddress = 0;
   unsigned int intQnty = 0;

   unsigned char ByteCnt = 0;

   switch ( buf[MB_FUNC_LOC] ) {
      case 1:
         // Convert message start address and Quantity of Coils hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);
         intQnty = (buf[MBC_QUANT_HI_LOC] << 8) + (buf[MBC_QUANT_LO_LOC]);

         // Is message length correct for this request?
         ByteCnt = intQnty/8;
         if ( ( intQnty % 8 ) != 0 ) {
             ByteCnt += 1;
         }

         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.Coil_Count ) || ( ByteCnt > MaxRxSize ) ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?
         else if ( (intAddress < ModbusConfig.Coil_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.Coil_StartAddr + ModbusConfig.Coil_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 2:
         // Convert message start address and Quantity of Inputs hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);
         intQnty = (buf[MBC_QUANT_HI_LOC] << 8) + (buf[MBC_QUANT_LO_LOC]);

         // Calculate the number of bytes being requested
         ByteCnt = intQnty/8;
         if ( ( intQnty % 8 ) != 0 ) {
             ByteCnt += 1;
         }

         // Is message length correct for this request?
         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.DiscInput_Count ) || ( ByteCnt > MaxRxSize ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?
         else if ( ( intAddress < ModbusConfig.DiscInput_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.DiscInput_StartAddr + ModbusConfig.DiscInput_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 3:
         // Convert message start address and Quantity of Inputs hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);
         intQnty = (buf[MBC_QUANT_HI_LOC] << 8) + (buf[MBC_QUANT_LO_LOC]);

         ByteCnt = 2 * ( ( unsigned char ) intQnty );

         // Is message length correct for this request?
         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.HoldReg_Count ) || ( ByteCnt > MaxRxSize ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?
         else if ( ( intAddress < ModbusConfig.HoldReg_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.HoldReg_StartAddr + ModbusConfig.HoldReg_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 4:
         // Convert message start address and Quantity of Inputs hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);
         intQnty = (buf[MBC_QUANT_HI_LOC] << 8) + (buf[MBC_QUANT_LO_LOC]);

         ByteCnt = 2 * ( ( unsigned char ) intQnty );

         // Is message length correct for this request?
         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.InReg_Count ) || ( ByteCnt > MaxRxSize ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?
         else if ( ( intAddress < ModbusConfig.InReg_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.InReg_StartAddr + ModbusConfig.InReg_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 5:
         // Convert message output address and write values hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);
         intQnty = (buf[MBC_QUANT_HI_LOC] << 8) + (buf[MBC_QUANT_LO_LOC]);

         // Is message length correct for this request?
         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is output value 0x0000 or 0xFF00?
         else if ( ( intQnty != 0x0000 ) && ( intQnty != 0xFF00 ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is address ok?
         else if ( ( intAddress < ModbusConfig.Coil_StartAddr ) ||
              ( intAddress > ( ModbusConfig.Coil_StartAddr + ModbusConfig.Coil_Count - 1 ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 6:
         // Convert message register address hi and low bytes to integer values
         intAddress = (buf[MBC_ADDR_HI_LOC] << 8) + (buf[MBC_ADDR_LO_LOC]);

         // Is message length correct for this request?
         if ( buf_len != 6 ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is address ok?
         else if ( ( intAddress < ModbusConfig.HoldReg_StartAddr ) ||
              ( intAddress > ( ModbusConfig.HoldReg_StartAddr + ModbusConfig.HoldReg_Count - 1 ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 15:
         // Convert message start address and Quantity of Coils hi and low bytes to integer values
         intAddress = ( buf[MBC_ADDR_HI_LOC] << 8 ) + buf[MBC_ADDR_LO_LOC];
         intQnty = ( buf[MBC_QUANT_HI_LOC] << 8 ) + buf[MBC_QUANT_LO_LOC];

         // caluculate the correct length of the message based on the data and compare with buf_len
         // calculate the correct Byte Count value from the quantity of outputs
         ByteCnt = intQnty/8;
         if ( intQnty % 8) {
            ByteCnt += 1;
         }

         // check if byte count in the message is correct
         if ( buf[MBC_BYTE_CNT_LOC] != ByteCnt ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is message length correct for this request?
         else if ( buf_len != ( ByteCnt + 7 ) ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.Coil_Count ) || ( ByteCnt > MaxRxSize ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?  NOTE: if 1st addr is at 0000, max coil addr is at 0000 + max coils - 1
         else if ( (intAddress < ModbusConfig.Coil_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.Coil_StartAddr + ModbusConfig.Coil_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      case 16:
         // Convert message start address and Quantity of Registers hi and low bytes to integer values
         intAddress = ( buf[MBC_ADDR_HI_LOC] << 8 ) + buf[MBC_ADDR_LO_LOC];
         intQnty = ( buf[MBC_QUANT_HI_LOC] << 8 ) + buf[MBC_QUANT_LO_LOC];

         // calculate Byte Count
         if ( ( intQnty < 1 ) || ( intQnty > 255 ) ) {
               err = INVALID_DATALENGTH_ERROR;
               break;
         }
         else {
            ByteCnt = 2 * ( ( unsigned char ) intQnty );
         }
         // check if byte count in the message is correct
         if ( buf[MBC_BYTE_CNT_LOC] != ByteCnt ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Check message length
         else if ( buf_len != ( ByteCnt + 7 ) ) {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is quantity of Coils Requested ok?
         else if ( ( intQnty < 1 ) || ( intQnty > ModbusConfig.HoldReg_Count ) || ( ByteCnt > MaxRxSize ) )
         {
            err = INVALID_DATALENGTH_ERROR;
         }
         // Is Starting address ok?
         else if ( ( intAddress < ModbusConfig.HoldReg_StartAddr ) ||
                   ( ( intAddress + intQnty ) > ( ModbusConfig.HoldReg_StartAddr + ModbusConfig.HoldReg_Count ) ) )
         {
            err = INVALID_ADR_ERROR;
         }
         break;
      default:
         err = INVALID_FUNC_ERROR;
   }

   return err;
}

// Dummy functions of ASCII Module
void AsciiSetDPS (MSG *msg)
{
	return;
}
void AsciiSetBaudrate (MSG *msg)
{
	return;
}
void ToggleAndLockSyncBits (void)
{
	return;
}
unsigned char current_status_byte (void)
{
	return 0;
}
void *AsciiFunc (MSG *msg)
{
	// Nothing to do in MODBUS code for this function
	return 0;
}

// end of file
