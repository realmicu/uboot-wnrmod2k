#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <net.h>
#include "ar7240_soc.h"
#ifdef CFG_NMRP
extern int NmrpState;
extern ulong NmrpAliveTimerStart;
extern ulong NmrpAliveTimerBase;
extern int NmrpAliveTimerTimeout;
extern void NmrpSend(void);
#endif

/*********************** AR9287 ************************/
#define AR9287_BASE 0X10000000

#define AR9287_NUM_GPIO 10

#define AR9287_GPIO_IN_OUT                           0x4048 // GPIO input / output register
#define AR9287_GPIO_IN_VAL                           0x001FF800
#define AR9287_GPIO_IN_VAL_S                         11

#define AR9287_GPIO_OE_OUT                           0x404c // GPIO output enable register
#define AR9287_GPIO_OE_OUT_DRV                       0x3    // 2 bit field mask, shifted by 2*bitpos
#define AR9287_GPIO_OE_OUT_DRV_NO                    0x0    // tristate
#define AR9287_GPIO_OE_OUT_DRV_LOW                   0x1    // drive if low
#define AR9287_GPIO_OE_OUT_DRV_HI                    0x2    // drive if high
#define AR9287_GPIO_OE_OUT_DRV_ALL                   0x3    // drive always

#define AR9287_GPIO_OUTPUT_MUX1                      0x4060
#define AR9287_GPIO_OUTPUT_MUX2                      0x4064
#define AR9287_GPIO_IE_VALUE                         0x4054 // GPIO input enable and value register

#define AR9287_GPIO_OUTPUT_MUX_AS_OUTPUT             0
#define AR9287_GPIO_OUTPUT_MUX_AS_PCIE_ATTENTION_LED 1
#define AR9287_GPIO_OUTPUT_MUX_AS_PCIE_POWER_LED     2
#define AR9287_GPIO_OUTPUT_MUX_AS_TX_FRAME           3
#define AR9287_GPIO_OUTPUT_MUX_AS_RX_CLEAR_EXTERNAL  4
#define AR9287_GPIO_OUTPUT_MUX_AS_MAC_NETWORK_LED    5
#define AR9287_GPIO_OUTPUT_MUX_AS_MAC_POWER_LED      6

#define MS(_v, _f)  (((_v) & _f) >> _f##_S)

#define AR9287_GPIO_BIT(_gpio) (1 << (_gpio))

#define ar9287_reg_wr(_off, _val) ar7240_reg_wr(((_off) + AR9287_BASE), (_val))
#define ar9287_reg_rd(_off) (ar7240_reg_rd((_off) + AR9287_BASE))
#define ar9287_reg_rmw(_off, _set, _clr) do { \
            ar7240_reg_rmw_clear(((_off) + AR9287_BASE), (_clr)); \
            ar7240_reg_rmw_set(((_off) + AR9287_BASE), (_set)); \
} while(0)

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);

extern flash_info_t flash_info[];

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*
 * Configure GPIO Output Mux control
 */
static void ar9287GpioCfgOutputMux(unsigned int gpio, u32 type)
{
    int          addr;
    u32    gpio_shift;

    // MUX1 control 6 GPIOs(0-5), MUX2 control 4 GPIOs(6-9).
    if (gpio > 5) {
        addr = AR9287_GPIO_OUTPUT_MUX2;
    } else {
        addr = AR9287_GPIO_OUTPUT_MUX1;
    }

    // 5 bits per GPIO pin. Bits 0..4 for 1st pin in that mux, bits 5..9 for 2nd pin, etc.
    gpio_shift = (gpio % 6) * 5;

    ar9287_reg_rmw(addr, (type << gpio_shift), (0x1f << gpio_shift));
}

/*
 * Configure GPIO Output lines
 */
void ar9287GpioCfgOutput(unsigned int gpio)
{
    u32    gpio_shift;

    if (gpio >= AR9287_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }
    // Configure the MUX
    ar9287GpioCfgOutputMux(gpio, AR9287_GPIO_OUTPUT_MUX_AS_OUTPUT);

    // 2 bits per output mode
    gpio_shift = 2*gpio;

    ar9287_reg_rmw(AR9287_GPIO_OE_OUT,
               (AR9287_GPIO_OE_OUT_DRV_ALL << gpio_shift),
               (AR9287_GPIO_OE_OUT_DRV << gpio_shift));
}

/*
 * Configure GPIO Input lines
 */
void ar9287GpioCfgInput(unsigned int gpio)
{
    u32    gpio_shift;

    if (gpio >= AR9287_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }
    /* TODO: configure input mux for AR9287 */
    /* If configured as input, set output to tristate */
    gpio_shift = 2*gpio;

    ar9287_reg_rmw(AR9287_GPIO_OE_OUT,
               (AR9287_GPIO_OE_OUT_DRV_NO << gpio_shift),
               (AR9287_GPIO_OE_OUT_DRV << gpio_shift));
}

/*
 * Once configured for I/O - set output lines
 */
void ar9287GpioSet(unsigned int gpio, int val)
{
    if (gpio >= AR9287_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }
    ar9287_reg_rmw(AR9287_GPIO_IN_OUT, ((val&1) << gpio), AR9287_GPIO_BIT(gpio));
}

/*
 * Once configured for I/O - get input lines
 */
u32 ar9287GpioGet(unsigned int gpio)
{
    if (gpio >= AR9287_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return 0xffffffff;
    }
    // Read output value for all gpio's, shift it left, and verify whether a
    // specific gpio bit is set.
    return (MS(ar9287_reg_rd(AR9287_GPIO_IN_OUT), AR9287_GPIO_IN_VAL) & AR9287_GPIO_BIT(gpio)) != 0;
}


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
#ifdef CONFIG_K31
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xd8));
#else
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xfa));
#endif
}

int
ar7240_mem_config(void)
{
    unsigned int tap_val1, tap_val2;
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

    /* Default tap values for starting the tap_init*/
    if (!(is_ar7241() || is_ar7242()))  {
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);
        ar7240_ddr_tap_init();
    }
    else {
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL2, 0x0);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL3, 0x0);
    }

    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);

    printf("#### TAP VALUE 1 = 0x%x, 2 = 0x%x [0x%x: 0x%x]\n",
                tap_val1, tap_val2, *(unsigned *)0x80500000,
                *(unsigned *)0x80500004);
    ar7240_usb_initial_config();
    ar7240_gpio_config();

    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7240_mem_config());
}

static void drawline(void)
{
    printf("---------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf("WNR2200 (ar7241) U-boot dni25 V1.5\n"); */
    drawline();
    printf(" WNR2200 (ar7241), %d MHz, 64 MB RAM, %d MB flash\n", CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}

uint32_t get_magic_number_of_wnr2200(void)
{
    return ((uint32_t)(simple_strtoul(getenv("magic_number"),NULL,16))?(simple_strtoul(getenv("magic_number"),NULL,16)):(IH_MAGIC_DEFAULT));
}

/*ledstat 0:on; 1:off*/
void board_power_led(int ledstat)
{
    ar9287GpioCfgOutput(TEST_LED);
    ar9287GpioSet(TEST_LED,1);

    ar9287GpioCfgOutput(POWER_LED);
    ar9287GpioSet(POWER_LED,ledstat);
}

/*ledstat 0:on; 1:off*/
void board_test_led(int ledstat)
{
    ar9287GpioCfgOutput(TEST_LED);
    ar9287GpioSet(TEST_LED,ledstat);

    ar9287GpioCfgOutput(POWER_LED);
    ar9287GpioSet(POWER_LED,1);
}

void board_reset_default_LedSet(void)
{
    static int DiagnosLedCount = 1;
    if ((DiagnosLedCount++ % 2)== 1)
    {
	/*power on test led 0.25s*/
	board_test_led(0);
	NetSetTimeout ((CFG_HZ*1)/4, board_reset_default_LedSet);
    }else{
	/*power off test led 0.75s*/
        board_test_led(1);
        NetSetTimeout ((CFG_HZ*3)/4, board_reset_default_LedSet);
    }
}

/*return value 0: not pressed, 1: pressed*/
int board_reset_button_is_press(void)
{
    ar9287GpioCfgInput(RESET_BUTTON_GPIO);
    if(ar9287GpioGet(RESET_BUTTON_GPIO))
	return 0;
    return 1;
}

extern int flash_sect_erase (ulong, ulong);

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

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
	/* To avoid amber led gpio1 pulled down, force its level to high */
	/* According to wnr2000v3 HW engineer Bennette said, there are */
	/* 10 pcs out of 100 may have this issue */
	printf("Misc:  force GPIO1 output high\n");
	ar7240_gpio_out_val(1, 0x1);
	return 0;
}
#endif

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
