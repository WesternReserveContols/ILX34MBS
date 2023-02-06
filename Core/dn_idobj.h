//***********************************************************************
// Project Name              DeviceNet
// Module                    DN_IDOBJ.H
//
// Module Description       This Module contains the includes for the
//                     DeviceNet Identity Object.
//
//             Copyright (c) 1997 Allen-Bradley Co.
//***********************************************************************

#include <dn_def.h>
#include <std.h>

/*
// Identity Object
*/
typedef struct
{
	unsigned char  vendor[2];		 // usually 01 = AB
	unsigned char  type[2];			 // usually 01 = AB
	unsigned char  pc[2];			 // usually 01 = AB
	unsigned char  rev[2];			 // usually 01 = AB
	unsigned char  serial_number[4]; // Serial Number
	unsigned char *name;
} IdentityObjectStructure;

extern IdentityObjectStructure IdentityObjectRAM;

extern void IdentityObjectGetInstanceAttr ();
extern void IdentityObjectSetInstanceAttr ();
extern void IdentityObjectInit ();
extern void IdentityObject ();

extern unsigned char cpValueLow;

/*
// Identity OBJECT ATTRIBUTE IDS
*/
#define DEVOBJ_ALL_ATTRS	0x00 /*  NOT SUPPORTED */
#define DEVOBJ_REVISION		0x01
#define DEVOBJ_MAX_INSTANCE 0x02

#define DEVOBJ_VENDOR_ID	0x01
#define DEVOBJ_PROD_TYPE	0x02
#define DEVOBJ_PROD_CODE	0x03
#define DEVOBJ_REV			0x04
#define DEVOBJ_STATUS		0x05
#define DEVOBJ_SERIAL_NUM	0x06
#define DEVOBJ_PRODUCT_NAME 0x07
#define DEVOBJ_CCV			0x09

/*
// Indentity Object Status defines
*/
#define DEVICE_OWNED_B1		  0x01
#define DEVICE_CONFIGURED_B1  0x04
#define DEVICE_MINOR_FAULT_B2 0x02
#define DEVICE_MAJOR_FAULT_B2 0x08
#define RESTART				  0x00
#define OUTOFBOX_RESET		  0x01
#define COLD_BOOT			  0x02      // Rick_TEST
