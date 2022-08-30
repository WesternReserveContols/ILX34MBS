#include <stdint.h>
#include <stdbool.h>

#include "bsp_main.h"
#include "module_id.h"
/*
bool mock_pin_id_1734_232 = false;
bool mock_pin_id_1734_485 = false;
bool mock_pin_id_1738_232 = false;
bool mock_pin_id_1738_485 = false;
bool mock_pin_id_34_232 = false;
bool mock_pin_id_34_485 = false;*/

bool pin_id_1734_232 ()
{
	// return mock_pin_id_1734_232;
	return (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_7)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_0)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1));
}

bool pin_id_1734_485 ()
{
	// return mock_pin_id_1734_485;
	return (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_7)) && (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_0)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1));
}

bool pin_id_1738_232 ()
{
	// return mock_pin_id_1738_232;
	return (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_0)) && (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_7)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1));
}

bool pin_id_1738_485 ()
{
	// return mock_pin_id_1738_485;
	return (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_1)) && (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_7)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_0));
}

bool pin_id_34_232 ()
{
	// return mock_pin_id_34_232;
	return (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_7)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_0)) && (!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1));
}

bool pin_id_34_485 ()
{
	// return mock_pin_id_34_485;
	return (!HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_6)) && (HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_7)) && (!HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_0)) && (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1));
}
