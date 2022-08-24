#include <c505c.h>
#include <dn_def.h>
#include <dn_dnobj.h>
#include <dn_eeprm.h>
#include <dn_tmobj.h>
#include <dn_uiobj.h>
#include <std.h>
#include <string.h>
#include "i2c_eeprom.h"

// take out in end er
#define NO_EE_REV_CHK

// Define EEPROM Status Structure
//   All values are staged in a RAM table and then written individually
//   Write speed may be improved by doing bulk (page) writes, but
//   compatibility issues may arise if different devices have different page sizes.
// The CurrentCRC variable is kept for compatibility.
typedef struct
{
	BOOL		 bEEPROMInitialized;
	unsigned int CurrentCRC;
	uint8_t		 Global_Pending;		 // Any value need written
	uint8_t		 EE_Ram[EEPROM_SIZE];	 // our ram values
	uint8_t		 W_Pending[EEPROM_SIZE]; // value needs written
} EEPROMObjectStructure;

static EEPROMObjectStructure EEPROMObjectRAM;

//**********************************************************************
// Function		EEPROMObjectInit
//
// Description	Initializes the EEPROM at power up.  Virgin check is done.
//
// Inputs		none
//
// Outputs	none
//
// Edit History
//	[0]	11jun99	dsw	Already existed
//	[1]	12Jun99	dsw	Clean up for 505C
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************

extern void ParamEEReset (void);

void EEPROMObjectInit (void)
{
	unsigned int RAMChecksum = 0;

	// Set all our pending flags to zero
	memset (EEPROMObjectRAM.W_Pending, 0, sizeof (EEPROMObjectRAM.W_Pending));
	EEPROMObjectRAM.Global_Pending = 0;

	// Initialize RAM - Read out our entire eeprom
	if (EE_HAL_Read (0, EEPROMObjectRAM.EE_Ram, EEPROM_SIZE) != EE_SUCCESS)
	{
		// If we cannot read our EEPROM, we are in powerup fail mode
		// Dont check anything else, just return.
		UIObjectRAM.cHealthState		   = POWERUP_FAIL;
		EEPROMObjectRAM.bEEPROMInitialized = FALSE;
		return;
	}

	// Get our checksum stored in EEPROM
	RAMChecksum = EEPROMObjectRAM.EE_Ram[EE_CS_Lo] + (EEPROMObjectRAM.EE_Ram[EE_CS_Hi] << 8);

	// compute checksum value and saves to "CurrentCRC" variable
	EEPROMObjectComputeCSFromEE ();

	// IF Calculated checksum does not match RAM, we check init status
	// OR IF calculated checksum is zero, we need to initialize.
	if ((EEPROMObjectRAM.CurrentCRC != RAMChecksum) || (EEPROMObjectRAM.CurrentCRC == 0))
	{
		// check if EEPROM is configured, and set Initialized status
		EEPROMObjectRAM.bEEPROMInitialized = (NOT_A_VIRGIN == EEPROMObjectRead (EE_Virgin));

		if (!EEPROMObjectRAM.bEEPROMInitialized)
		// if (1) // HACK to reset eeprom no matter what (even if not a virgin)
		{
			// this should only run if a module does not have
			// a properly initialized EEPROM.  Most likely production
			// had some kind of problem.

			// make it no longer a virgin
			EEPROMObjectWriteAndUpdate (EE_Virgin, NOT_A_VIRGIN);

			// write EEPROMRevision Value
			EEPROMObjectWriteAndUpdate (EE_Prod_Rev_Maj, EE_REV_MAJ);
			EEPROMObjectWriteAndUpdate (EE_Prod_Rev_Min, 0x00); // 0 will be special case

			// load default macid, baudrate, etc...
			DeviceNetObjectFactoryDefaults ();
			DeviceNetObjectSaveEEPROM ();

			// load defaults for the application
			ParamEEReset ();

			if (EEPROMObjectRead (EE_Virgin) > 0x20)
				EEPROMObjectWrite (EE_Name_Str_Len, 0x20);

			// Configure EEPROM Size
			EEPROMObjectWrite (EE_End_Hi, ((EEPROM_SIZE >> 8) & 0xff));
			EEPROMObjectWrite (EE_End_Lo, (EEPROM_SIZE & 0xff));

			// compute new checksum for data area - value written back to EEPROM
			EEPROMObjectComputeCSFromEE (); // sets value in structure
			EEPROMObjectWriteCRC ();		// write value to EEPROM memory

			EEPROMObjectRAM.bEEPROMInitialized = TRUE;
		}
		else
		{
			// checksum failed - want red LED and to loop forever (for now)
			UIObjectRAM.cHealthState = POWERUP_FAIL;
			// don't need to return here
		}
	}
	else
	{
		EEPROMObjectRAM.bEEPROMInitialized = TRUE;
	}

	// if we are initialized successfully, lets do a blocking write to eeprom
	if (EEPROMObjectRAM.bEEPROMInitialized == TRUE)
	{
		EEPROMObjectWriteRAMtoEE (1);
	}

#ifndef NO_EE_REV_CHK
	// Check EEPROM REVISION
	if (EE_REV_MAJ != EEPROMObjectRead (EE_Prod_Rev_Maj))
		UIObjectRAM.cHealthState = POWERUP_FAIL;
#endif
}

void EEPROMObjectWriteCRC (void)
{
	// save checksum in EEPROM
	EEPROMObjectWrite (EE_CS_Lo, EEPROMObjectRAM.CurrentCRC);
	EEPROMObjectWrite (EE_CS_Hi, EEPROMObjectRAM.CurrentCRC >> 8);
}

// Write any pending values to EEPROM
// "blocking write" is a flag indicating if we block the thread while waiting for the
// write to finish. Normally we do not want to block
//  We can block if doing critical writes (aka before reboot)
uchar EEPROMObjectWriteRAMtoEE (uchar block)
{
	// uint16_t	  RAMChecksum = 0;
	ee_err_status ee_error = EE_SUCCESS;
	int			  i		   = 0;

	// only do this if we are initialized, which should always be the case
	if (EEPROMObjectRAM.bEEPROMInitialized != TRUE)
	{
		EE_Error ("Write to EE before Init.");
		return 1;
	}

	/*
	// Verify checksum with RAM checksum values. This could indicate a problem
	EEPROMObjectComputeCSFromEE (); // sets value in structure
	RAMChecksum = EEPROMObjectRAM.EE_Ram[EE_CS_Lo] + (EEPROMObjectRAM.EE_Ram[EE_CS_Hi] << 8);

	if ((uint16_t)EEPROMObjectRAM.CurrentCRC != RAMChecksum)
	{
		DSC_Writes (DSC_LEVEL_INFO, "ERROR:RAM Checksum did not Match!\n\r");
		return 2;
	}
	*/

	// Do any values need written?
	if (EEPROMObjectRAM.Global_Pending == 1)
	{
		EEPROMObjectRAM.Global_Pending = 0;
		//
		// Write all pending values.
		//   We could get clever here and do consecutive writes if we need to
		//   But we would have to consider the EEPROM page size.
		// Loop through EEPROM begining at
		for (i = 0; i < EEPROM_SIZE; i++)
		{
			if (EEPROMObjectRAM.W_Pending[i])
			{
				// check if we are ready to send. Either blocking or nonblocking
				EE_wait_until_ready (block);

				// clear pending flag once ready.
				// note that we much clear AFTER the wait, as another thread could have
				//   updated our ram/pending table.
				EEPROMObjectRAM.W_Pending[i] = 0;

				// write to EEPROM over i2c
				if ((ee_error = EE_HAL_WriteOne ((uint16_t)i, EEPROMObjectRAM.EE_Ram[i]) != 0))
				{
					// error. Something is wrong, exit for now.
					// reassert the pending flag so we can try again later.
					EEPROMObjectRAM.W_Pending[i] = 1;
					return 3;
				}
			}
		} // END for all EEPROM_SIZE
	}	  // END Global Pending == 1
	return 0;
}

//**********************************************************************
// Function		EEPROMObjectInitialized?
//
// Description	Initializes the EEPROM at power up.  Virgin check is done.
//
// Inputs		none
//
// Outputs	BOOL
//
// Edit History
//	[0]	11jun99	dsw	Already existed
//	[1]	11Jun99	dsw	Clean up for 505C
//
//             Copyright (c) 1999 Allen-Bradley Co.
//**********************************************************************
BOOL EEPROMObjectInitialized (void)
{
	return (TRUE == EEPROMObjectRAM.bEEPROMInitialized);
}

//***************************************************************************
//
//  Function: write_buffered_vals_to_EE()
//
//  This function called when data was unable to be written to EE from an
//     intlevel request.  Now we're in the foreground, and can do it.
//  ASSUMING that no other int level requests are coming.  (GOOD ASSUMPTION)
//
//  Inputs:     unsigned int offset   Offset from the start of the
//              EE Prom to the place where the data is to be written.
//
//  Revision History:
//     020700 - CRSchumaker
//
//****************************************************************************

//***************************************************************************
//
//  Function: EEPROMObjectWAndU_INT()
//
//  This function will write a single byte of data to the serial eeprom
//     and update the CRC.  This is only called from an interrupt routine.
//
//  Inputs:     unsigned int offset   Offset from the start of the
//              EE Prom to the place where the data is to be written.
//
//  Revision History:
//     020700 - CRSchumaker
//
//****************************************************************************
void EEPROMObjectWAndU_INT (unsigned int offset, uchar newval)
{
	//
	// Just call into our object write. This is only writing RAM
	EEPROMObjectWriteAndUpdate (offset, newval);
}

//***************************************************************************
//
//  Function: EEPROMObjectWriteAndUpdate()
//
//  This function will write a single byte of data to the serial eeprom
//     and update the CRC.
//
//  Inputs:     unsigned int offset   Offset from the start of the
//              EE Prom to the place where the data is to be written.
//
//  Outputs:    uchar Error   returns 0 if success
//                            returns 1 if address error
//                            returns 0xff if  Write failed.
//
//  Revision History:
//     072799 - CRSchumaker
//
//****************************************************************************

void EEPROMObjectWriteAndUpdate (unsigned int offset, uchar newval)
{
	// check our offset
	if (offset > EEPROM_SIZE)
	{
		EE_Error ("EE Write Out of Bounds.");
		return;
	}

	// if no change, do nothing
	if (EEPROMObjectRAM.EE_Ram[offset] == newval)
	{
		return;
	}

	// adjust the CRC value
	EEPROMObjectRAM.CurrentCRC -= (unsigned int)EEPROMObjectRAM.EE_Ram[offset];
	EEPROMObjectRAM.CurrentCRC += (unsigned int)newval;

	// write new CRC
	EEPROMObjectWrite (EE_CS_Lo, (unsigned int)EEPROMObjectRAM.CurrentCRC);
	EEPROMObjectWrite (EE_CS_Hi, (unsigned int)(EEPROMObjectRAM.CurrentCRC >> 8));

	// write new data value
	EEPROMObjectWrite (offset, newval);
}
//***************************************************************************
//
//  Function: EEPROMObjectWrite()
//
//  This function will write a single byte of data to the serial eeprom
//
//  Inputs:     unsigned int offset   Offset from the start of the
//              EE Prom to the place where the data is to be written.
//
//  Outputs:    uchar Error   returns 0 if success
//                            returns 1 if address error
//                            returns 0xff if  Write failed.
//
//  Revision History:
//   Feb 17, 1998 Steve Blech   Ported from 1756 Non Isolated Analog
//	[1]	11Jun99	dsw	Clean up
//   [2]  15June	dsw	added Wait call at end (cleaned for 505C)
//	[3]  27Jul00   dsw  finally added call to servicetimer while wait for write
//
//****************************************************************************
uchar EEPROMObjectWrite (unsigned int offset, uchar val)
{
	// check our offset
	if (offset > EEPROM_SIZE)
	{
		EE_Error ("EE Write Out of Bounds.");
		return ERR_LENGTH;
	}
	//
	// Insert our new value and indicate our write is pending
	EEPROMObjectRAM.EE_Ram[offset]	  = val;
	EEPROMObjectRAM.W_Pending[offset] = 1;
	EEPROMObjectRAM.Global_Pending	  = 1;

	return DN_EE_SUCCESS;
}

//**************************************************************************
//
// Function: EEPROMObjectDoneWriting(void)
//
// Check whether eeprom device is done with a write command.
//
// Inputs:     none
//
// Outputs:    uchar done   0  Not done.
//                          1  done writing
//
// Revision History:
//   Mar 17, 1999  CRSchumaker   Created
//
//*************************************************************************

//***************************************************************************
//
// Function: ee_write_byte(uchar data)
//
// This function will write one byte of data to the eeprom. This function
// assumes that clock is hi and the data port is out. clock is returned hi
//
// Inputs:     uchar data      Byte of Data to be written.
//
// Outputs:    uchar err_code   ERR_WRITE_FAIL  Failure to write byte.
//                             SUCCESS         Operation Succeded.
//
// Revision History:
//   Feb 17, 1998  Steve Blech   Ported from 1756 Non Isolated Analog
//   Jun 15, 1999     dsw        Cleaned up for 505C
//
//*************************************************************************

//**************************************************************************
//
// Function: EEPROMObjectRead(uchar offset)
//
// This function will read the specified amount of data from the eeprom. The
// will take advantage of the sequential read mode of the serial eeprom
//
// Inputs:     USINT  offset    Offset from the start of the eeprom
//
// Outputs:    INT Error_Code   ERR_WRITE_FAIL  Write failed.
//                              SUCCESS         Data read
//
// Revision History:
//  Feb 17, 1998 Steve Blech   Ported from 1756 Non Isolated Analog
//  Jun 15, 1999    dsw        Cleaned up for 505C
//  Aug  9, 1999    crs        no more status value - not using cpvaluelow
//
//*************************************************************************

uchar EEPROMObjectRead (unsigned int offset)
{
	// original implementation doesnt have method for returning error
	// just send a zero
	if (offset > EEPROM_SIZE)
	{
		EE_Error ("EE Read Out of Bounds.");
		return 0;
	}
	//
	// Just return our RAM value
	return EEPROMObjectRAM.EE_Ram[offset];
}

//*************************************************************************
//
// Function: ee_read_byte(void)
//
// This function will read one byte of data from the eeprom.
// This function assumes that the eeprom internal address has been set up
// and the clock line is high. An acknowledge is retuned after the data
// has been read. The clock line is returned hi, and the data port out
//
// Inputs:     None
//
// Outputs:    uchar    data byte that was read from eeprom.
//
// Revision History:
//     Feb 17, 1998     Steve Blech  Function Created.
//     Jun 15, 1999     dsw        Cleaned up for 505C
//
//***********************************************************************

//************************************************************************
//
// Function: ee_start_cmd(void)
//
// This function will issue a start command to the eeprom so that it
// will begin accepting commands. The start command is a high-to-low
// transition on the data line while the clock line is high. This
// function assumes the port lines are in the right direction
//
// Inputs:     None
//
// Outputs:    None
//
// Revision History:
//   Feb 17, 1998  Steve Blech   Ported from 1756 Non Isolated Analog
//   Jun 15, 1999     dsw        Cleaned up for 505C
//
//***********************************************************************

//************************************************************************
//
// Function: ee_stop_cmd(void)
//
// This function will issue the stop command to the eeprom causing it to
// return to the idle state. The stop coondition is defined as a lo-to-hi
// transition on the data line while the clock line is high.
// The clock and data lines will be left hi.
//
// Inputs:     None
//
// Outputs:    None
//
// Revision History:
//   Feb 17, 1998  Steve Blech   Ported from 1756 Non Isolated Analog
//   Jun 15, 1999     dsw        Cleaned up for 505C
//
//************************************************************************

//*************************************************************************
//
// Function: ee_delay(uchar count)
//
// This function will implement the specified delay.
//
// Inputs:     uchar count  Number of counts to delay. Delays are typically
//             5 uSec for standard IIC, or 1.5 uSec for 400 kHz parts
//
// Outputs:    None
//
// Revision History:
//     Feb 17, 1998     Steve Blech  Ported from 1756 Non Isolated Analog
//     Jun 17, 1999     dsw  when count=0 miminum clock is 6 us on
//                           505C @20MHz (checked with scope)
//
//************************************************************************

//*************************************************************************
//
// Function: EEPROMObjectComputeCSFromEE()
//
// This function computes the Check Sum for the EEPROM to help in verifying
// data.  The Check Sum is stored in EEPROM
//
// Inputs:     none
//
// Outputs:    checksum is stored in EEPROM
//
// Revision History:
//   [1]  15June	dsw	created
//	[2]	18Jan01	dsw	changed how end of eeprom is determeined
//
//*************************************************************************

void EEPROMObjectComputeCSFromEE ()
{
	uchar		 temp;
	unsigned int LCV;
	unsigned int EndEEPROM;
	unsigned int CheckSum = 0; // 16 bit checksum

	temp	  = EEPROMObjectRead (EE_End_Hi); // get high byte of end of range
	EndEEPROM = (unsigned int)temp;
	EndEEPROM <<= 8; // shift into high byte
	EndEEPROM += EEPROMObjectRead (EE_End_Lo);

	// Check if our end is equal to our size
	// If our end does not equal eeprom size, init it to the correct size.
	if (EndEEPROM != EEPROM_SIZE)
	{
		EEPROMObjectWrite (EE_End_Hi, (EEPROM_SIZE >> 8) & 0xff); // set end of EEPROM
		EEPROMObjectWrite (EE_End_Lo, EEPROM_SIZE & 0xff);
		EndEEPROM = EEPROM_SIZE; // variable set to new value too
	}

	// Loop through EEPROM begining at
	for (LCV = EE_FirstCS; LCV < EndEEPROM; LCV++)
	{
		// get byte and add it
		CheckSum = CheckSum + EEPROMObjectRead (LCV);
	}

	EEPROMObjectRAM.CurrentCRC = CheckSum;
}

//**************************************************************************
//
// Function: BumpCCV(void)
//
// Increments CCV in EEPROM
//
//
// Revision History:
//   [1]	17Jul99	dsw	Created
//
//*************************************************************************

void BumpCCV (void) {}
