/*****************************************************************************/
/*! file ap94.c
** /brief Boot support for AP94 board
**    
**  This provides the support code required for the AP94 board in the U-Boot
**  environment.  This board is a Hydra based system with two Merlin WLAN
**  interfaces.
**
**  Copyright (c) 2008 Atheros Communications Inc.  All rights reserved.
**
*/

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <net.h>
#include "ar7100_soc.h"
#ifdef CFG_NMRP
extern int NmrpState;
extern ulong NmrpAliveTimerStart;
extern ulong NmrpAliveTimerBase;
extern int NmrpAliveTimerTimeout;
extern void NmrpSend(void);
#endif

extern flash_info_t flash_info[];
extern int ar7100_ddr_find_size(void);
extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern unsigned long GpioGet(unsigned long regoffs, unsigned long mask);
extern void GpioSet(unsigned long regoffs, unsigned long mask, unsigned long value);

/******************************************************************************/
/*!
**  \brief ar7100_mem_config
**
**  This is a "C" level implementation of the memory configuration interface
**  for the AP94.  
**
**  \return RAM size in bytes
*/

int
ar7100_mem_config(void)
{

    /* XXX - should be set based board configuration */
    *(volatile unsigned int *)0xb8050004 = 0x50C0;
    udelay(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
    *(volatile unsigned int *)0xb805001c = 0x67;/*set PCI clock to 66MHz*/
    udelay(10);
    *(volatile unsigned int *)0xb8050010 = 0x1099;
    udelay(10);

    return (ar7100_ddr_find_size());
}

/******************************************************************************/
/*!
**  \brief ar7100_usb_initial_config
**
**  -- Enter Detailed Description --
**
**  \param param1 Describe Parameter 1
**  \param param2 Describe Parameter 2
**  \return Describe return value, or N/A for void
*/

long int initdram(int board_type)
{
    printf("b8050000: 0x%x\n",* (unsigned long *)(0xb8050000));
    return (ar7100_mem_config());
}

/******************************************************************************/
/*!
**  \brief ar7100_usb_initial_config
**
**  -- Enter Detailed Description --
**
**  \param param1 Describe Parameter 1
**  \param param2 Describe Parameter 2
**  \return Describe return value, or N/A for void
*/

static void drawline(void)
{
    printf("------------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf("WNDR3700U (ar7100) U-boot " ATH_AP83_UBOOT_VERSION " dni6 V1.7\n"); */
    drawline();
    printf(" WNDR3700v1 (ar7100), %d MHz, 64 MB RAM, %d MB flash\n", CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long 
flash_get_geom (flash_info_t *flash_info)
{
    int i;
    
    /* XXX this is hardcoded until we figure out how to read flash id */

    flash_info->flash_id  = FLASH_M25P64;
    flash_info->size = CFG_FLASH_SIZE; /* bytes */
    flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
        flash_info->protect[i] = 0;
    }

    printf ("flash size %dMB, sector count = %d\n", (CFG_FLASH_SIZE>>20), flash_info->sector_count);
    return (flash_info->size);

}
/*ledstat 0:on; 1:off*/
void board_power_led(int ledstat)
{
   GpioSet(AR7100_GPIO_OE,TEST_LED,1);
   GpioSet(AR7100_GPIO_OUT,TEST_LED,1);

   GpioSet(AR7100_GPIO_OE,POWER_LED,1);
   GpioSet(AR7100_GPIO_OUT,POWER_LED,ledstat);
}
/*ledstat 0:on; 1:off*/
void board_test_led(int ledstat)
{
   GpioSet(AR7100_GPIO_OE,TEST_LED,1);
   GpioSet(AR7100_GPIO_OUT,TEST_LED,ledstat);

   GpioSet(AR7100_GPIO_OE,POWER_LED,1);
   GpioSet(AR7100_GPIO_OUT,POWER_LED,1);
}
/*return value 0:not pressed, 1:pressed*/
int board_reset_button_is_press(void)
{
	if(GpioGet(AR7100_GPIO_IN,RESET_BUTTON_GPIO))
		return 0;
	return 1;
}
void board_reset_default_LedSet(void)
{
       static int DiagnosLedCount = 1;
       if ((DiagnosLedCount++ % 2)== 1)
        {
               /*power on test led 0.25s*/
		board_test_led(0);
		NetSetTimeout ((CFG_HZ*1)/4, board_reset_default_LedSet);
         }
         else{
               /*power off test led 0.75s*/
		board_test_led(1);
		NetSetTimeout ((CFG_HZ*3)/4, board_reset_default_LedSet);
         }
}
void Update_LedSet(void)
{
#ifdef CFG_NMRP
	if(NmrpState != 0)
		return;
#endif
	static int DiagnosLedCount = 1;
	if ((DiagnosLedCount++ % 2)== 1)
	{
		/*power on test led 0.25s*/
		board_power_led(0);
		NetSetTimeout ((CFG_HZ*1)/4,Update_LedSet);
	}
	else{
		/*power off test led 0.75s*/
		board_power_led(1);
		NetSetTimeout ((CFG_HZ*3)/4, Update_LedSet);
	}
}
/*erase the config sector of flash*/
void board_reset_default(void)
{
    printf("Restore to factory default\n");
    flash_sect_erase (CFG_FLASH_CONFIG_BASE, CFG_FLASH_CONFIG_BASE+CFG_FLASH_CONFIG_PARTITION_SIZE-1);
#ifdef CFG_NMRP
    if(NmrpState != 0)
        return;
#endif
    printf("Rebooting...\n");
    do_reset(NULL,0,0,NULL);
}

#if defined(NETGEAR_BOARD_ID_SUPPORT)
/*
 * item_name_want could be "device" to get Model Id, "version" to get Version
 * or "hd_id" to get Hardware ID.
 */
void board_get_image_info(ulong fw_image_addr, char *item_name_want, char *buf)
{
    char image_header[HEADER_LEN];
    char item_name[HEADER_LEN];
    char *item_value;
    char *parsing_string;

    memset(image_header, 0, HEADER_LEN);
    memcpy(image_header, (unsigned char *)fw_image_addr, HEADER_LEN);

    parsing_string = strtok(image_header, "\n");
    while (parsing_string != NULL) {
       memset(item_name, 0, sizeof(item_name));
       strncpy(item_name, parsing_string, (int)(strchr(parsing_string, ':') - parsing_string));

       if (strcmp(item_name, item_name_want) == 0) {
           item_value = strstr(parsing_string, ":") + 1;

           memcpy(buf, item_value, strlen(item_value));
       }

       parsing_string = strtok(NULL, "\n");
    }
}

int board_match_image_hw_id (ulong fw_image_addr)
{
    char board_hw_id[BOARD_HW_ID_LENGTH + 1];
    char image_hw_id[BOARD_HW_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_version[32];

    /* For wnr2000v3 firmware version <= V1.1.1.39, do not support HW ID yet.
     * We do not support HW ID in these old firmware image.
     */
    memset(image_model_id, 0, sizeof(image_model_id));
    memset(image_version, 0, sizeof(image_version));
    board_get_image_info(fw_image_addr, "device", (char*)image_model_id);
    board_get_image_info(fw_image_addr, "version", (char*)image_version);

    /*get hardward id from board */
    memset(board_hw_id, 0, sizeof(board_hw_id));
    memcpy(board_hw_id, (void *)(BOARDCAL + BOARD_HW_ID_OFFSET),
           BOARD_HW_ID_LENGTH);
    printf("HW ID on board: %s\n", board_hw_id);

    /*get hardward id from image */
    memset(image_hw_id, 0, sizeof(image_hw_id));
    board_get_image_info(fw_image_addr, "hd_id", (char*)image_hw_id);
    printf("HW ID on image: %s\n", image_hw_id);

    if (memcmp(board_hw_id, image_hw_id, BOARD_HW_ID_LENGTH) != 0) {
            printf("Firmware Image HW ID do not match Board HW ID\n");
            return 0;
    }
    printf("Firmware Image HW ID matched Board HW ID\n\n");
    return 1;
}

int board_match_image_model_id (ulong fw_image_addr)
{
    char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];

    /*get hardward id from board */
    memset(board_model_id, 0, sizeof(board_model_id));
    memcpy(board_model_id, (void *)(BOARDCAL + BOARD_MODEL_ID_OFFSET),
           BOARD_MODEL_ID_LENGTH);
    printf("MODEL ID on board: %s\n", board_model_id);

    /*get hardward id from image */
    memset(image_model_id, 0, sizeof(image_model_id));
    board_get_image_info(fw_image_addr, "device", (char*)image_model_id);
    printf("MODEL ID on image: %s\n", image_model_id);

    if (memcmp(board_model_id, image_model_id, BOARD_MODEL_ID_LENGTH) != 0) {
            printf("Firmware Image MODEL ID do not match Board model ID\n");
            return 0;
    }
    printf("Firmware Image MODEL ID matched Board model ID\n\n");
    return 1;
}

void board_update_image_model_id (ulong fw_image_addr)
{
    char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char image_model_id[BOARD_MODEL_ID_LENGTH + 1];
    char sectorBuff[CFG_FLASH_SECTOR_SIZE];

    /* Read boarddata */
    memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    /*get hardward id from board */
    memset(board_model_id, 0, sizeof(board_model_id));
    memcpy(board_model_id, (void *)(BOARDCAL + BOARD_MODEL_ID_OFFSET),
           BOARD_MODEL_ID_LENGTH);
    printf("Original board MODEL ID: %s\n", board_model_id);

    /*get hardward id from image */
    memset(image_model_id, 0, sizeof(image_model_id));
    board_get_image_info(fw_image_addr, "device", (char*)image_model_id);
    printf("New MODEL ID from image: %s\n", image_model_id);

    printf("Updating MODEL ID\n");
    memcpy(sectorBuff + BOARD_MODEL_ID_OFFSET, image_model_id,
	    BOARD_MODEL_ID_LENGTH);
    flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
    write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    printf("done\n\n");
}
#endif	/* BOARD_ID */
