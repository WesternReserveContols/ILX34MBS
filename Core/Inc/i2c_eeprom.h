/*
 * i2c_eeprom.h
 *
 *  Created on: Feb 3, 2020
 *      Author: wrc
 */

#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

#include <stdint.h>

typedef enum
{
	EE_SUCCESS = 0U,
	EE_ERR_Write_Fail,
	EE_ERR_Offset_Length
} ee_err_status;

void		  MX_I2C1_Init (void);
ee_err_status EE_HAL_WriteOne (uint16_t offset, uint8_t data);
ee_err_status EE_HAL_ReadOne (uint16_t offset, uint8_t *read_data);
ee_err_status EE_HAL_Read (uint16_t offset, uint8_t *read_data, uint16_t num_elements);
void		  EE_wait_until_ready (unsigned char block);
int			  Eeprom_test (void);
void 		  EE_Error (char *userStr);

#endif
