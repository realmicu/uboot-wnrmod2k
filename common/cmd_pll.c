/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * SPI Read/Write Utilities
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <config.h>
#include <ar7240_soc.h>

#if (CONFIG_COMMANDS & CFG_CMD_PLL)

/*-----------------------------------------------------------------------
 * Definitions
 */

static void prepare_flash(char **buff_ptr,long **val) 
{
	if((long)*buff_ptr % 4)
		*buff_ptr = *buff_ptr + ((long)*buff_ptr % 4);
        memcpy(*buff_ptr,(void *)((long)PLL_FLASH_ADDR),CFG_FLASH_SECTOR_SIZE);
	flash_sect_erase(PLL_FLASH_ADDR,(PLL_FLASH_ADDR + CFG_FLASH_SECTOR_SIZE - 1));
	*val = (long *)(*buff_ptr + (CFG_FLASH_SECTOR_SIZE - 0x10));
}

int do_pll (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char  *buff,*buff_ptr = NULL;
	char *cmd_str = NULL;
	long *val;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	/* last sector of uboot 0xbf040000 - 0xbf050000
	 * write MAGIC at 0xbf04fff0 
	 * write value at 0xbf04fff4
         */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc == 2) {
			cmd_str = argv[1];
		}
		else {
			printf("Invalid number of argument\n");
			return -1;
		}
	}

	buff = (char *) malloc(CFG_FLASH_SECTOR_SIZE + 4);
	buff_ptr = buff;

	if (strcmp(cmd_str,"400_400_200") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val  = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                    (0x28 << PLL_CONFIG_PLL_DIV_SHIFT)| (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"400_400_100") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val  =  ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x1 << PLL_CONFIG_AHB_DIV_SHIFT) |
	             (0x28 << PLL_CONFIG_PLL_DIV_SHIFT)|(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"360_360_180") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val    = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
	              (0x24 << PLL_CONFIG_PLL_DIV_SHIFT)| (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"350_350_175") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
           *val  = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                      (0x23 << PLL_CONFIG_PLL_DIV_SHIFT)| (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"340_340_170") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
	             (0x22 << PLL_CONFIG_PLL_DIV_SHIFT)| (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"320_320_160") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT)|(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)|
	             (0x20 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"320_320_80") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x1 << PLL_CONFIG_AHB_DIV_SHIFT) |
 	             (0x20 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"300_300_150") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
 	            (0x1e << PLL_CONFIG_PLL_DIV_SHIFT) |(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}		
	else if (strcmp(cmd_str,"300_300_75") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) |(0x1 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x1e << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"200_200_100") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x14 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
        else if (strcmp(cmd_str,"370_370_185") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x25 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
        else if (strcmp(cmd_str,"380_380_190") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x26 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
         else if (strcmp(cmd_str,"390_390_195") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x27 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
        else if (strcmp(cmd_str,"410_410_205") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x29 << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
        else if (strcmp(cmd_str,"420_420_210") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x2a << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
        else if (strcmp(cmd_str,"430_430_215") == 0) {
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val   = ((0x0 << PLL_CONFIG_DDR_DIV_SHIFT) | (0x0 << PLL_CONFIG_AHB_DIV_SHIFT) |
                     (0x2b << PLL_CONFIG_PLL_DIV_SHIFT) | (0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT));
	}
	else if (strcmp(cmd_str,"erase") == 0) {
	   prepare_flash(&buff_ptr,&val);
	printf("erase Programmed val:%x value:0x%0.8X\n",val,*(val));
	   *val++ = (long)0xffffffff;
	   *val   = (long)0xffffffff;
        }
	else {
	   printf("Writing new configuration\n");
	   prepare_flash(&buff_ptr,&val);
	   *val++ = (long)PLL_MAGIC;
	   *val = simple_strtoul(cmd_str, NULL, 10);
	}

	flash_write(buff_ptr,(long)(PLL_FLASH_ADDR),CFG_FLASH_SECTOR_SIZE);
	printf("Programmed value:0x%0.8X\n",*(val));
	free(buff);

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	pll,	6,	1,	do_pll,
	"pll    - Set to change CPU/AHB/DDR speeds\n",
	"<value>  - string pll_300_300_150\n"
);

#endif	/* CFG_CMD_SPI */
