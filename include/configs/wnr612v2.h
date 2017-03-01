/*
 * This file contains configuration parameters
 * specific for Netgear WNR612v2 router.
 */

#ifndef __WNR612V2_H
#define __WNR612V2_H

#define ATHEROS_PRODUCT_ID              138

#define IH_MAGIC_DEFAULT		0x32303631

#define LOW_DRIVE_STRENGTH

#undef CONFIG_MISC_INIT_R
#undef NETGEAR_BOARD_ID_SUPPORT

/*
 ***********
 *  FLASH  *
 ***********
 */
#ifndef FLASH_SIZE
#	define FLASH_SIZE	4	/* Original router has 4MB flash */
#endif

/*
 *****************
 *  Clock - PLL  *
 *****************
 */
#define CFG_AR7241_PLL_FREQ	CFG_PLL_400_400_200
#define CFG_PLL_FREQ		CFG_PLL_400_400_200

/* this file must be included with FLASH_SIZE macro set */
/* and PLL clock defined				*/
#include <configs/wnr_common.h>

/*
 **********
 *  GPIO  *
 **********
 */
/* AR7240 */
#define POWER_LED		11
#define TEST_LED		11
/* AR9285 */
#define WLAN_LED		1
#define RESET_BUTTON_GPIO	7

/*
 ****************
 *  ART config  *
 ****************
 */

/* For Kite, only PCI-e interface is valid */
#define AR7240_ART_PCICFG_OFFSET        3

/* 6(LAN mac) + 6(WAN mac) = 12(0x0c) */
#define WPSPIN_OFFSET			0x000c
#define WPSPIN_LENGTH			8
/* 12(WPSPIN_OFFSET) + 8(WPSPIN_LENGTH) = 20(0x14) */
#define SERIAL_NUMBER_OFFSET		(WPSPIN_OFFSET + WPSPIN_LENGTH)
#define SERIAL_NUMBER_LENGTH		13

/*
 **************
 *  Commands  *
 **************
 */
#define CFG_PROMPT	"wnr612v2> "      /* Monitor Command Prompt    */

#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | CFG_CMD_PCI |CFG_CMD_PLL|\
	CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_JFFS2 | CFG_CMD_ENV | CFG_CMD_DDR| CFG_CMD_FLS|\
	CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | CFG_CMD_ELF | CFG_CMD_ETHREG ))

#include <cmd_confdefs.h>

#endif /* __WNR612V2_H */

