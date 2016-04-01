/*
 * This file contains configuration parameters
 * specific for Netgear WNR2200 router.
 */

#ifndef __WNR2200_H
#define __WNR2200_H

#define ATHEROS_PRODUCT_ID              137

#define IH_MAGIC_DEFAULT		0x32323030	

#undef CONFIG_MISC_INIT_R
#undef NETGEAR_BOARD_ID_SUPPORT

/*
 ***********
 *  FLASH  *
 ***********
 */
#ifndef FLASH_SIZE
#	define FLASH_SIZE	8	/* Original router has 8MB flash */
#endif

/*
 *****************
 *  Clock - PLL  *
 *****************
 */
#define CFG_AR7241_PLL_FREQ	CFG_PLL_360_360_180
#define CFG_PLL_FREQ		CFG_PLL_360_360_180

/* this file must be included with FLASH_SIZE macro set */
/* and PLL clock defined				*/
#include <configs/wnr_common.h>

/*
 **********
 *  GPIO  *
 **********
 */
/* AR9287 */
#define TEST_LED		1
#define POWER_LED		2
#define RESET_BUTTON_GPIO	6

/*
 ****************
 *  ART config  *
 ****************
 */

/*
 * wnr2200 hardware's PCB number is 2976360001, 8M Flash, 64M SDRam
 * It's HW_ID would be "29763600+08+64".
 * "(8 MSB of PCB number)+(Flash size)+(SDRam size)", length should be 14
 */
#if (FLASH_SIZE == 16)
#	define BOARD_HW_ID_DEFAULT		"29763600+16+64"
#elif (FLASH_SIZE == 8)
#	define BOARD_HW_ID_DEFAULT		"29763600+08+64"
#else /* FLASH_SIZE == 4 */
#	define BOARD_HW_ID_DEFAULT		"29763600+04+64"
#endif

#define BOARD_MODEL_ID_DEFAULT		"WNR2200"

/* For Kite, only PCI-e interface is valid */
#define AR7240_ART_PCICFG_OFFSET        12

/* 6(LAN mac) + 6(WAN mac) = 12(0x0c) */
#define WPSPIN_OFFSET			0x000c
#define WPSPIN_LENGTH			8
/* 12(WPSPIN_OFFSET) + 8(WPSPIN_LENGTH) = 20(0x14) */
#define SERIAL_NUMBER_OFFSET		(WPSPIN_OFFSET + WPSPIN_LENGTH)
#define SERIAL_NUMBER_LENGTH		13
/* 20(SERIAL_NUMBER_OFFSET) + 13(SERIAL_NUMBER_LENGTH) = 33(0x21) */
#define REGION_NUMBER_OFFSET		(SERIAL_NUMBER_OFFSET + SERIAL_NUMBER_LENGTH)
#define REGION_NUMBER_LENGTH		2
#define BOARD_HW_ID_OFFSET		(REGION_NUMBER_OFFSET + REGION_NUMBER_LENGTH)
#define BOARD_HW_ID_LENGTH		14
#define BOARD_MODEL_ID_OFFSET		(BOARD_HW_ID_OFFSET + BOARD_HW_ID_LENGTH)
#define BOARD_MODEL_ID_LENGTH		10
#define BOARD_SSID_OFFSET		(BOARD_MODEL_ID_OFFSET + BOARD_MODEL_ID_LENGTH)
#define BOARD_SSID_LENGTH		32
#define BOARD_PASSPHRASE_OFFSET		(BOARD_SSID_OFFSET + BOARD_SSID_LENGTH)
#define BOARD_PASSPHRASE_LENGTH		64

/*
 **************
 *  Commands  *
 **************
 */
#define CFG_PROMPT	"wnr2200> "      /* Monitor Command Prompt    */

#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | CFG_CMD_PCI |CFG_CMD_PLL|\
	CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_JFFS2 | CFG_CMD_ENV | CFG_CMD_DDR| CFG_CMD_FLS|\
	CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | CFG_CMD_ETHREG ))

#include <cmd_confdefs.h>

#endif /* __WNR2200_H */

