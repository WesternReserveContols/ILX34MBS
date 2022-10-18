/**
 * @file module_id.c
 * @author Jason
 * @date 7 Oct 2020
 * @brief This file containt module id related information.
 *
 */

/******************************************************************************

							Online C Compiler.
				Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bsp_main.h"
#include "module_id.h"

void				  Module_ID_Flash_Init (void);
static const uint32_t MODULE_ID_FLASH_ADDRESS = 0x801FFFC;

typedef enum
{
	MODULE_TYPE_ERROR		  = 0,
	MODULE_TYPE_UNINITIALIZED = 0xFFFFFFFF,
	MODULE_TYPE_1734_232	  = 0x00000011,
	MODULE_TYPE_1734_485	  = 0x00001100,
	MODULE_TYPE_1738_232	  = 0x00110000,
	MODULE_TYPE_1738_485	  = 0x11000000,
	MODULE_TYPE_ILX34_MBS232  = 0x00000111,
	MODULE_TYPE_ILX34_MBS485  = 0x00001111
} Module_Type;

static Module_Type Module_Type_Sanitize (Module_Type value)
{
	switch (value)
	{
	case MODULE_TYPE_1734_232:
	case MODULE_TYPE_1734_485:
	case MODULE_TYPE_1738_232:
	case MODULE_TYPE_1738_485:
	case MODULE_TYPE_ILX34_MBS232:
	case MODULE_TYPE_ILX34_MBS485:
	case MODULE_TYPE_UNINITIALIZED:
		return value;
	default:
		return MODULE_TYPE_ERROR;
	}
}

// Not a valid runtime value
bool Module_Type_Permitted (Module_Type type)
{
	if (type == MODULE_TYPE_UNINITIALIZED || type == MODULE_TYPE_ERROR)
		return false;
	return true;
}

void assert_with_message (bool value, const char *message)
{
	if (!value)
		Debug_ReportError (DEBUG_ERR_ASSERT, message, DEBUG_STOP);
}

void assert_permitted_runtime_module_type (Module_Type type, const char *message)
{
	assert_with_message (Module_Type_Permitted (type), message);
}

/*
	Read module ID from flash
	Crashes upon failure
*/
static Module_Type Module_ID_Read_Flash ()
{
	uint32_t *value = (uint32_t *)MODULE_ID_FLASH_ADDRESS;

	return Module_Type_Sanitize (*value);
}

/*
	Check is flash is initialized with any non-default module id value (valid or invalid)
	Crashes upon failure
*/
bool Module_ID_Initialized ()
{
	Module_Type type = Module_ID_Read_Flash ();

	if (type == MODULE_TYPE_UNINITIALIZED)
		return false;
	return true;
}

/**	@brief  Read module ID from IO pins Crashes upon failure
 *	@retval HAL Status
 */
static Module_Type Module_ID_Read_Pins ()
{
	Module_Type type = MODULE_TYPE_UNINITIALIZED;
/* Rick_TEST 8/29/2022
	if (pin_id_1734_232 ())
		type &= MODULE_TYPE_1734_232;

	if (pin_id_1734_485 ())
		type &= MODULE_TYPE_1734_485;

	if (pin_id_1738_232 ())
		type &= MODULE_TYPE_1738_232;

	if (pin_id_1738_485 ())
		type &= MODULE_TYPE_1738_485;
*/
	if (pin_id_34_232 ())
		type &= MODULE_TYPE_ILX34_MBS232;

	if (pin_id_34_485 ())
		type &= MODULE_TYPE_ILX34_MBS485;

	type = Module_Type_Sanitize (type);
	assert_permitted_runtime_module_type (type, "read invalid module id from factory jumpers");

	return type;
}

/*
	Write module ID to flash
	Crashes upon failure
*/
static void Module_ID_Write_Flash (Module_Type type)
{
	assert_permitted_runtime_module_type (type, "attempted to write invalid module id to flash");

	/* manually reset module ID for testing */

	// Rick_TEST 9/2/2022  used for enable writing to flash for writing.
	HAL_FLASH_Unlock ();
	FLASH_EraseInitTypeDef erase;
	erase.NbPages	  = 1;
	erase.PageAddress = 0x801F800;
	erase.TypeErase	  = FLASH_TYPEERASE_PAGES;

	uint32_t PageError = 0;
	HAL_FLASHEx_Erase (&erase, &PageError);
	HAL_FLASH_Lock ();




	HAL_FLASH_Unlock ();
	HAL_FLASH_Program (FLASH_TYPEPROGRAM_WORD, MODULE_ID_FLASH_ADDRESS, (uint32_t)type);
	HAL_FLASH_Lock ();
}

/*
	Verify module ID in flash
	Crashes upon failure
*/
static void Module_ID_Verify_Flash (Module_Type expected_type)
{
	Module_Type actual_type = Module_ID_Read_Flash ();
	assert_with_message (actual_type == expected_type, "verify module id written to flash failed");
}

static void Module_ID_Set_Global_Variables (Module_Type type);
/*
	Figure out which module type this is.
	Crashes upon failure.
*/
void Module_ID_Init ()
{

#ifdef Rick_TEST
	if(1)  // Forced to read pins and write to Flash
#else
	if (!Module_ID_Initialized ())
#endif

	{
	Module_Type type = Module_ID_Read_Pins ();
	Module_ID_Write_Flash (type);
	Module_ID_Verify_Flash (type);
	}

	Module_Type type = Module_ID_Read_Flash ();
	Module_ID_Set_Global_Variables (type);

}

// 1734 Application Global Variables
unsigned char name_mem[] = "\x1A"
						   "Error: ID not programmed!.";
unsigned int  product_type_mem	 = 1;
unsigned int  product_code_mem	 = 1;
unsigned char major_revision_mem = 1;
unsigned char minor_revision_mem = 1;
unsigned int  vendor_id_mem		 = 1;


const unsigned char id_ILX_232_name_mem[] = "\x0C""ILX34-MBS232";
const unsigned int	id_ILX_232_product_type_mem   = 12;
const unsigned int	id_ILX_232_product_code_mem   = 4001;
const unsigned char id_ILX_232_major_revision_mem = 1;
const unsigned char id_ILX_232_minor_revision_mem = 16;
const unsigned int	id_ILX_232_vendor_id_mem	   = 309;

const unsigned char id_ILX_485_name_mem[] = "\x0C""ILX34-MBS485";
const unsigned int	id_ILX_485_product_type_mem   = 12;
const unsigned int	id_ILX_485_product_code_mem   = 4002;
const unsigned char id_ILX_485_major_revision_mem = 1;
const unsigned char id_ILX_485_minor_revision_mem = 16;
const unsigned int	id_ILX_485_vendor_id_mem	   = 309;
/*
	Sets 1734 applications module ID variables
*/
static void Module_ID_Set_Global_Variables (Module_Type type)
{
	assert_permitted_runtime_module_type (type, "attempted set identity object to an invalid module type");

	const unsigned char *p_name_mem			  = 0;
	const unsigned int * p_product_type_mem	  = 0;
	const unsigned int * p_product_code_mem	  = 0;
	const unsigned char *p_major_revision_mem = 0;
	const unsigned char *p_minor_revision_mem = 0;
	const unsigned int * p_vendor_id_mem	  = 0;

	switch (type)
	{
	case MODULE_TYPE_ILX34_MBS232:
		p_name_mem			 = id_ILX_232_name_mem;
		p_product_type_mem	 = &id_ILX_232_product_type_mem;
		p_product_code_mem	 = &id_ILX_232_product_code_mem;
		p_major_revision_mem = &id_ILX_232_major_revision_mem;
		p_minor_revision_mem = &id_ILX_232_minor_revision_mem;
		p_vendor_id_mem		 = &id_ILX_232_vendor_id_mem;
		break;
	case MODULE_TYPE_ILX34_MBS485:
		p_name_mem			 = id_ILX_485_name_mem;
		p_product_type_mem	 = &id_ILX_485_product_type_mem;
		p_product_code_mem	 = &id_ILX_485_product_code_mem;
		p_major_revision_mem = &id_ILX_485_major_revision_mem;
		p_minor_revision_mem = &id_ILX_485_minor_revision_mem;
		p_vendor_id_mem		 = &id_ILX_485_vendor_id_mem;
		break;
	default:
		break;
	}

	assert_with_message (p_name_mem != NULL, "null module id pointer");
	assert_with_message (p_product_type_mem != NULL, "null module id pointer");
	assert_with_message (p_product_code_mem != NULL, "null module id pointer");
	assert_with_message (p_major_revision_mem != NULL, "null module id pointer");
	assert_with_message (p_minor_revision_mem != NULL, "null module id pointer");
	assert_with_message (p_vendor_id_mem != NULL, "null module id pointer");

	memcpy (name_mem, p_name_mem, sizeof (name_mem));
	memcpy (&product_type_mem, p_product_type_mem, sizeof (product_type_mem));
	memcpy (&product_code_mem, p_product_code_mem, sizeof (product_code_mem));
	memcpy (&major_revision_mem, p_major_revision_mem, sizeof (major_revision_mem));
	memcpy (&minor_revision_mem, p_minor_revision_mem, sizeof (minor_revision_mem));
	memcpy (&vendor_id_mem, p_vendor_id_mem, sizeof (vendor_id_mem));

	DSC_Writes (DSC_LEVEL_INFO, "MODULE ID (decimal)\r\nproduct string: ");
	DSC_Writes (DSC_LEVEL_INFO, (char *)name_mem);
	DSC_Writes (DSC_LEVEL_INFO, "\r\nproduct type: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, product_type_mem, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\r\nproduct code: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, product_code_mem, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\r\nmajor rev: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, major_revision_mem, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\r\nminor rev: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, minor_revision_mem, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\r\nvendor id: ");
	DSC_Write_UInt (DSC_LEVEL_INFO, vendor_id_mem, 0);
	DSC_Writes (DSC_LEVEL_INFO, "\r\n");
}
