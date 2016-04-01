/*
 * This file contains common configuration parameters
 * for Netgear WNR2000v3 router family.
 */

#ifndef __WNR_COMMON_H
#define __WNR_COMMON_H

#include <configs/ar7240.h>

/* 
 ************************
 *  General parameters  *
 ************************
 */
#define CONFIG_FS		1
#define CFG_FS_SQUASHFS		1
#define CONFIG_SQUASHFS_LZMA
#ifndef CONFIG_LZMA
#	define CONFIG_LZMA	1
#endif

#define ENABLE_DYNAMIC_CONF	1
#define CONFIG_SUPPORT_AR7241	1

#define CONFIG_BOOTDELAY	2	/* autoboot after 2 seconds */

#define SKIP_ATHEROS_ETHADDR_CHECKING 1	/* Skip Atheros Ethernet Address Checking*/

#define CFG_NMRP				1
#define CFG_SINGLE_FIRMWARE			1
#define TFTP_RECOVER_MODE_PINGABLE		1
#define FIRMWARE_INTEGRITY_CHECK		1
#define FIRMWARE_RECOVER_FROM_TFTP_SERVER	1

#define CONFIG_IPADDR				192.168.1.1
#define CONFIG_SERVERIP				192.168.1.5
#define CONFIG_ETHADDR				0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN		1

#define DEBUG

#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2		"hush>"

#define CFG_FLASH_WORD_SIZE     	unsigned short
#define CFG_INIT_SP_OFFSET		0x1000	/* Cache lock for stack */
#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI

/*
 ***********
 *  FLASH  *
 ***********
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */

#define CFG_FLASH_SECTOR_SIZE   (64*1024)

#define CFG_FLASH_BASE		0x9f000000

#if (FLASH_SIZE == 16)
#	define CFG_MAX_FLASH_SECT	256		/* max number of sectors on one 16MB chip */
#	define CFG_FLASH_SIZE		0x01000000	/* Total 16MB flash size */
#	define BOARDCAL			(CFG_FLASH_BASE + 0x00ff0000)	/* ART: System config */
#	define WLANCAL			(CFG_FLASH_BASE + 0x00ff1000)	/* ART: Radio calibration */
#	define MTDPARTS_DEFAULT		"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),16000k(firmware),64k(art)"
#	define CONFIG_BOOTARGS		"console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),16000k(firmware),64k(art) REVISIONID"
#elif (FLASH_SIZE == 8)
#	define CFG_MAX_FLASH_SECT	128		/* max number of sectors on one 8MB chip */
#	define CFG_FLASH_SIZE		0x00800000	/* Total 8MB flash size */
#	define BOARDCAL			(CFG_FLASH_BASE + 0x007f0000)	/* ART: System config */
#	define WLANCAL			(CFG_FLASH_BASE + 0x007f1000)	/* ART: Radio calibration */
#	define MTDPARTS_DEFAULT		"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),7808k(firmware),64k(art)"
#	define CONFIG_BOOTARGS		"console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),7808k(firmware),64k(art) REVISIONID"
#elif (FLASH_SIZE == 4)
#	define CFG_MAX_FLASH_SECT	64		/* max number of sectors on one 4MB chip */
#	define CFG_FLASH_SIZE		0x00400000	/* Total 4MB flash size */
#	define BOARDCAL			(CFG_FLASH_BASE + 0x003f0000)	/* ART: System config */
#	define WLANCAL			(CFG_FLASH_BASE + 0x003f1000)	/* ART: Radio calibration */
#	define MTDPARTS_DEFAULT		"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),3712k(firmware),64k(art)"
#	define CONFIG_BOOTARGS		"console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),3712k(firmware),64k(art) REVISIONID"
#else
#	error "Invalid flash configuration"
#endif

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define	CAL_SECTOR		(CFG_MAX_FLASH_SECT - 1)

/*
 ************
 *  U-boot  *
 ************
 */
#define UBOOT_FLASH_SIZE		(256 * 1024)

#define CFG_ENV_ADDR			(CFG_FLASH_BASE + UBOOT_FLASH_SIZE)
#define CFG_ENV_SIZE			(CFG_FLASH_SECTOR_SIZE)
#define	CFG_ENV_IS_IN_FLASH		1

#undef CFG_ENV_IS_NOWHERE

#ifdef ENABLE_DYNAMIC_CONF
#	define UBOOT_ENV_SEC_START	(CFG_ENV_ADDR)
#	define CFG_FLASH_MAGIC		0xaabacada  
#	define CFG_FLASH_MAGIC_F	(CFG_ENV_ADDR + CFG_FLASH_SECTOR_SIZE - 0x20)
#	define CFG_FLASH_SECTOR_SIZE_F	*(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#	define CFG_FLASH_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x8)
#	define CFG_MAX_FLASH_SECT_F    	(CFG_MAX_FLASH_SECT)
#endif

#define	CFG_MONITOR_BASE		(TEXT_BASE)
#define	CFG_MONITOR_LEN			(192 << 10)

#ifdef FIRMWARE_RECOVER_FROM_TFTP_SERVER
#	define CFG_IMAGE_LEN			0x350000
#	define CFG_IMAGE_BASE_ADDR		(CFG_ENV_ADDR + CFG_ENV_SIZE)
#	define CFG_IMAGE_ADDR_BEGIN		(CFG_IMAGE_BASE_ADDR)
#	define CFG_IMAGE_ADDR_END		(CFG_IMAGE_BASE_ADDR + CFG_IMAGE_LEN)
#	define CFG_FLASH_CONFIG_BASE		(CFG_IMAGE_ADDR_END)
#	define CFG_FLASH_CONFIG_PARTITION_SIZE	(CFG_FLASH_SECTOR_SIZE)
#	define CFG_STRING_TABLE_LEN		0x19000	/* Each string table takes 100K to save */
#	define CFG_STRING_TABLE_NUMBER		1
#	define CFG_STRING_TABLE_TOTAL_LEN	0x20000 /* Totally allocate 128K to save only one string table */
#	define CFG_STRING_TABLE_BASE_ADDR	(CFG_FLASH_BASE + 0x003b0000)
#	define CFG_STRING_TABLE_ADDR_BEGIN	(CFG_STRING_TABLE_BASE_ADDR)
#	define CFG_STRING_TABLE_ADDR_END	(CFG_STRING_TABLE_ADDR_BEGIN + CFG_STRING_TABLE_TOTAL_LEN)
#endif

/*
 *****************
 *  Clock - PLL  *
 *****************
 */
#undef CFG_HZ
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#	define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#	define CFG_HZ          (300000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_340_340_170)
#	define CFG_HZ          (340000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_350_350_175)
#	define CFG_HZ          (350000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_360_360_180)
#       define CFG_HZ          (360000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#	define CFG_HZ          (333000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200) || (CFG_PLL_FREQ == CFG_PLL_400_400_100)
#	define CFG_HZ          (400000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_320_320_80) || (CFG_PLL_FREQ == CFG_PLL_320_320_160)
#	define CFG_HZ          (320000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_410_400_200)
#	define CFG_HZ          (410000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_420_400_200)
#	define CFG_HZ          (420000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_240_240_120)
#	define CFG_HZ          (240000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_160_160_80)
#	define CFG_HZ          (160000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_200_200)
#	define CFG_HZ          (400000000/2)
#endif

#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase in ticks */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write in ticks */

/*
 *********************
 *  DDR init values  *
 *********************
 */
#define CONFIG_NR_DRAM_BANKS	2

/* DDR values to support AR7241 */
#ifdef CONFIG_SUPPORT_AR7241 
#	define CFG_7241_DDR1_CONFIG_VAL		0xc7bc8cd0
//#	define CFG_7241_DDR1_CONFIG_VAL		0x6fbc8cd0
#	define CFG_7241_DDR1_MODE_VAL_INIT	0x133
#	define CFG_7241_DDR1_EXT_MODE_VAL	0x0
#	define CFG_7241_DDR1_MODE_VAL		0x33
//#	define CFG_7241_DDR1_MODE_VAL		0x23
#	define CFG_7241_DDR1_CONFIG2_VAL	0x9dd0e6a8
#	define CFG_7241_DDR2_CONFIG_VAL		0xc7bc8cd0
#	define CFG_7241_DDR2_MODE_VAL_INIT	0x133
#	define CFG_7241_DDR2_EXT_MODE_VAL	0x402
#	define CFG_7241_DDR2_MODE_VAL		0x33
#	define CFG_7241_DDR2_CONFIG2_VAL	0x9dd0e6a8
#endif /* _SUPPORT_AR7241 */

/* DDR settings for AR7240 */
#define CFG_DDR_REFRESH_VAL			0x4f10
#define CFG_DDR_CONFIG_VAL			0xc7bc8cd0
#define CFG_DDR_MODE_VAL_INIT			0x133

#ifdef LOW_DRIVE_STRENGTH
#       define CFG_DDR_EXT_MODE_VAL		0x2
#else
#       define CFG_DDR_EXT_MODE_VAL		0x0
#endif

#define CFG_DDR_MODE_VAL			0x33

#define CFG_DDR_TRTW_VAL			0x1f
#define CFG_DDR_TWTR_VAL			0x1e

#define CFG_DDR_CONFIG2_VAL			0x9dd0e6a8
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL		0x00ff

/* DDR2 Init values */
#define CFG_DDR2_EXT_MODE_VAL			0x402

/* DDR value from Flash */
#ifdef ENABLE_DYNAMIC_CONF
#	define CFG_DDR_MAGIC			0xaabacada  
#	define CFG_DDR_MAGIC_F			(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#	define CFG_DDR_CONFIG_VAL_F		*(volatile int *)(CFG_DDR_MAGIC_F + 4)
#	define CFG_DDR_CONFIG2_VAL_F		*(volatile int *)(CFG_DDR_MAGIC_F + 8)
#	define CFG_DDR_EXT_MODE_VAL_F		*(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

/*
 ****************
 *  ART config  *
 ****************
 */
#define WAN_MAC_OFFSET			0x0000
#define LAN_MAC_OFFSET			0x0006
#define WLAN_MAC_OFFSET			0x108c

/*
 ********************
 *  Network switch  *
 ********************
 */
#define CFG_PHY_ADDR			0
#define CFG_AG7240_NMACS		2
#define CFG_GMII			0
#define CFG_MII0_RMII			1
#define CFG_AG7100_GE0_RMII		1
#define CFG_RX_ETH_BUFFER		16

/*
 ***********
 *  JFFS2  *
 ***********
 */
#undef CFG_JFFS_CUSTOM_PART
#define MTDIDS_DEFAULT			"nor0=ar7240-nor0"
#define CONFIG_JFFS2_PART_OFFSET	0x50040

/*
 **************
 *  Commands  *
 **************
 */
#undef CONFIG_JFFS2_CMDLINE

#define	CFG_LONGHELP			/* undef to save memory */

#define CONFIG_BOOTCOMMAND "fsload 80800000 image/uImage;bootm 80800000"

#define CONFIG_EXTRA_ENV_SETTINGS \
"clearenv=erase 0x9f040000 +0x10000\0" \
"ddr_ext_mode_value=0x0\0" \
"magic_number=0x0"

#endif	/* __WNR_COMMON_H */
