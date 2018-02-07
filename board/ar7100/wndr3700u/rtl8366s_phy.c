/*
 * Copyright c                  Realtek Semiconductor Corporation, 2006 
 * All rights reserved.
 * 
 * Program : Control  smi connected RTL8366
 * Abstract : 
 * Author : Yu-Mei Pan (ympan@realtek.com.cn)                
 *  $Id: smi.c,v 1.5 2007/05/23 11:30:59 abelshie Exp $
 *
*/

#ifndef _RTL_TYPES_H
typedef unsigned long long	uint64;
typedef long long			int64;
typedef unsigned int		uint32;
typedef int					int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uint8;
typedef char				int8;

typedef uint32          	ipaddr_t;

#define ETHER_ADDR_LEN		6
typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;


#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))  


#ifdef _LITTLE_ENDIAN
	#define ntohs(x)   (swaps16(x))
	#define ntohl(x)   (swapl32(x))
	#define htons(x)   (swaps16(x))
	#define htonl(x)   (swapl32(x))
#else
	#define ntohs(x)	(x)
	#define ntohl(x)	(x)
	#define htons(x)	(x)
	#define htonl(x)	(x)
#endif


#ifdef _LITTLE_ENDIAN
	#define MEM16(x)		(x)
#else
	#define MEM16(x)		(swaps16(x)) 
#endif


#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#ifdef __KERNEL__
#define rtlglue_printf printk
#else// __KERNEL__
#define rtlglue_printf printf
#endif// __KERNEL__
#define PRINT			rtlglue_printf

int32 smi_reset(uint32 port, uint32 pinRST);
int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
int32 smi_read(uint32 mAddrs, uint32 *rData);
int32 smi_write(uint32 mAddrs, uint32 rData);

/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  smi connected RTL8366
* Abstract : 
* Author : Yu-Mei Pan (ympan@realtek.com.cn)                
*  $Id: smi.c,v 1.5 2007/05/23 11:30:59 abelshie Exp $
*/

#include <common.h>
#include <ar7100_soc.h>


void GpioSet(unsigned long regoffs,unsigned long mask,unsigned long value)
{
        unsigned long gppData;
        gppData = *((volatile unsigned long*)regoffs);
        if(value)
                gppData |= mask;
        else
                gppData &= ~mask;
        *((volatile unsigned long *)regoffs) = gppData;
}
unsigned long GpioGet(unsigned long regoffs,unsigned long mask)
{
        unsigned long gppData;
        gppData = *((volatile unsigned long*)regoffs);
        gppData &= mask;
	 if (gppData)
		return 1;
	 else 
		return 0;
}


 
#define DELAY						10000
#define CLK_DURATION(clk)			{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)		{ /*if (!(ok)) return FAILED; */}

typedef int gpioID;

#define smi_SCK	(1 << 7)		/* GPIO used for SMI Clock Generation */
#define smi_SDA	(1 << 5)		/* GPIO used for SMI Data signal */
#define smi_RST	(1 << 8)     /* GPIO used for reset swtich */


#define ack_timer					5
#define max_register				0x018A 


void _smi_start(void)
{

	#if 1
	/* change GPIO pin to Output only */
	//_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	//_rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
	GpioSet(AR7100_GPIO_OE, smi_SDA, 1);
	
	/* Initial state: SCK: 0, SDA: 1 */
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	GpioSet(AR7100_GPIO_OUT, smi_SDA, 1);
	CLK_DURATION(DELAY);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	CLK_DURATION(DELAY);

	/* CLK 2: */
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SDA, 0);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SDA, 1);
	#endif
}



void _smi_writeBit(uint16 signal, uint32 bitLen)
{
	#if 1
	GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
	GpioSet(AR7100_GPIO_OE, smi_SDA, 1);
	for( ; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			GpioSet(AR7100_GPIO_OUT, smi_SDA, 1);	
		else 
			GpioSet(AR7100_GPIO_OUT, smi_SDA, 0);	
		CLK_DURATION(DELAY);

		/* clocking */
		GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);
		CLK_DURATION(DELAY);
		GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	}
	#endif
}



void _smi_readBit(uint32 bitLen, uint32 *rData) 
{
	uint32 u;

	/* change GPIO pin to Input only */
	//_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

	for (*rData = 0; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* clocking */
		GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);
		GpioSet(AR7100_GPIO_OE, smi_SDA, 0);
		CLK_DURATION(DELAY);
		u = GpioGet(AR7100_GPIO_IN, smi_SDA);
		CLK_DURATION(DELAY);
		GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
		
		GpioSet(AR7100_GPIO_OUT,smi_SCK, 0);

		*rData |= (u << (bitLen - 1));
	}

	/* change GPIO pin to Output only */
	//_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
}




void _smi_stop(void)
{

	GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
	GpioSet(AR7100_GPIO_OE, smi_SDA, 1);
	
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SDA, 0);	
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);	
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SDA, 1);	
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);

    /* add a click */
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 0);
	CLK_DURATION(DELAY);
	GpioSet(AR7100_GPIO_OUT, smi_SCK, 1);


	/* change GPIO pin to Output only */
	//_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	//_rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

}


int32 smi_reset(uint32 port, uint32 pinRST)
{
	GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
	GpioSet(AR7100_GPIO_OE, smi_SDA, 1);
	
	/* Initialize GPIO port A, pin 7 as SMI RESET */
	#if 0
	gpioID gpioId;
	int32 res;

	gpioId = GPIO_ID(port, pinRST);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_RST = gpioId;
	#endif

	GpioSet(AR7100_GPIO_OUT, smi_RST, 1);
	CLK_DURATION(1000000);
	GpioSet(AR7100_GPIO_OUT, smi_RST, 0);	
	CLK_DURATION(1000000);
	GpioSet(AR7100_GPIO_OUT, smi_RST, 1);
	CLK_DURATION(1000000);

	/* change GPIO pin to Input only */
	//_rtl865x_initGpioPin(smi_RST, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

	return SUCCESS;
}


int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA)
{
	#if 0
	gpioID gpioId;
	int32 res;

	/* change GPIO pin to Input only */
	/* Initialize GPIO port C, pin 0 as SMI SDA0 */
	gpioId = GPIO_ID(port, pinSDA);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_SDA = gpioId;


	/* Initialize GPIO port C, pin 1 as SMI SCK0 */
	gpioId = GPIO_ID(port, pinSCK);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_SCK = gpioId;


	_rtl865x_setGpioDataBit(smi_SDA, 1);	
	_rtl865x_setGpioDataBit(smi_SCK, 1);	
	return SUCCESS;
	#endif
	GpioSet(AR7100_GPIO_OE, smi_SCK, 1);
	GpioSet(AR7100_GPIO_OE, smi_SDA, 1);
	GpioSet(AR7100_GPIO_OE, smi_RST, 1);
	//GpioSet(AR7100_GPIO_OE,(smi_SCK | smi_SDA | smi_RST))
	return SUCCESS;
}





int32 smi_read(uint32 mAddrs, uint32 *rData)
{
	uint32 rawData=0, ACK;
	uint8  con;
	uint32 ret = SUCCESS;
/*
	if ((mAddrs > max_register) || (rData == NULL))  return	FAILED;
*/

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
	#if 0 //stoneliu
	rtlglue_drvMutexLock();
	#endif
	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x1, 1);						/* 1: issue READ command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing READ command*/
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */	
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK by RTL8369 */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_readBit(8, &rawData);					/* Read DATA [7:0] */
	*rData = rawData&0xff; 

	_smi_writeBit(0x00, 1);						/* ACK by CPU */

	_smi_readBit(8, &rawData);					/* Read DATA [15: 8] */

	_smi_writeBit(0x01, 1);						/* ACK by CPU */
	*rData |= (rawData<<8);

	_smi_stop();

	#if 0
	rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	#endif

	return ret;
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{
/*
	if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return	FAILED;
*/
	int8 con;
	uint32 ACK;
	uint32 ret = SUCCESS;	

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
	#if 0
	rtlglue_drvMutexLock();
	#endif

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x0, 1);						/* 0: issue WRITE command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing WRITE command*/
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit(rData&0xff, 8);				/* Write Data [7:0] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for writting data [7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit(rData>>8, 8);					/* Write Data [15:8] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);						/* ACK for writting data [15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_stop();	

	#if 0 
	rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	#endif

	return ret;
}

#define RTL8366S_LED_INDICATED_CONF_REG			0x421
#define ERRNO_INVALIDINPUT					-1000/* invalid input parameter */
#define ERRNO_SMIERROR						-1001/* SMI access error */
#define RTL8366S_LED_0_1_FORCE_REG				0x422
#define RTL8366S_LED_2_3_FORCE_REG				0x423
#define RTL8366S_PHY_ACCESS_CTRL_REG			0x8028
#define RTL8366S_PHY_CTRL_WRITE				0

enum RTL8366S_LEDCONF{

	LEDCONF_LEDOFF=0, 		
	LEDCONF_DUPCOL,		
	LEDCONF_LINK_ACT,		
	LEDCONF_SPD1000,		
	LEDCONF_SPD100,		
	LEDCONF_SPD10,			
	LEDCONF_SPD1000ACT,	
	LEDCONF_SPD100ACT,	
	LEDCONF_SPD10ACT,		
	LEDCONF_SPD10010ACT,  
	LEDCONF_FIBER,			
	LEDCONF_FAULT,			
	LEDCONF_LINKRX,		
	LEDCONF_LINKTX,		
	LEDCONF_MASTER,		
	LEDCONF_LEDFORCE,		
};

#define RTL8366S_LED_GROUP_MAX				4

/*
@func int32 | rtl8366s_setAsicReg | Set content of asic register.
@parm uint32 | reg | Register's address.
@parm uint32 | value | Value setting to register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The value will be set to ASIC mapping address only and it is always return SUCCESS while setting un-mapping address registers.
	
*/
int32 rtl8366s_setAsicReg(uint32 reg, uint32 value)
{

#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	int32 retVal;

	retVal = RTL_SINGLE_WRITE(reg, value);
	if (retVal != TRUE) return ERRNO_SMIERROR;

	if (cleDebuggingDisplay==0x8366)
		PRINT("\nset reg=0x%4.4x data=0x%4.4x\n",reg,value);

#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)
	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8366S_MIB_START_ADDRESS && reg <= RTL8366S_MIB_END_ADDRESS)
	{
		reg = (reg & 0x0003) + RTL8366S_MIB_READ_VIRTUAL_ADDR;
	}
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	Rtl8366sVirtualReg[reg] = value;
#else
	int32 retVal;

	retVal = smi_write(reg, value);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicForceLeds | Turn on/off Led of dedicated port
@parm uint32 | ledG0Msk | Turn on or turn off Leds of group 0, 1:on 0:off.
@parm uint32 | ledG1Msk | Turn on or turn off Leds of group 1, 1:on 0:off.
@parm uint32 | ledG2Msk | Turn on or turn off Leds of group 2, 1:on 0:off.
@parm uint32 | ledG3Msk | Turn on or turn off Leds of group 3, 1:on 0:off.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can turn on/off desired Led group of dedicated port while indicated information configuration of LED group is set to force mode.
 */
int32 rtl8366s_setAsicForceLeds(uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk, uint32 ledG3Msk)
{
	uint32 retVal, regValue;

	regValue = (ledG0Msk & 0x3F) | (ledG1Msk&0x3F) << 6;

	retVal = rtl8366s_setAsicReg(RTL8366S_LED_0_1_FORCE_REG, regValue); 	
	if(retVal != SUCCESS)
		return retVal;

	regValue = (ledG2Msk & 0x3F) | (ledG3Msk&0x3F) << 6;
	retVal = rtl8366s_setAsicReg(RTL8366S_LED_2_3_FORCE_REG, regValue); 	
	
	return retVal;
}

/*
@func int32 | rtl8366s_getAsicReg | Get content of register.
@parm uint32 | reg | Register's address.
@parm uint32* | value | Value of register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
 	Value 0x0000 will be returned for ASIC un-mapping address.
	
*/
int32 rtl8366s_getAsicReg(uint32 reg, uint32 *value)
{
	
#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;

	*value = regData;
	

	if (cleDebuggingDisplay)
		PRINT("\nget reg=0x%4.4x data=0x%4.4x\n",reg,regData);

#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)
       uint16 virtualMIBs;

	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8366S_MIB_START_ADDRESS && reg <= RTL8366S_MIB_END_ADDRESS)
	{
		virtualMIBs = reg;
		
		reg = (reg & 0x0003) + RTL8366S_MIB_READ_VIRTUAL_ADDR;

		Rtl8366sVirtualReg[reg] = virtualMIBs;
	}
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	*value = Rtl8366sVirtualReg[reg];
#else
	uint32 regData;
	int32 retVal;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;

	*value = regData;

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicLedIndicateInfoConfig | Set Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8366S_LEDCONF | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
	Definition		LED Statuses			Description
	0000		LED_Off				LED pin Tri-State.
	0001		Dup/Col				Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
	0010		Link/Act				Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
	0011		Spd1000				1000Mb/s Speed Indicator. Low for 1000Mb/s.
	0100		Spd100				100Mb/s Speed Indicator. Low for 100Mb/s.
	0101		Spd10				10Mb/s Speed Indicator. Low for 10Mb/s.
	0110		Spd1000/Act			1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	0111		Spd100/Act			100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1000		Spd10/Act			10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1001		Spd100 (10)/Act		10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1010		Fiber				Fiber link Indicator. Low for Fiber.
	1011		Fault	Auto-negotiation 	Fault Indicator. Low for Fault.
	1100		Link/Rx				Link, Activity Indicator. Low for link established. Link/Rx Blinks every 43ms when the corresponding port is transmitting.
	1101		Link/Tx				Link, Activity Indicator. Low for link established. Link/Tx Blinks every 43ms when the corresponding port is receiving.
	1110		Master				Link on Master Indicator. Low for link Master established.
	1111		LED_Force			Force LED output, LED output value reference 
 */
int32 rtl8366s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF config)
{
	uint32 retVal, regValue;

	if(ledNo >=RTL8366S_LED_GROUP_MAX)
		return ERRNO_INVALIDINPUT;

	if(config > LEDCONF_LEDFORCE)	
		return ERRNO_INVALIDINPUT;


	/* Get register value */
	retVal = rtl8366s_getAsicReg(RTL8366S_LED_INDICATED_CONF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	regValue =  (regValue & (~(0xF<<(ledNo*4)))) | (config<<(ledNo*4));

	
	retVal = rtl8366s_setAsicReg(RTL8366S_LED_INDICATED_CONF_REG, regValue); 	

	return retVal;
}

/*
@func int32 | rtl8366s_getAsicLedIndicateInfoConfig | Get Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8366S_LEDCONF* | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can get LED indicated information configuration for each LED group.
 */
int32 rtl8366s_getAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF* config)
{
	uint32 retVal, regValue;

	if(ledNo >=RTL8366S_LED_GROUP_MAX)
		return ERRNO_INVALIDINPUT;

	/* Get register value */
	retVal = rtl8366s_getAsicReg(RTL8366S_LED_INDICATED_CONF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;
	
	*config = (regValue >> (ledNo*4)) & 0x000F;
		
	return SUCCESS;
}

/*
@func int32 | rtl8366s_initChip | Set chip to default configuration enviroment
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set chip registers to default configuration for different release chip model.
*/
int32 rtl8366s_initChip(void)
{
	uint32 index;
	uint32 regData;
	uint32 ledGroup;
	enum RTL8366S_LEDCONF ledCfg[RTL8366S_LED_GROUP_MAX];
	
	const uint32 chipB[][2] = {{0x0000,	0x0038},{0x8100,	0x1B37},{0xBE2E,	0x7B9F},{0xBE2B,	0xA4C8},
							{0xBE74,	0xAD14},{0xBE2C,	0xDC00},{0xBE69,	0xD20F},{0xBE3B,	0xB414},
							{0xBE24,	0x0000},{0xBE23,	0x00A1},{0xBE22,	0x0008},{0xBE21,	0x0120},
							{0xBE20,	0x1000},{0xBE24,	0x0800},{0xBE24,	0x0000},{0xBE24,	0xF000},
							{0xBE23,	0xDF01},{0xBE22,	0xDF20},{0xBE21,	0x101A},{0xBE20,	0xA0FF},
							{0xBE24,	0xF800},{0xBE24,	0xF000},{0x0242,	0x02BF},{0x0245,	0x02BF},
							{0x0248,	0x02BF},{0x024B,	0x02BF},{0x024E,	0x02BF},{0x0251,	0x02BF},
							{0x0230,	0x0A32},{0x0233,	0x0A32},{0x0236,	0x0A32},{0x0239,	0x0A32},
							{0x023C,	0x0A32},{0x023F,	0x0A32},{0x0254,	0x0A3F},{0x0255,	0x0064},
							{0x0256,	0x0A3F},{0x0257,	0x0064},{0x0258,	0x0A3F},{0x0259,	0x0064},
							{0x025A,	0x0A3F},{0x025B,	0x0064},{0x025C,	0x0A3F},{0x025D,	0x0064},
							{0x025E,	0x0A3F},{0x025F,	0x0064},{0x0260,	0x0178},{0x0261,	0x01F4},
							{0x0262,	0x0320},{0x0263,	0x0014},{0x021D,	0x9249},{0x021E,	0x0000},
							{0x0100,	0x0004},{0xBE4A,	0xA0B4},{0xBE40,	0x9C00},{0xBE41,	0x501D},
							{0xBE48,	0x3602},{0xBE47,	0x8051},{0xBE4C,	0x6465},{0x8000,	0x1F00},
							{0x8001,	0x000C},{0x8008,	0x0000},{0x8007,	0x0000},{0x800C,	0x00A5},
							{0x8101,	0x02BC},{0xBE53,	0x0005},{0x8E45,	0xAFE8},{0x8013,	0x0005},
							{0xBE4B,	0x6700},{0x800B,	0x7000},{0xBE09,	0x0E00},
							{0xFFFF, 0xABCD}};
	
	const uint32 chipDefault[][2] = {{0x0242, 0x02BF},{0x0245, 0x02BF},{0x0248, 0x02BF},{0x024B, 0x02BF},
								{0x024E, 0x02BF},{0x0251, 0x02BF},
								{0x0254, 0x0A3F},{0x0256, 0x0A3F},{0x0258, 0x0A3F},{0x025A, 0x0A3F},
								{0x025C, 0x0A3F},{0x025E, 0x0A3F},
								{0x0263, 0x007C},{0x0100,	0x0004},									
								{0xBE5B, 0x3500},{0x800E, 0x200F},{0xBE1D, 0x0F00},{0x8001, 0x5011},
								{0x800A, 0xA2F4},{0x800B, 0x17A3},{0xBE4B, 0x17A3},{0xBE41, 0x5011},
								{0xBE17, 0x2100},{0x8000, 0x8304},{0xBE40, 0x8304},{0xBE4A, 0xA2F4},
								{0x800C, 0xA8D5},{0x8014, 0x5500},{0x8015, 0x0004},{0xBE4C, 0xA8D5},
								{0xBE59, 0x0008},{0xBE09,	0x0E00},{0xBE36, 0x1036},{0xBE37, 0x1036},
								{0x800D, 0x00FF},{0xBE4D, 0x00FF},
								{0xFFFF, 0xABCD}};


	for(ledGroup= 0;ledGroup<RTL8366S_LED_GROUP_MAX;ledGroup++)
	{
			if(SUCCESS != rtl8366s_getAsicLedIndicateInfoConfig(ledGroup,&ledCfg[ledGroup]))
			return FAILED;

		if(SUCCESS != rtl8366s_setAsicLedIndicateInfoConfig(ledGroup,LEDCONF_LEDFORCE))
			return FAILED;
	}

	if(SUCCESS != rtl8366s_setAsicForceLeds(0x3F,0x3F,0x3F,0x3F))
		return FAILED;

	/*resivion*/
	if(SUCCESS != rtl8366s_getAsicReg(0x5C,&regData))
		return FAILED;

	index = 0;
	switch(regData)
	{
 	 case 0x0000:	
		
		while(chipB[index][0] != 0xFFFF && chipB[index][1] != 0xABCD)
		{	
			/*PHY registers setting*/	
			if(0xBE00 == (chipB[index][0] & 0xBE00))
			{
				if(SUCCESS != rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
					return FAILED;
			}

			if(SUCCESS != rtl8366s_setAsicReg(chipB[index][0],chipB[index][1]))
				return FAILED;
			
			index ++;	
		}			
		break;
 	 case 0x6027:	
		while(chipDefault[index][0] != 0xFFFF && chipDefault[index][1] != 0xABCD)
		{	
			/*PHY registers setting*/	
			if(0xBE00 == (chipDefault[index][0] & 0xBE00))
			{
				if(SUCCESS != rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
					return FAILED;

			}

			if(SUCCESS != rtl8366s_setAsicReg(chipDefault[index][0],chipDefault[index][1]))
				return FAILED;
			
			index ++;	
		}
		break;
	 default:
		return FAILED;
		break;
	}



	for(ledGroup= 0;ledGroup<RTL8366S_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8366s_setAsicLedIndicateInfoConfig(ledGroup,ledCfg[ledGroup]))
			return FAILED;
	}

	return SUCCESS;
}

void rtl8366s_phy_setup(int mac)
{
	int32 ret;
	static int init_done=0;

	printf("in rtl8366s_phy_setup mac=%d\n", mac);
	if (init_done == 0) { /* only initialize once */
		init_done=1;
		ret = rtl8366s_initChip();
	printf("after rtl8366s_initChip ret=%d\n", ret);
		if (ret) /* ret should be 0 if initilaization succeeds */
			printf("rtl8366s_initChip fail\n");
	}
}
