#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <net.h>	/* for NetSetTimeout() */
#include <linux/string.h>
#include "ar7240_soc.h"
#ifdef CFG_NMRP
extern int NmrpState;
extern ulong NmrpAliveTimerStart;
extern ulong NmrpAliveTimerBase;
extern int NmrpAliveTimerTimeout;
extern void NmrpSend(void);
#endif

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);

extern flash_info_t flash_info[];

/* Defined in board/ar7240/wnr1000v2/ar9285gpio.c */
extern u32 ar9285GpioGet(unsigned int gpio);

/* Defined in cpu/mips/cpu.c */
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void
ar7240_gpio_config_output(int gpio)
{
    ar7240_reg_rmw_set(AR7240_GPIO_OE, (1 << gpio));
}

void
ar7240_gpio_config_input(int gpio)
{
    ar7240_reg_rmw_clear(AR7240_GPIO_OE, (1 << gpio));
}

void
ar7240_gpio_out_val(int gpio, int val)
{
    if (val & 0x1) {
        ar7240_reg_rmw_set(AR7240_GPIO_OUT, (1 << gpio));
    }
    else {
        ar7240_reg_rmw_clear(AR7240_GPIO_OUT, (1 << gpio));
    }
}

int
ar7240_gpio_in_val(int gpio)
{
    return((1 << gpio) & (ar7240_reg_rd(AR7240_GPIO_IN)));
}

int
ar7240GpioGet(int gpio)
{
    ar7240_gpio_config_input(gpio);
    return ar7240_gpio_in_val(gpio);
}

void
ar7240GpioSet(int gpio, int val)
{
    ar7240_gpio_config_output(gpio);
    ar7240_gpio_out_val(gpio, val);
}

void
ar7240_usb_initial_config(void)
{
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0a04081e);
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0804081e);
}

void ar7240_gpio_config(void)
{
    /* Disable clock obs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0xffe7e0ff));
    /* Enable eth Switch LEDs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xf8));
}

int
ar7240_mem_config(void)
{
    unsigned int tap_val1, tap_val2;
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

/* Default tap values for starting the tap_init*/
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);

    /* ar7240 gpio would be configured in start.S, so don't need to be set again here.*/
    /* ar7240_gpio_config(); */
    ar7240_ddr_tap_init();

    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);
    printf("#### TAP VALUE 1 = %x, 2 = %x\n",tap_val1, tap_val2);

    ar7240_usb_initial_config();

    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7240_mem_config());
}

#if defined(CONFIG_WNR1000V2_VC)
static void drawline(void)
{
    printf("--------------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf("WNR1000v2-VC (ar7240) U-boot dni25 V1.7\n"); */
    drawline();
    printf(" WNR1000v2-VC (ar7240), %d MHz, 32 MB RAM, %d MB flash\n", CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}
#elif defined(CONFIG_WNR1000V2_VM)
static void drawline(void)
{
    printf("--------------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf(" WNR1000v2-VM (ar7240) U-boot dni25 V1.7\n"); */
    drawline();
    printf("WNR1000v2-VM (ar7240), %d MHz, 32 MB RAM, %d MB flash\n", CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}
#else
static void drawline(void)
{
    printf("-----------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf("WNR1000v2 (ar7240) U-boot dni25 V1.8\n"); */
    drawline();
    printf(" WNR1000v2 (ar7240), %d MHz, 32 MB RAM, %d MB flash\n", CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}
#endif

uint32_t GetMagicNumberOfWnr1000v2(void)
{
    return ((uint32_t)(simple_strtoul(getenv("magic_number"),NULL,16))?(simple_strtoul(getenv("magic_number"),NULL,16)):(IH_MAGIC_DEFAULT));
}

/*ledstat 0:on; 1:off*/
void wnr1000v2_power_led(int ledstat)
{
    ar7240GpioSet(TEST_LED,1);
    ar7240GpioSet(TEST_LED,1);

    ar7240GpioSet(POWER_LED,1);
    ar7240GpioSet(POWER_LED,ledstat);
}

/*ledstat 0:on; 1:off*/
void wnr1000v2_test_led(int ledstat)
{
    ar7240GpioSet(TEST_LED,1);
    ar7240GpioSet(TEST_LED,ledstat);

    ar7240GpioSet(POWER_LED,1);
    ar7240GpioSet(POWER_LED,1);
}

void wnr1000v2_reset_default_LedSet(void)
{
    static int DiagnosLedCount = 1;
    if ((DiagnosLedCount++ % 2)== 1)
    {
	/*power on test led 0.25s*/
	wnr1000v2_test_led(0);
	NetSetTimeout ((CFG_HZ*1)/4, wnr1000v2_reset_default_LedSet);
    }else{
	/*power off test led 0.75s*/
        wnr1000v2_test_led(1);
        NetSetTimeout ((CFG_HZ*3)/4, wnr1000v2_reset_default_LedSet);
    }
}

/*return value 0: not pressed, 1: pressed*/
int wnr1000v2_reset_button_is_press(void)
{
    if(ar9285GpioGet(RESET_BUTTON_GPIO))
	return 0;
    return 1;
}

extern int flash_sect_erase (ulong, ulong);

/*erase the config sector of flash*/
void wnr1000v2_reset_default(void)
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

#if defined(CFG_SINGLE_FIRMWARE)
void board_upgrade_string_table(unsigned char *load_addr, int table_number, unsigned int file_size)
{
    unsigned char *string_table_addr, *addr2;
    unsigned long offset;
    unsigned int table_length;
    unsigned char high_bit, low_bit;
    unsigned long passed;
    int offset_num;
    uchar *src_addr;
    ulong target_addr;

    /* Read whole string table partition from Flash to RAM(load_addr+128k) */
    string_table_addr = load_addr + 0x20000;
    memset(string_table_addr, 0, CFG_STRING_TABLE_TOTAL_LEN);
    memcpy(string_table_addr, (unsigned char *)CFG_STRING_TABLE_ADDR_BEGIN,
	   CFG_STRING_TABLE_TOTAL_LEN);

    /* Save string table checksum to (CFG_STRING_TABLE_LEN - 1) */
    memcpy(load_addr + CFG_STRING_TABLE_LEN - 1, load_addr + file_size- 1, 1);
    /* Remove checksum from string table file's tail */
    memset(load_addr + file_size - 1, 0, 1);

    table_length = file_size - 1;
    printf("string table length is %d\n", table_length);

    /* Save (string table length / 1024)to CFG_STRING_TABLE_LEN-3 */
    high_bit = table_length / 1024;
    addr2 = load_addr + CFG_STRING_TABLE_LEN - 3;
    memcpy(addr2, &high_bit, sizeof(high_bit));

    /* Save (string table length % 1024)to CFG_STRING_TABLE_LEN-2 */
    low_bit = table_length % 1024;
    addr2 = load_addr + CFG_STRING_TABLE_LEN - 2;
    memcpy(addr2, &low_bit, sizeof(low_bit));

    /* Copy processed string table from load_addr to RAM */
    offset = (table_number - 1) * CFG_STRING_TABLE_LEN;
    memcpy(string_table_addr + offset, load_addr, CFG_STRING_TABLE_LEN);

    /* Write back string tables from RAM to Flash */
    printf("erase %x to %x\n", CFG_STRING_TABLE_ADDR_BEGIN, CFG_STRING_TABLE_ADDR_END);
    for (offset_num = 0; offset_num < (CFG_STRING_TABLE_TOTAL_LEN / CFG_FLASH_SECTOR_SIZE); offset_num++) {
        /* erase 64K */
        flash_sect_erase (CFG_STRING_TABLE_ADDR_BEGIN + offset_num * CFG_FLASH_SECTOR_SIZE,\
                          CFG_STRING_TABLE_ADDR_BEGIN + ((offset_num+1) * CFG_FLASH_SECTOR_SIZE) - 1);

        /* Check if Alive-timer expires? */
        passed = get_timer(NmrpAliveTimerStart);
        if ((passed / CFG_HZ) + NmrpAliveTimerBase > NmrpAliveTimerTimeout) {
            printf("Active-timer expires\n");
            NmrpSend();
            NmrpAliveTimerBase = NmrpAliveTimerTimeout / 4;
            NmrpAliveTimerStart = get_timer(0);
        } else {
            printf("Alive-timer %u\n",(passed / CFG_HZ) + NmrpAliveTimerBase);
            /* If passed 1/4 NmrpAliveTimerTimeout,
             * add 1/4 NmrpAliveTimerTimeout to NmrpAliveTimerBase.
             * This is for avoiding "passed" overflow.
             */
            if ((passed) / CFG_HZ >= (NmrpAliveTimerTimeout / 4)) {
                NmrpAliveTimerBase += NmrpAliveTimerTimeout / 4;
                NmrpAliveTimerStart = get_timer(0);
            }
        }
    }

    printf ("Copy all string tables to Flash...\n");
    for (offset_num = 0; offset_num < (CFG_STRING_TABLE_TOTAL_LEN / CFG_FLASH_SECTOR_SIZE); offset_num++) {
        src_addr = string_table_addr + offset_num * CFG_FLASH_SECTOR_SIZE;
        target_addr = CFG_STRING_TABLE_ADDR_BEGIN + offset_num * CFG_FLASH_SECTOR_SIZE;
        flash_write( src_addr, target_addr, CFG_FLASH_SECTOR_SIZE);

	/* Check if Alive-timer expires? */
	passed = get_timer(NmrpAliveTimerStart);
        if ((passed / CFG_HZ) + NmrpAliveTimerBase > NmrpAliveTimerTimeout) {
            printf("Active-timer expires\n");
            NmrpSend();
            NmrpAliveTimerBase = NmrpAliveTimerTimeout / 4;
            NmrpAliveTimerStart = get_timer(0);
        } else {
            printf("Alive-timer %u\n",(passed / CFG_HZ) + NmrpAliveTimerBase);
            /* If passed 1/4 NmrpAliveTimerTimeout,
             * add 1/4 NmrpAliveTimerTimeout to NmrpAliveTimerBase.
             * This is for avoiding "passed" overflow.
             */
            if ((passed) / CFG_HZ >= (NmrpAliveTimerTimeout / 4)) {
                NmrpAliveTimerBase += NmrpAliveTimerTimeout / 4;
                NmrpAliveTimerStart = get_timer(0);
            }
        }
    }
    return;
}
#endif /* defined(CFG_SINGLE_FIRMWARE) */

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
    memcpy(image_header, (char *)fw_image_addr, HEADER_LEN);

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
    unsigned char board_hw_id[BOARD_HW_ID_LENGTH + 1];
    unsigned char image_hw_id[BOARD_HW_ID_LENGTH + 1];
    unsigned char image_model_id[BOARD_MODEL_ID_LENGTH + 1];
    unsigned char image_version[20];

    /* For wnr1000v2 firmware old version, do not support HW ID yet.
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

    if (strcmp(image_model_id ,"WNR1000v2") == 0) {
        printf("WNR1000v2 Firmware, check special case\n");
	    if (board_hw_id[0] == 0xff || board_hw_id[0] == 0x0		/* no hw id in board */
         || image_hw_id[0] == 0xff || image_hw_id[0] == 0x0)	/* no hw id in image */
        {
            printf("WNR1000v2 Firmware, Ignore HW ID checking\n\n");
            return 1;
        }
        else
        {
            printf("WNR1000v2 Firmware, need check HW ID\n\n");
        }
    }

    if (memcmp(board_hw_id, image_hw_id, BOARD_HW_ID_LENGTH) != 0) {
        printf("Firmware Image HW ID do not match Board HW ID\n");
        return 0;
    }
    printf("Firmware Image HW ID matched Board HW ID\n\n");
	return 1;
}

int board_match_image_model_id (ulong fw_image_addr)
{
    unsigned char board_model_id[BOARD_MODEL_ID_LENGTH + 1];
    unsigned char image_model_id[BOARD_MODEL_ID_LENGTH + 1];

    /*get hardward id from board */
    memset(board_model_id, 0, sizeof(board_model_id));
    memcpy(board_model_id, (void *)(BOARDCAL + BOARD_MODEL_ID_OFFSET),
           BOARD_MODEL_ID_LENGTH);
    printf("MODEL ID on board: %s\n", board_model_id);
    if (board_model_id[0] == 0xff || board_model_id[0] == 0x0) {
        printf("MODEL ID not set into board, ignore board model id check!\n");
        return 1;
    }

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
#endif /* BOARD_ID */
