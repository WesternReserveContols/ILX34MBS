/*
 * can_hal.c
 *
 *  Created on: Oct 17, 2019
 *      Author: jason
 */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bsp_main.h"
#include "io_peripheral.h"
#include "can_hal.h"
#include "can_fifo.h"
#include "can.h"

#include "dn_dnobj.h"

/////////////////////////////
// Internal variables
//

CAN_HandleTypeDef	  hcan1			 = { 0 };		 // Can Handle Variable - CAN1 state data
static uint32_t		  can1_prescaler = CH_BAUD_125K; // used in set baud instruction. Default here to 125k
static CH_LEC_Error_t can_LEC_error	 = CH_LEC_None;

/////////////////////////////
// Internal Functions
//
static HAL_StatusTypeDef CH_Set_Filter_Params (void);
static CH_Error_t		 CH_Frame_to_HAL_Frame (CAN_TxHeaderTypeDef *halHeader, uint8_t *halData, CH_Frame_t *chFrame);
static CH_Error_t		 HAL_Frame_to_CH_Frame (CH_Frame_t *chFrame, CAN_RxHeaderTypeDef *halHeader, uint8_t *halData);

/////////////////////////////
// Exported Functions
//  These are the standard functions exposed in can_hal.h
//  The can_hal.h header is shared between the Linux and FreeRtos port
//

/**
 * @brief  Initialize the CAN1 Hardware Port.
 * @retval CAN Hardware Error status
 */
CH_Error_t CH_Init (void)
{
	CH_Error_t err				= CH_ERROR_None;
	uint32_t   interruptEnables = 0;
	//
	// Set the Interrupt enables to unmask FIFO 0 RX events.
	interruptEnables = CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN;
	//
	// Disable all CAN FIFO 0 Interrupts
	// We may be CAN active and receiving frames if this is a reset.
	// All interrupts will be Re-enabled by the Init functions
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	// Init our FIFO. This will discard all frames in ram!
	CAN_FIFO_Reset ();
	//
	// Check our init status.
	if ((HAL_CAN_GetState (&hcan1) == HAL_CAN_STATE_READY) || (HAL_CAN_GetState (&hcan1) == HAL_CAN_STATE_LISTENING))
	{
		// DSC_Writes (DSC_LEVEL_INFO, "Re-Init CAN port\n\r");
		//
		// Remove any lingering frames in our hardware buffer.
		// Docs were not super clear that a CAN RESET would clear these...
		while (HAL_CAN_GetRxFifoFillLevel (&hcan1, CAN_RX_FIFO0) != 0)
		{
			CAN_RxHeaderTypeDef dummy_header  = { 0 };
			uint8_t				dummy_data[8] = { 0 };
			DSC_Writes (DSC_LEVEL_INFO, "Remove HW FIFO Frame\n\r");
			if (HAL_CAN_GetRxMessage (&hcan1, CAN_RX_FIFO0, &dummy_header, dummy_data) != HAL_OK)
			{
				// If we have a problem removing for some reason, we dont want to get stuck.
				// Just break out and continue with the Init
				DSC_Writes (DSC_LEVEL_INFO, "Error removing frame?\n\r");
				break;
			}
		}
		// Clear Interrupt Enables
		HAL_CAN_DeactivateNotification (&hcan1, interruptEnables);
		// Bring the hardware to the reset state
		HAL_CAN_DeInit (&hcan1);
	}
	// Clear any pending interrupts
	HAL_NVIC_ClearPendingIRQ (CEC_CAN_IRQn);
	//
	// Set CAN Handle properties
	hcan1.Instance			 = CAN;
	hcan1.Init.Prescaler	 = can1_prescaler; // set by baud function
	hcan1.Init.Mode			 = CAN_MODE_NORMAL;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
//	hcan1.Init.TimeSeg1		 = CAN_BS1_4TQ;
//	hcan1.Init.TimeSeg2		 = CAN_BS2_3TQ;
	hcan1.Init.TimeSeg1		 = CAN_BS1_6TQ;    //   Rick_TEST 1/4/23  see 1734ASC redesign V8 Patch information
	hcan1.Init.TimeSeg2		 = CAN_BS2_1TQ;
	// Defaults
	hcan1.Init.TimeTriggeredMode	= DISABLE; // Not used
	hcan1.Init.AutoBusOff			= DISABLE; // Disable auto recovery from busoff (we manage in software)
	hcan1.Init.AutoWakeUp			= DISABLE; // We will never sleep
	hcan1.Init.AutoRetransmission	= ENABLE;  // Part of CAN standard
	hcan1.Init.ReceiveFifoLocked	= DISABLE; // New RX frames will overwrite FIFO on overflow
	hcan1.Init.TransmitFifoPriority = ENABLE;  // Send TX Frames in order requested
	//
	// Set up CAN HardwareSoftware
	//
	if (HAL_CAN_Init (&hcan1) != HAL_OK)
	{
		Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_Init", DEBUG_STOP);
		err = CH_ERROR_Init;
	}
	else if (CH_Set_Filter_Params () != HAL_OK)
	{
		Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_ConfigFilter", DEBUG_STOP);
		err = CH_ERROR_Init;
	}
	else if (HAL_CAN_ActivateNotification (&hcan1, interruptEnables) != HAL_OK)
	{
		Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_ActivateNotification", DEBUG_STOP);
		err = CH_ERROR_Init;
	}
	else if (HAL_CAN_Start (&hcan1) != HAL_OK)
	{
		// Print our error number
		DSC_Write_UInt (DSC_LEVEL_INFO, HAL_CAN_GetError (&hcan1), 1);
		err = CH_ERROR_Init;
		//
		// CAN bus Hardware requires 11 consecutive idle bits on bus to start. (See datasheet)
		// On an active high speed bus (1M), there may not be enough idle time
		//  to start the bus at a low speed like 125k. This can happen during autobaud
		//  Error number for this would be a timeout 0x20000 TODO handle any other way?
		//
		// If we are not in autobaud, a no-start would be a problem. Assert
		if (ABAUD_ENABLED != DeviceNetObjectRAM.bAutoBaud)
		{
			Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_Start no autobaud", DEBUG_STOP);
		}
		else
		{
			// in autobaud, just alert and continue.
			Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_Start", DEBUG_CONTINUE);
		}
	}
	//
	return err;
}

bool CH_Is_Fifo_Empty (void)
{
	// Disable HW FIFO 0 Interrupts. We need to touch our shared RAM Fifo
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	// Peek at our fifo
	bool ret = IS_CAN_FIFO_Empty ();

	// exit critical section
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);
	//
	return ret;
}

CH_Error_t CH_Receive (CH_Frame_t *frame)
{
	CH_Assert (frame);
	CH_Error_t err = CH_ERROR_No_Data;
	//
	// Disable HW FIFO 0 Interrupts. We need to touch our shared CAN1 structure and RAM Fifo
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	if (!IS_CAN_FIFO_Empty ())
	{
		// Copy the data from RAM to our caller
		CAN_FIFO_Get (frame);

		// ** TEST *****************
		// char buff[30] = { 0 };
		/*
		static uint32_t handled	 = 0;
		static int		max_buff = 0;

		handled++;
		if (CAN_FIFO_Num_Elements () > max_buff)
		{
			max_buff = CAN_FIFO_Num_Elements ();
		}

		// sprintf (buff, "Handled: %lu, FIFO: %d\r\n", handled, CAN_FIFO_Num_Elements ());
		// DSC_Writes (DSC_LEVEL_INFO, buff);
		// DSC_Writes (DSC_LEVEL_INFO, "num fifo maxfifo\r\n");
		// DSC_Write_UInt (DSC_LEVEL_INFO, handled, 0);
		DSC_Write_UInt (DSC_LEVEL_INFO, CAN_FIFO_Num_Elements (), 0);
		DSC_Write_UInt (DSC_LEVEL_INFO, max_buff, 0);
		DSC_Writes (DSC_LEVEL_INFO, "\r\n");
		*/
		// ** TEST *****************
		err = CH_ERROR_None;
	}
	// Check our HW FIFO Overflow error state.
	if (HAL_CAN_GetError (&hcan1) & HAL_CAN_ERROR_RX_FOV0)
	{
		// Hardware FIFO 0 Overflow detected.
		// This is set in the hcan1 state by the HAL IRQ routine
		// reset our error code
		hcan1.ErrorCode &= ~HAL_CAN_ERROR_RX_FOV0;
		//
		// TODO: What will we do in production
		// err = CH_ERROR_None
		Debug_ReportError (DEBUG_ERR_CAN, "FIFO HW OVFL", DEBUG_STOP);
	}
	//
	// exit critical section
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);

	return err;
}

CH_Error_t CH_Transmit (CH_Frame_t *frame)
{
	CH_Assert (frame);
	CH_Assert (frame->length <= 8);
	//
	CAN_TxHeaderTypeDef txHeader  = { 0 };
	uint8_t				txData[8] = { 0 };
	uint32_t			mailbox	  = 0;
	CH_Error_t			err		  = CH_ERROR_None;
	//
	// First convert to valid HAL type frame, Then add to send queue
	// This will add to any mailbox that is not empty
	//
	if (CH_Frame_to_HAL_Frame (&txHeader, txData, frame) != CH_ERROR_None)
	{
		Debug_ReportError (DEBUG_ERR_CAN, "CH_Frame_to_HAL_Frame", DEBUG_STOP);
		err = CH_ERROR_Packet;
	}
	if (HAL_CAN_AddTxMessage (&hcan1, &txHeader, txData, &mailbox) != HAL_OK)
	{
		Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_AddTxMessage", DEBUG_STOP);
		err = CH_ERROR_Tx;
	}
	//
	return err;
}

CH_Error_t CH_Transmit_Dummy (void)
{
	// **** Testing with dummy data ***

	CAN_TxHeaderTypeDef header		= { 0 };
	uint8_t				testData[8] = { 0 };
	uint32_t			mailbox		= 0;
	CH_Error_t			err			= CH_ERROR_None;

	testData[0] = 0x01;
	testData[7] = 0xFF;

	static uint32_t id = 0;
	id++;
	if (id > 0xF)
	{
		id = 0;
	}

	header.StdId			  = 0x0; // id;
	header.ExtId			  = 0x0;
	header.IDE				  = CAN_ID_STD;
	header.RTR				  = CAN_RTR_DATA;
	header.DLC				  = 8;
	header.TransmitGlobalTime = DISABLE;

	while (CH_Is_Finished () == false)
	{
		// DSC_Writes (DSC_LEVEL_INFO, "*");
	}

	if (HAL_CAN_AddTxMessage (&hcan1, &header, testData, &mailbox) != HAL_OK)
	{
		DSC_Write_UInt (DSC_LEVEL_INFO, HAL_CAN_GetError (&hcan1), 1);
		Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_AddTxMessage", DEBUG_STOP);
		err = CH_ERROR_Tx;
	}

	return err;
}

// Check if we have a free mailbox to place a TX message
// There are 3 TX mailboxes on the 072 hardware CAN interface
bool CH_Is_Finished (void)
{
	bool ret = true;
	//
	// enter critical section - we touch our shared state
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	// returns 0 if all mailboxes are pending
	if (HAL_CAN_GetTxMailboxesFreeLevel (&hcan1) == 0)
	{
		ret = false;
	}
	//
	// exit critical section
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);
	//
	return ret;
}

// Check if bus off flag (read only flag)
// returns true if bus is off
bool CH_Is_Busoff (void)
{
	bool ret = false;
	//
	// enter critical section - we touch our shared state
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	// if if we are in a valid CAN state, read the bus off register bit (read only)
	if (hcan1.State != HAL_CAN_STATE_RESET)
	{
		if (((hcan1.Instance->ESR) & CAN_ESR_BOFF) != 0U)
		{
			DSC_Writes (DSC_LEVEL_INFO, "BUS OFF!\n\r");
			ret = true;
		}
	}
	// exit critical section
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);
	//
	return ret;
}

// Returns an ENUM corresponding to the C505 LEC error codes
// Return 0 if no error.
CH_LEC_Error_t CH_Last_Bus_Error_Code (void)
{
	CH_LEC_Error_t returnval = 0;
	//
	// enter critical section - we touch our shared state
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
	//
	// if we are in a valid CAN state, read the ESR register and clear LEC bits
	if (hcan1.State != HAL_CAN_STATE_RESET)
	{
		returnval = can_LEC_error;
	}
	//
	// Reset
	can_LEC_error = CH_LEC_None;
	//
	// exit critical section
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);
	//
	return returnval;
}

CH_Error_t CH_Set_Baud (CH_Baud_t baud)
{
	CH_Error_t ret = CH_ERROR_None;
	switch (baud)
	{
	case CH_BAUD_125K:
		DSC_Writes (DSC_LEVEL_INFO, "125k\n\r");
		can1_prescaler = baud;
		break;
	case CH_BAUD_250K:
		DSC_Writes (DSC_LEVEL_INFO, "250k\n\r");
		can1_prescaler = baud;
		break;
	case CH_BAUD_500K:
		DSC_Writes (DSC_LEVEL_INFO, "500k\n\r");
		can1_prescaler = baud;
		break;
	case CH_BAUD_1M:
		DSC_Writes (DSC_LEVEL_INFO, "1M\n\r");
		can1_prescaler = baud;
		break;
	default:
		Debug_ReportError (DEBUG_ERR_CAN, "Bad Baud Request", DEBUG_STOP);
		ret = CH_ERROR_Init;
	}
	return ret;
}

void CH_Enable_Interrupts (void)
{
	HAL_NVIC_EnableIRQ (CEC_CAN_IRQn);
}

void CH_Disable_Interrupts (void)
{
	HAL_NVIC_DisableIRQ (CEC_CAN_IRQn);
}

// HAL Function callback override that gets called from the standard HAL CAN ISR
//  The function pointer is bound to the CAN state struct
//  This will get called if we have activated the RX FIFO MSG interrupt notification (IER) mask
//  See the HAL CAN functions for more details...
//
void HAL_CAN_RxFifo0MsgPendingCallback (CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef canHeader;
	uint8_t				canData[8];
	CH_Frame_t			chFrame;
	//
	// Here we want to loop around to empty all messages from HW fifo (3 max)
	//  NOTE: The HAL library always copies all 8 bytes of message data from the HW fifo.
	do
	{
		if (IS_CAN_FIFO_Full ())
		{
			// We cannot do anything more. Our RAM fifo is full.
			// This would signify a serious problem - messages not handled fast enough
			// TODO: Is there any other action to take? If we get here we are about to lose messages
			// Lets make this fatal
			Debug_ReportError (DEBUG_ERR_CAN, "CAN RAM FIFO Full", DEBUG_STOP);
			return;
		}
		else if (HAL_CAN_GetRxMessage (hcan, CAN_RX_FIFO0, &canHeader, canData) != HAL_OK)
		{
			Debug_ReportError (DEBUG_ERR_CAN, "HAL_CAN_GetRxMessage", DEBUG_STOP);
		}
		else if (HAL_Frame_to_CH_Frame (&chFrame, &canHeader, canData) != CH_ERROR_None)
		{
			Debug_ReportError (DEBUG_ERR_CAN, "HAL_Frame_to_CH_Frame", DEBUG_STOP);
		}
		else if (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud)
		{
			// If we are autobauding, just indicate RX a packet. Dont touch RAM fifo.
			CP_Set_RXOK (true);
			CP_Set_SUP (true);
		}
		else if (IO_GET_CanEnable ())
		{
			// If our can bus hardware is actually enabled
			// we will stage the frame into the RAM FIFO
			CAN_FIFO_Put (&chFrame);
		}
		//
	} while (HAL_CAN_GetRxFifoFillLevel (hcan, CAN_RX_FIFO0) != 0);
}

// HAL Function callback override that gets called from the standard HAL CAN ISR
//  The function pointer is bound to the CAN state struct
//  This will get called if we have activated the error interrupt notification (IER) mask
//  See the HAL CAN functions for more details...
//
void HAL_CAN_ErrorCallback (CAN_HandleTypeDef *hcan)
{
	// Get error that was just set in ISR
	uint32_t err = HAL_CAN_GetError (hcan);
	//
	// Indicate to the application
	// that there is a can port status update pending.
	CP_Set_SUP (true);
	//
	/* Check Flag that was set in ISR and Clear error bit*/
	//  - These return values are mapped to the C505 LEC codes
	if (err & HAL_CAN_ERROR_STF)
	{
		/* Set CAN error code to Stuff error */
		can_LEC_error = CH_LEC_Stuff;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_STF;
	}
	else if (err & HAL_CAN_ERROR_FOR)
	{
		/* Set CAN error code to Form error */
		can_LEC_error = CH_LEC_Form;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_FOR;
	}
	else if (err & HAL_CAN_ERROR_ACK)
	{
		/* Set CAN error code to Acknowledgement error */
		can_LEC_error = CH_LEC_Ack;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_ACK;
	}
	else if (err & HAL_CAN_ERROR_BR)
	{
		/* Set CAN error code to Bit recessive error */
		can_LEC_error = CH_LEC_BitRec;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_BR;
	}
	else if (err & HAL_CAN_ERROR_BD)
	{
		/* Set CAN error code to Bit Dominant error */
		DSC_Writes (DSC_LEVEL_INFO, "Bit Dominant Error!\n\r");
		can_LEC_error = CH_LEC_BitDom;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_BD;
	}
	else if (err & HAL_CAN_ERROR_CRC)
	{
		/* Set CAN error code to CRC error */
		can_LEC_error = CH_LEC_Crc;
		hcan->ErrorCode &= ~HAL_CAN_ERROR_CRC;
	}
}

/////////////////////////////
// Internal utility Functions
//

// Set CAN Filter Properties
// Filter must be enabled to pass frames to Hardware FIFO
static HAL_StatusTypeDef CH_Set_Filter_Params (void)
{
	CAN_FilterTypeDef filtercan = { 0 };
	HAL_StatusTypeDef err		= HAL_OK;
	uint32_t		  bank		= 0;
	unsigned char	  id		= DeviceNetObjectRAM.macId;
	//
	// common filter params
	filtercan.FilterActivation	   = CAN_FILTER_ENABLE;
	filtercan.FilterFIFOAssignment = CAN_FILTER_FIFO0; // only use fifo 0
	filtercan.FilterScale		   = CAN_FILTERSCALE_16BIT;
	filtercan.SlaveStartFilterBank = 0; // ignored. Only one can bus on STMF0xx
	//
	// SPECIAL CASE
	// Need to know if we are autobauding so can
	// Accept all frames in autobaud mode
	if (ABAUD_ENABLED == DeviceNetObjectRAM.bAutoBaud)
	{
		filtercan.FilterMode	   = CAN_FILTERMODE_IDMASK;
		filtercan.FilterBank	   = 0; // 0 is first bank
		filtercan.FilterIdHigh	   = 0; // accept all as 'dont care'
		filtercan.FilterMaskIdHigh = 0;
		filtercan.FilterIdLow	   = 0;
		filtercan.FilterMaskIdLow  = 0;
		//
		// Set in CAN registers
		return HAL_CAN_ConfigFilter (&hcan1, &filtercan);
	}
	//
	// Set values as used in original C505 UAR1 and UAR0 registers in dn_init.c
	// to set the hardware to ignore all packets we dont want to process
	// The software layer will sort into the proper C505 RAM 'register'
	// This will reduce CAN ISR activity and reduce processor load.
	//
	// C505 reference: UAR0 is MSBs of the 11 bit identifier. UAR1 3 MSB bits are LSB of identifier
	// This nicely maps to our STM32 16 bit filter registers
	// In general these are fully defined (exact match with no dont care bits) unless noted
	// MSG_OBJECT                   [UAR0]    [UAR1]
	// RCV_COS_ACK_MESSAGE_OBJECT   [x80 + id][x40] Grp2 Msg Type: 2
	// RCV_EXPLICIT_MESSAGE_OBJECT  [x80 + id][x80] Grp2 Msg Type: 4
	// RCV_POLL_MESSAGE_OBJECT      [x80 + id][xA0] Grp2 Msg Type: 5
	// RCV_UNCNNCTD_MESSAGE_OBJECT  [x80 + id][xC0] Grp2 Msg Type: 6
	//
	filtercan.FilterMode	   = CAN_FILTERMODE_IDLIST;		// all are ID (no mask)
	filtercan.FilterBank	   = bank++;					// Post-increment start at 0
	filtercan.FilterIdHigh	   = ((0x80 + id) << 8) + 0x40; // RCV_COS_ACK_MESSAGE_OBJECT;
	filtercan.FilterMaskIdHigh = ((0x80 + id) << 8) + 0x80; // RCV_EXPLICIT_MESSAGE_OBJECT
	filtercan.FilterIdLow	   = ((0x80 + id) << 8) + 0xA0; // RCV_POLL_MESSAGE_OBJECT
	filtercan.FilterMaskIdLow  = ((0x80 + id) << 8) + 0xC0; // RCV_UNCNNCTD_MESSAGE_OBJECT
	//
	err = HAL_CAN_ConfigFilter (&hcan1, &filtercan);
	if (err != HAL_OK)
		return err;
	//
	// RCV_DUP_MAC_MESSAGE_OBJECT   [x80 + id][xE0] Grp2 Msg Type: 7
	// set up the one RX group 4 message objects. is only used for fault recovery
	// We will always pass it in the hardware
	// RCV_COMM_FLTD_MSG_OBJ [xFD][xA0] Communication Faulted Request Message
	//
	filtercan.FilterMode	   = CAN_FILTERMODE_IDLIST;
	filtercan.FilterBank	   = bank++;					// Post-increment
	filtercan.FilterIdHigh	   = ((0x80 + id) << 8) + 0xE0; // RCV_DUP_MAC_MESSAGE_OBJECT
	filtercan.FilterIdLow	   = 0xFDA0;					// RCV_COMM_FLTD_MSG_OBJ
	filtercan.FilterMaskIdHigh = 0xFFFF;					// dont let anything through other filter
	filtercan.FilterMaskIdLow  = 0xFFFF;
	//
	err = HAL_CAN_ConfigFilter (&hcan1, &filtercan);
	if (err != HAL_OK)
		return err;
	//
	// RCV_STROBE_MESSAGE_OBJECT    [x80     ][x00] Grp2 Msg Type: 0
	// - Note strobe msg uses master's MAC, so MAC bits [8:3] must be dont care (DN CIP 3-46)
	// - We will use a mask mode here (0=Dont care, 1=match)
	//
	filtercan.FilterMode	   = CAN_FILTERMODE_IDMASK; // use mask
	filtercan.FilterBank	   = bank++;				// Post-increment
	filtercan.FilterIdHigh	   = (0x80 << 8);			// RCV_DUP_MAC_MESSAGE_OBJECT
	filtercan.FilterMaskIdHigh = 0xC0FF;				// 0=dont care, 1=match
	filtercan.FilterIdLow	   = 0xFFFF;				// dont let anything through other filter
	filtercan.FilterMaskIdLow  = 0xFFFF;
	//
	return HAL_CAN_ConfigFilter (&hcan1, &filtercan);
}

static CH_Error_t CH_Frame_to_HAL_Frame (CAN_TxHeaderTypeDef *halHeader, uint8_t *halData, CH_Frame_t *chFrame)
{
	CH_Assert (chFrame);
	CH_Assert (halHeader);
	CH_Assert (halData);
	CH_Error_t ret = CH_ERROR_None;
	//
	// No equivalent for these in the CH_Frame
	// Set to what we are calling 'defaults' at this time
	halHeader->RTR				  = CAN_RTR_DATA;
	halHeader->TransmitGlobalTime = DISABLE;
	//
	if (chFrame->extended == true)
	{
		halHeader->IDE	 = CAN_ID_EXT;
		halHeader->ExtId = chFrame->id;
	}
	else
	{
		halHeader->IDE	 = CAN_ID_STD;
		halHeader->StdId = chFrame->id;
	}
	//
	// Can data is 8 bytes
	if ((chFrame->length <= 8) && (chFrame->length >= 0))
	{
		halHeader->DLC = chFrame->length;
		//
		memset (halData, 0, sizeof (chFrame->data));
		memcpy (halData, chFrame->data, chFrame->length);
	}
	else
	{
		// bad header data length.
		DSC_Writes (DSC_LEVEL_INFO, "BAD TX header data length");
		ret = CH_ERROR_Packet;
	}
	return ret;
}

static CH_Error_t HAL_Frame_to_CH_Frame (CH_Frame_t *chFrame, CAN_RxHeaderTypeDef *halHeader, uint8_t *halData)
{
	CH_Assert (chFrame);
	CH_Assert (halHeader);
	CH_Assert (halData);
	int ret = CH_ERROR_None;
	//
	if (halHeader->IDE == CAN_ID_STD)
	{
		chFrame->id		  = halHeader->StdId;
		chFrame->extended = false;
	}
	else if (halHeader->IDE == CAN_ID_EXT)
	{
		chFrame->id		  = halHeader->ExtId;
		chFrame->extended = true;
	}
	else
	{
		// bad ID
		DSC_Writes (DSC_LEVEL_INFO, "BAD RX Frame ID");
		ret = CH_ERROR_Packet;
	}
	//
	// Can data is 8 bytes
	if ((halHeader->DLC <= 8) && (halHeader->DLC >= 0))
	{
		chFrame->length = halHeader->DLC;
		//
		memset (chFrame->data, 0, sizeof (chFrame->data));
		memcpy (chFrame->data, halData, halHeader->DLC);
	}
	else
	{
		// bad header data length
		DSC_Writes (DSC_LEVEL_INFO, "BAD RX header data length");
		ret = CH_ERROR_Packet;
	}
	return ret;
}
