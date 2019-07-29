#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <net.h>	/* for NetSetTimeout() */
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

/* Defined in board/ar7240/wnr612/ar9285gpio.c */
extern void ar9285DisableJTAG(unsigned int uDisable);
extern void ar9285GpioCfgOutput(unsigned int gpio);
extern void ar9285GpioSet(unsigned int gpio, int val);
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

#ifndef RAM_VERSION	
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

/* Default tap values for starting the tap_init*/
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);

    /* ar7240 gpio would be configured in start.S, so don't need to be set again here.*/
    /*
    ar7240_gpio_config();
    */
    ar7240_ddr_tap_init();
#endif

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

static void drawline(void)
{
    printf("----------------------------------------------------\n");
}

int checkboard (void)
{
    /* printf("U-boot WNR612 V0.2, built on %s\n", __DATE__); */
    char *soc_model;
    if (is_ar7240())
	soc_model = "ar7240";
    else if (is_ar7241())
	soc_model = "ar7241";
    else if (is_ar7242())
	soc_model = "ar7242";
    else
	soc_model = "ar7xxx";
    drawline();
    printf(" WNR612v2 (%s), %d MHz, 32 MB RAM, %d MB flash\n", soc_model, CFG_HZ/500000, FLASH_SIZE);
    drawline();
    return 0;
}

uint32_t GetMagicNumberOfWnr612(void)
{
    return ((uint32_t)(simple_strtoul(getenv("magic_number"),NULL,16))?(simple_strtoul(getenv("magic_number"),NULL,16)):(IH_MAGIC_DEFAULT));
}

/*ledstat 0:on; 1:off*/
void wnr612_power_led(int ledstat)
{
//    ar7240GpioSet(TEST_LED,1);
//    ar7240GpioSet(TEST_LED,1);

    ar7240GpioSet(POWER_LED,1);
    ar7240GpioSet(POWER_LED,ledstat);
}

/*ledstat 0:on; 1:off*/
void wnr612_test_led(int ledstat)
{
    ar7240GpioSet(TEST_LED,1);
    ar7240GpioSet(TEST_LED,ledstat);

//    ar7240GpioSet(POWER_LED,1);
//    ar7240GpioSet(POWER_LED,1);
}

/*
void wnr612_jtag( unsigned int uDisable)
{
	ar9285DisableJTAG(uDisable);
}
*/

/*ledstat 0:on; 1:off*/
/*
void wnr612_wlan_led(int ledstat)
{
static int iCfgInit = 0;

	if( !iCfgInit )
	{
		ar9285GpioCfgOutput(WLAN_LED);
		iCfgInit = 1;
	}
	ar9285GpioSet(WLAN_LED, ledstat );
}
*/

void wnr612_reset_default_LedSet(void)
{
    static int DiagnosLedCount = 1;
    if ((DiagnosLedCount++ % 2)== 1)
    {
	/*power on test led 0.25s*/
	wnr612_test_led(0);
	NetSetTimeout ((CFG_HZ*1)/4, wnr612_reset_default_LedSet);
    }else{
	/*power off test led 0.75s*/
        wnr612_test_led(1);
        NetSetTimeout ((CFG_HZ*3)/4, wnr612_reset_default_LedSet);
    }
}

/*return value 0: not pressed, 1: pressed*/
int wnr612_reset_button_is_press(void)
{
    if(ar9285GpioGet(RESET_BUTTON_GPIO))
	return 0;
    return 1;
}

extern int flash_sect_erase (ulong, ulong);

/*erase the config sector of flash*/
void wnr612_reset_default(void)
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
