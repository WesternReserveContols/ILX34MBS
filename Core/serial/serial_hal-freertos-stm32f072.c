/*
 * serial_hal.c
 *
 *  Created on: Feb 13, 2020
 *      Author: wrc
 */
#include "bsp_main.h"
#include "dsc.h"
#include "serial_hal.h"
#include "serial_config.h"

/////////////////////////////
// Variables

// UART Handle Variable
//  This is used by this module and the ISR to address the UART Registers
UART_HandleTypeDef huart_main = { USART1 };
//
// user setting parameters.
//  Init to our defaults 8N1 9600
static int		sh_flag_7N2	   = 0; // flag for special framing case where we insert stop bit
static uint32_t sh_baud		   = 9600;
static uint32_t sh_word_length = UART_WORDLENGTH_8B;
static uint32_t sh_stop_bits   = UART_STOPBITS_1;
static uint32_t sh_parity	   = UART_PARITY_NONE;

////////////////////////////
// Internal Function Prototypes

void Serial_RX_ISR (void);
void Serial_TX_ISR (void);

// init function
serial_status SH_Init (void)
{
	// Disable Interrupts
	// We may be active if this is a reset.
	// All interrupts will be Re-enabled by the Init functions
	SH_Disable_Interrupts ();
	//
	// Disable and then reset status and flags
	__HAL_UART_DISABLE (&huart_main);
	//
	// not used, clear to known state
	__HAL_UART_CLEAR_FLAG (&huart_main, UART_FLAG_CTS);
	__HAL_UART_CLEAR_FLAG (&huart_main, UART_FLAG_LBDF);
	//
	// Reset Transmission Complete
	__HAL_UART_CLEAR_FLAG (&huart_main, UART_FLAG_TC);
	//
	// Reset all errors and clear RX not empty flag
	__HAL_UART_CLEAR_PEFLAG (&huart_main);
	//
	// Clear any pending interrupts
	HAL_NVIC_ClearPendingIRQ (USART1_IRQn);
	//
	// Set UART3 Handle properties
	huart_main.Instance					   = USART1; // register offset address
	huart_main.Init.BaudRate			   = sh_baud;
	huart_main.Init.WordLength			   = sh_word_length;
	huart_main.Init.StopBits			   = sh_stop_bits;
	huart_main.Init.Parity				   = sh_parity;
	huart_main.Init.Mode				   = UART_MODE_TX_RX;	   // Full Duplex
	huart_main.Init.HwFlowCtl			   = UART_HWCONTROL_NONE;  // Never Used
	huart_main.Init.OverSampling		   = UART_OVERSAMPLING_16; // Our Default
	huart_main.Init.OneBitSampling		   = UART_ONE_BIT_SAMPLE_DISABLE;
	huart_main.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init (&huart_main) != HAL_OK)
	{
		Debug_ReportError (DEBUG_ERR_SERIAL, "HAL_UART_Init", DEBUG_STOP);
		return SER_Fail;
	}

	DSC_Writes (DSC_LEVEL_INFO, "Init UART\n\r");

	// TX interrupts only on TX complete (instead of TX register empty)
	//  This is becuase main app will trigger TX/RX enable line on this interrupt
	//  so we must only service when the full transfer is complete.
	//  Could change this, but this is simplest, if slower
	__HAL_UART_ENABLE_IT (&huart_main, UART_IT_TC);

	// RX interrupt is on RX data register not empty.
	__HAL_UART_ENABLE_IT (&huart_main, UART_IT_RXNE);

	// Parity Error Interrupt
	__HAL_UART_ENABLE_IT (&huart_main, UART_IT_PE);

	// Error Interrupts
	__HAL_UART_ENABLE_IT (&huart_main, UART_IT_ERR);

	// Enable Interrupts
	SH_Enable_Interrupts ();

	return SER_Success;
}

// Here we translate the ASCII structure to our serial hardware settings.
//  This simply sets internal variables in this file and SH_Init must be run
//  to enact changes.
//  If a SH_Init is run before setting with this function, our defaults are used
serial_status SH_Set_Parameters (void)
{
	serial_status err = SER_Success;
	//
	// We are touching Ascii shared variable
	SH_Disable_Interrupts ();
	//
	if (Ascii.flowcontrol != 0)
	{
		// flow control not supported, invalid state
		// Just warn here, no error
		DSC_Writes (DSC_LEVEL_INFO, "Flow Control not Supported\n\r");
	}
	//
	// framing define will set Data, parity, and stop bits
	switch (Ascii.Framing)
	{
	case FRAME_7N2:
		// special case for our driver
		// We will need to mask off one stop bit out of the 8 data bits
		DSC_Writes (DSC_LEVEL_INFO, "7N2\r\n");
		sh_flag_7N2	   = 1;
		sh_word_length = UART_WORDLENGTH_8B; //TODO changed from UART_WORDLENGTH_8B; to UART_WORDLENGTH_7B to make it workable
		sh_stop_bits   = UART_STOPBITS_1; //TODO changed from UART_STOPBITS_1; to UART_STOPBITS_2 to make it workable
		sh_parity	   = UART_PARITY_NONE;
		break;
	case FRAME_7E1:
		DSC_Writes (DSC_LEVEL_INFO, "7E1\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_1;
		sh_parity	   = UART_PARITY_EVEN;
		break;
	case FRAME_7O1:
		DSC_Writes (DSC_LEVEL_INFO, "7O1\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_1;
		sh_parity	   = UART_PARITY_ODD;
		break;
	case FRAME_8N1:
		DSC_Writes (DSC_LEVEL_INFO, "8N1\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_1;
		sh_parity	   = UART_PARITY_NONE;
		break;
	case FRAME_8N2:
		DSC_Writes (DSC_LEVEL_INFO, "8N2\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_2;
		sh_parity	   = UART_PARITY_NONE;
		break;
	case FRAME_8E1:
		DSC_Writes (DSC_LEVEL_INFO, "8E1\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_9B;
		sh_stop_bits   = UART_STOPBITS_1;
		sh_parity	   = UART_PARITY_EVEN;
		break;
	case FRAME_8O1:
		DSC_Writes (DSC_LEVEL_INFO, "8O1\r\n");
		sh_word_length = UART_WORDLENGTH_9B;
		sh_stop_bits   = UART_STOPBITS_1;
		sh_parity	   = UART_PARITY_ODD;
		break;
	case FRAME_7E2:
		DSC_Writes (DSC_LEVEL_INFO, "7E2\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_2;
		sh_parity	   = UART_PARITY_EVEN;
		break;
	case FRAME_7O2:
		DSC_Writes (DSC_LEVEL_INFO, "7O2\r\n");
		sh_flag_7N2	   = 0;
		sh_word_length = UART_WORDLENGTH_8B;
		sh_stop_bits   = UART_STOPBITS_2;
		sh_parity	   = UART_PARITY_ODD;
		break;
	default:
		// invalid Parameter
		Debug_ReportError (DEBUG_ERR_SERIAL, "Invalid Frame request", DEBUG_STOP);
		err = SER_Fail;
		break;
	}
	//
	// baudrate is used as an offset that maps to the BaudDiv array.
	// just map it here to our desired baud rate

	switch (BaudDiv[Ascii.BaudRate])
	{
	case BAUD12:
		DSC_Writes (DSC_LEVEL_INFO, "1200b\n\r");
		sh_baud = 1200;
		break;
	case BAUD24:
		DSC_Writes (DSC_LEVEL_INFO, "2400b\n\r");
		sh_baud = 2400;
		break;
	case BAUD48:
		DSC_Writes (DSC_LEVEL_INFO, "4800b\n\r");
		sh_baud = 4800;
		break;
	case BAUD96:
		DSC_Writes (DSC_LEVEL_INFO, "9600b\n\r");
		sh_baud = 9600;
		break;
	case BAUD19:
		DSC_Writes (DSC_LEVEL_INFO, "19200b\n\r");
		sh_baud = 19200;
		break;
	case BAUD38:
		DSC_Writes (DSC_LEVEL_INFO, "38400\n\r");
		sh_baud = 38400;
		break;
	default:
		// Invalid parameter
		Debug_ReportError (DEBUG_ERR_SERIAL, "Invalid Baud", DEBUG_STOP);
		err = SER_Fail;
		break;
	}
	// We are touching Ascii shared variable
	SH_Enable_Interrupts ();

	return err;
}

// Disable UART Interrupts
void SH_Disable_Interrupts (void)
{
	HAL_NVIC_DisableIRQ (USART1_IRQn);
}

// Enable UART Interrupts
void SH_Enable_Interrupts (void)
{
	HAL_NVIC_EnableIRQ (USART1_IRQn);
}

// This is run when any interrupt is fired.
// Code adapted from HAL library ISR.
void SH_IRQ (void)
{
	if (huart_main.Instance != USART1)
	{
		return;
	}

	uint32_t isrflags	= READ_REG (huart_main.Instance->ISR);
	uint32_t cr1its		= READ_REG (huart_main.Instance->CR1);
	uint32_t cr3its		= READ_REG (huart_main.Instance->CR3);
	uint32_t errorflags = 0x00U;

	/* If no error, check if RX interrupt and call receive function*/
	errorflags = (isrflags & (uint32_t) (USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver -------------------------------------------------*/
		if (((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			Serial_RX_ISR();
			return;
		}
	}

	/* If some errors occur */
	if ((errorflags != RESET)
		&& (((cr3its & USART_CR3_EIE) != RESET) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
	{
		/* UART parity error interrupt occurred ----------------------------------*/
		if (((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		{
			__HAL_UART_CLEAR_FLAG (&huart_main, UART_CLEAR_PEF);
			DSC_Writes (DSC_LEVEL_INFO, "Parity ERR\r\n");
			Ascii.Status |= PARITY_ERROR_BIT;
		}

		/* UART noise error interrupt occurred -----------------------------------*/
		if (((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_FLAG (&huart_main, UART_CLEAR_NEF);
			DSC_Writes (DSC_LEVEL_INFO, "Noise ERR\r\n");
		}

		/* UART frame error interrupt occurred -----------------------------------*/
		if (((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_FLAG (&huart_main, UART_CLEAR_FEF);
			DSC_Writes (DSC_LEVEL_INFO, "Frame ERR\r\n");
		}

		/* UART Over-Run interrupt occurred --------------------------------------*/
		if (((isrflags & USART_ISR_ORE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_FLAG (&huart_main, UART_CLEAR_OREF);
			DSC_Writes (DSC_LEVEL_INFO, "Overrun ERR\r\n");
		}

		/* we still want to call our receive function if errors --------------------------*/
		// This is how original code worked - we only set parity error flag and still try to RX char
		// Get anything pending to RX if it is available still

		Serial_RX_ISR();

		return;

	} /* End if some error occurs */

	/* UART Transmit Complete Interrupt--------------------------------------------*/
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		__HAL_UART_CLEAR_FLAG (&huart_main, UART_FLAG_TC);
		Serial_TX_ISR ();
		return;
	}
}

// send single char
serial_status SH_Put_Char (char c)
{
	//
	// May call from interrupt or non-interrupt
	SH_Disable_Interrupts ();
	//
	// We must check that the TX buffer register is empty
	//  TXE is only signaling the TX buffer is empty, NOT that the tx is complete on the line
	// Check TX Empty flag. Even If Transmit is not complete, we can still put a char in TX buff
	if (__HAL_UART_GET_FLAG (&huart_main, UART_FLAG_TXE) == RESET)
	{
		// TX buffer not empty. We cannot do anything.
		// Exit since this may be called from an interrupt
		SH_Enable_Interrupts ();
		return SER_Fail;
	}
	//
	// Put char in buffer
	// If we are in our special case, handle it
	// else just set register. Hardware handles parity.
	if (sh_flag_7N2 == 1)
	{
		// in our special case, overwrite a stop bit (1) in the MSB position.
		huart_main.Instance->TDR = (c | (uint32_t)0x80);
	}
	else
	{
		// Only ever sending a char data type (even tho we can do 9 bits no parity)
		huart_main.Instance->TDR = (c & (uint32_t)0xFF);
	}
	//
	// May call from interrupt or non-interrupt
	SH_Enable_Interrupts ();
	//
	return SER_Success;
}

// Get char inteded to be called from the interrupt service
serial_status SH_Get_Char_ISR (char *c)
{
	// Check That RX is not empty.
	// Since function is intended only to be called from an ISR, RXNE should usually be SET
	//  unless we get error interrupts - then there may be nothing to get
	if (__HAL_UART_GET_FLAG (&huart_main, UART_FLAG_RXNE) == RESET)
	{
		// Error, nothing to get
		return SER_Fail;
	}
	//
	// First, check our flag for 7N2 special case
	// Else If we have an 8bit length with parity, Mask
	// Else we are always 8 bits.
	if (sh_flag_7N2 == 1)
	{
		// in our special case, we just mask off the MSB (stop bit)
		*c = (char)(huart_main.Instance->RDR & (uint32_t)0x7F);
	}
	else if ((huart_main.Init.Parity != UART_PARITY_NONE) && (huart_main.Init.WordLength == UART_WORDLENGTH_8B))
	{
		// mask off parity bit.
		*c = (char)(huart_main.Instance->RDR & (uint32_t)0x7F);
	}
	else
	{
		*c = (char)(huart_main.Instance->RDR & (uint32_t)0xFF);
	}
	//
	return SER_Success;
}


