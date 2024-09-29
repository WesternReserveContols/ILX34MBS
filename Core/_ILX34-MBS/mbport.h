/**
 * @file mbport.h
 * @author Jignesh
 * @date 25 Jul 2022
 * @brief This file contains mbport header related information.
 *
 */


#include <fifo.h>
#include <msg.h>
#include <serial_config.h>
#include "stdbool.h"

#ifndef _MBPORT_H
#define _MBPORT_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_MODBUS_MESSAGE_SIZE 		255
#define MB_DATA_BUFFER_SIZE MAX_MODBUS_MESSAGE_SIZE //size of the modbus data buffer
#define ASCII_MODE_INTER_CHAR_TO_INTERVAL  1000    // millisec

#define ASCII_MODE 0
#define RTU_MODE 1

#define TIMEOUT_ERROR  0xD0
#define PARITY_ERROR   0xD1

#define TxRx_RECV    0
#define TxRx_XMIT    1

#ifdef EMU  // 16mh
#define BAUD3	0
#define BAUD6	0
#define BAUD12	0	 //521
#define BAUD24	0	 //260
#define BAUD48	0
#define BAUD96	972
#define BAUD19	998
#define BAUD38 1011
#else // 20mh
#define BAUD12	503	 //521
#define BAUD24	764	 //260
#define BAUD48	894	 //130
#define BAUD96	959	 //65
#define BAUD19	991	 //33
#define BAUD38 1008	 //16
#endif

//framing defines
#define FRAME_7N2         0
#define FRAME_7E1         1
#define FRAME_7O1         2
#define FRAME_8N1         3
#define FRAME_8N2         4
#define FRAME_8E1         5
#define FRAME_8O1         6
#define FRAME_7E2         7
#define FRAME_7O2         8

#define ASCII  0
#define RTU    1

#define NONE 0
#define ODD  1
#define EVEN 2

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned int
#endif

#define MB_MASTERMODE	0
#define MB_SLAVEMODE	1

// errors 1-8 are BYTE ProduceData[DATA_BUFFER_SIZE];
//  BYerrors defined by MB Spec
#define INVALID_FUNC_ERROR                         0x01
#define INVALID_ADR_ERROR                          0x02
#define INVALID_DATALENGTH_ERROR                   0x03
#define SLAVE DEVICE FAILURE                       0x04
#define ACKNOWLEDGE                                0x05
#define SLAVE_DEVICE_BUSY                          0x06
#define NEGATIVE_ACKNOWLEDGE                       0x07
#define MEMORY_PARITY_ERROR                        0x08
#define GATEWAY_PATH_UNAVAILABLE                   0x0A  // decimal 10
#define GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND    0x0B  // decimal 11

// errors 10 + are ILX specific errors
#define CHECKSUM_ERROR                             0x14  // decimal 20
#define MODBUS_INVALID_MESSAGE	                   0x15  // decimal 21
#define MODBUS_TIMEOUT_ERROR	                   0x16  // decimal 22
#define MODBUS_BUFFER_OVERRUN                      0x19  // decimal 25
#define FLOAT_WORD_SWAP_UNEVEN_WORD_COUNT_ERROR    0x1A  // decimal 26 uneven word count for word swap 

// P_InMsgBuf Byte definitions  (Received Poll from PLC Assy 102) ILX-9  
#define MBM_CFGHDR_BYTES 0
#define MBM_CMDS        0
#define MBM_CMDS_BOOT  	0x01    // Command Bits  ILX-13
#define MBM_CMDS_RSTCTR 0x02    // Command Bits
#define MBM_CMDS_SWAP   0x04    // Command Bits
#define MBM_FLOAT_SWAP     0x18    // Float Swap Command Bits
#define MBM_FLOAT_SWAP_WORD_PAIRS  0x08
#define MBM_FLOAT_SWAP_BYTES_WORDS 0x10
#define MBM_FLOAT_SWAP_BYTES       0x18    
#define MBM_RESERVED_BIT_5 0x20    // reserved bit 5
#define MBM_RESERVED_BIT_6 0x40    // reserved bit 6
#define MBM_RESERVED_BIT_7 0x80    // reserved bit 7


enum mb_status_enum_t
{
	READY_FOR_COMMAND = 1,
	WAITING_FOR_RESPONSE,
	PROCESSING_COMMAND,
	PROCESSING_RESPONSE,
};

typedef struct
{
//  WORD  NumAttrib;
//  WORD  AttribList[ATTRIB_QTY];
//  WORD  ConsumePathSize;
//  BYTE ConsumePath[PATH_SIZE];
//  WORD  ProducePathSize;
//  BYTE ProducePath[PATH_SIZE];
//  WORD  ConsumeDataSize;
//  BYTE ConsumeData[DATA_BUFFER_SIZE];
//  WORD  ProduceDataSize;
//  BYTE ProduceData[DATA_BUFFER_SIZE];
  BYTE Status;
  BYTE Mode;
  BYTE ASCIIinstance;
} MODBUS_ATTRIB;


extern ASCIISTRUCT Ascii_attrib;


//typedef struct
//{
//  unsigned char   protocol;             // ascii or RTU
//  unsigned char   framing;              // serial comm framing 8N1...
//  unsigned char   baudrate;             // serial baudrate
//  unsigned char   rxsize;               // receive size
//  unsigned char   txsize;               // transmit size
//  unsigned char   type;                 // modbus master or slave mode
//  unsigned int    timeout;              // timeout for modbus communication
//  unsigned int    slaveID;              // id of the slave
//  unsigned int    Coil_StartAddr;       // starting address of the input coil
//  unsigned int    Coil_Count;           // 10/24/2013 DRC - now means global count that determines max upper coil address
//  unsigned int    DiscInput_StartAddr;  // the output coil start address
//  unsigned int    DiscInput_Count;      // 10/24/2013 DRC - now means global count that determines max upper discrete input address
//  unsigned int    InReg_StartAddr;      // the input register start address
//  unsigned int    InReg_Count;          // 10/24/2013 DRC - now means global count that determines max upper input register address
//  unsigned int    HoldReg_StartAddr;    // the output register start
//  unsigned int    HoldReg_Count;        // 10/24/2013 DRC - now means global count that determines max upper holding address
//}MB_CONFIG;

#define ASCII_MODE_INTER_CHAR_TO_INTERVAL  1000    // millisec


bool           MB_Sys_Busy(void);
unsigned char MB_Sys_err(void);	
void          StartMbSend(void);
int           MBLoad(unsigned char *src);
unsigned char ComputeIOConsumeSize(void);
unsigned char ComputeIOProduceSize(void);
extern void MBM_QueMbTxMsg(unsigned char  *P_InBuf);
//void          MBPort_8ms_Timer(void);
extern void	  ModbusMain(void);
extern void Serial_RX_ISR(void);
extern void Serial_TX_ISR(void);
extern void InitMbParam(void);
extern void InitAssembly(void);
extern void main_port_serial (void);
extern void Mb_FactoryDefaults(void);
extern void InitRtuTimeout(void);
extern void MB_Rtu_TimedOut(void);
extern void InitSerialIO(void);
extern void RestoreSerialFromEE (void);
// Get Functions
void GetFraming(MSG  * msg);
void GetBaudRate(MSG  * msg);

void MB_GetProtocol(MSG  * msg);
void MB_GetType(MSG  * msg);
void MB_GetTimeout(MSG  * msg);
void MB_GetSlaveID(MSG  * msg);
void MB_GetCoil_StartAddr(MSG  * msg);
void MB_GetCoil_Count(MSG  * msg);
void MB_GetDiscInput_StartAddr(MSG  * msg);
void MB_GetDiscInput_Count(MSG  * msg);
void MB_GetInReg_StartAddr(MSG  * msg);
void MB_GetInReg_Count(MSG  * msg);
void MB_GetHoldReg_StartAddr(MSG  * msg);
void MB_GetHoldReg_Count(MSG  * msg);

void SetBaudRate(MSG  * msg);void SetFraming(MSG  * msg);

void MB_SetProtocol(MSG  * msg);
void MB_SetType(MSG  * msg);
void MB_SetTimeout(MSG  * msg);
void MB_SetSlaveID(MSG  * msg);
void MB_SetCoil_StartAddr(MSG  * msg);
void MB_SetCoil_Count(MSG  * msg);
void MB_SetDiscInput_StartAddr(MSG  * msg);
void MB_SetDiscInput_Count(MSG  * msg);
void MB_SetInReg_StartAddr(MSG  * msg);
void MB_SetInReg_Count(MSG  * msg);
void MB_SetHoldReg_StartAddr(MSG  * msg);
void MB_SetHoldReg_Count(MSG  * msg);

/////these are read only, do not write to these ever.
// extern unsigned char xdata produce_buffer[MAX_MODBUS_MESSAGE_SIZE]; //AP
extern unsigned char xdata, produce_buffer[MAX_MODBUS_MESSAGE_SIZE];
extern unsigned char produce_buffer_len;
extern unsigned int Ascii_Mode_InterChar_Time;       // millisec
extern unsigned char ASCII_Mode_InterChar_TO_flg; // set to true whenever a timeout occurs
extern unsigned char ASCII_Mode_InterChar_TO_ON;  // set to true when ASCII_MODE to turn on timer
extern unsigned char  MB_Status, MB_Exception;
extern int waiting;
extern MB_CONFIG	ModbusConfig;

#define SOFT_RESET_ACTIVE 0xA5
extern unsigned char SoftReset;


#endif /*  _MBPORT_H_  ILX34_MBS_MBPORT_H_ */


