/*
 * This file contains configuration parameters
 * specific for Netgear WNDR3700v1h2/v2 router.
 */

#ifndef __WNDR3700V1H2_H
#define __WNDR3700V1H2_H

#define ATHEROS_PRODUCT_ID              136

#undef CONFIG_MISC_INIT_R
#undef NETGEAR_BOARD_ID_SUPPORT

/*
 ***********
 *  FLASH  *
 ***********
 */
#ifndef FLASH_SIZE
#	define FLASH_SIZE	16	/* Original router has 16MB flash */
#endif

/*
 *****************
 *  Clock - PLL  *
 *****************
 */
#define	CFG_PLL_FREQ	CFG_PLL_680_340_170

/* this file must be included with FLASH_SIZE macro set */
/* and PLL clock defined				*/
#include <configs/wndr_common.h>

/*
 **********
 *  GPIO  *
 **********
 */
#define TEST_LED		(1<<1)
#define POWER_LED		(1<<2)
#define RESET_BUTTON_GPIO	(1<<8)

/*
 ****************
 *  ART config  *
 ****************
 */

/*
 * wndr3700v2 hardware's PCB number is 2976360001, 16M Flash, 64M SDRam
 * It's HW_ID would be "29763600+08+64".
 * "(8 MSB of PCB number)+(Flash size)+(SDRam size)", length should be 14
 */
//#if (FLASH_SIZE == 16)
//#	define BOARD_HW_ID_DEFAULT		"29763600+16+64"
//#elif (FLASH_SIZE == 8)
//#	define BOARD_HW_ID_DEFAULT		"29763600+08+64"
//#else /* FLASH_SIZE == 4 */
//#	define BOARD_HW_ID_DEFAULT		"29763600+04+64"
//#endif

//#define BOARD_MODEL_ID_DEFAULT		"WNDR3700v2"

/* 6(LAN mac) + 6(WAN mac) + 6(wlan5g) = 18(0x12) */
#define WPSPIN_OFFSET			0x12
#define WPSPIN_LENGTH			8
/* 18(WPSPIN_OFFSET) + 8(WPSPIN_LENGTH) = 26(0x1a) */
#define SERIAL_NUMBER_OFFSET		(WPSPIN_OFFSET + WPSPIN_LENGTH)
#define SERIAL_NUMBER_LENGTH		13
/* 26(SERIAL_NUMBER_OFFSET) + 13(SERIAL_NUMBER_LENGTH) = 39(0x27) */
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
#define CFG_PROMPT	"wndr3700v2> "      /* Monitor Command Prompt    */

#define CONFIG_COMMANDS (( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | CFG_CMD_PCI |CFG_CMD_PLL |\
        CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_JFFS2 | CFG_CMD_ENV | CFG_CMD_DDR |\
	CFG_CMD_FLS | CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | CFG_CMD_BSP ))

#include <cmd_confdefs.h>

#endif /* __WNDR3700V1H2_H */
