/*
 * (C) Copyright 2001
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
 * MII Utilities
 */

#include <common.h>
#include <command.h>

#ifndef COMPRESSED_UBOOT
#if (CONFIG_COMMANDS & CFG_CMD_MII)
#include <miiphy.h>

extern flash_info_t flash_info[];	/* info for FLASH chips */

#ifdef CONFIG_TERSE_MII
/*
 * Display values from last command.
 */
uint last_op;
uint last_addr;
uint last_data;
uint last_reg;

/*
 * MII device/info/read/write
 *
 * Syntax:
 *  mii device {devname}
 *  mii info   {addr}
 *  mii read   {addr} {reg}
 *  mii write  {addr} {reg} {data}
 */
int do_mii (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char		op;
	unsigned char	addr, reg;
	unsigned short	data;
	int		rcode = 0;
	char		*devname;

#if defined(CONFIG_8xx) || defined(CONFIG_MCF52x2)
	mii_init ();
#endif

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op   = last_op;
	addr = last_addr;
	data = last_data;
	reg  = last_reg;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		op = argv[1][0];
		if (argc >= 3)
			addr = simple_strtoul (argv[2], NULL, 16);
		if (argc >= 4)
			reg  = simple_strtoul (argv[3], NULL, 16);
		if (argc >= 5)
			data = simple_strtoul (argv[4], NULL, 16);
	}

	/* use current device */
	devname = miiphy_get_current_dev();

	/*
	 * check device/read/write/list.
	 */
	if (op == 'i') {
		unsigned char j, start, end;
		unsigned int oui;
		unsigned char model;
		unsigned char rev;

		/*
		 * Look for any and all PHYs.  Valid addresses are 0..31.
		 */
		if (argc >= 3) {
			start = addr; end = addr + 1;
		} else {
			start = 0; end = 31;
		}

		for (j = start; j < end; j++) {
			if (miiphy_info (devname, j, &oui, &model, &rev) == 0) {
				printf ("PHY 0x%02X: "
					"OUI = 0x%04X, "
					"Model = 0x%02X, "
					"Rev = 0x%02X, "
					"%3dbaseT, %s\n",
					j, oui, model, rev,
					miiphy_speed (devname, j),
					(miiphy_duplex (devname, j) == FULL)
						? "FDX" : "HDX");
			} else {
				puts ("Error reading info from the PHY\n");
			}
		}
	} else if (op == 'r') {
		if (miiphy_read (devname, addr, reg, &data) != 0) {
			puts ("Error reading from the PHY\n");
			rcode = 1;
		} else {
			printf ("%04X\n", data & 0x0000FFFF);
		}
	} else if (op == 'w') {
		if (miiphy_write (devname, addr, reg, data) != 0) {
			puts ("Error writing to the PHY\n");
			rcode = 1;
		}
	} else if (op == 'd') {
		if (argc == 2)
			miiphy_listdev ();
		else
			miiphy_set_current_dev (argv[2]);
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op = op;
	last_addr = addr;
	last_data = data;
	last_reg = reg;

	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	mii,	5,	1,	do_mii,
	"mii     - MII utility commands\n",
	"device                     - list available devices\n"
	"mii device <devname>           - set current device\n"
	"mii info   <addr>              - display MII PHY info\n"
	"mii read   <addr> <reg>        - read  MII PHY <addr> register <reg>\n"
	"mii write  <addr> <reg> <data> - write MII PHY <addr> register <reg>\n"
);

#else /* ! CONFIG_TERSE_MII ================================================= */

typedef struct _MII_reg_desc_t {
	ushort regno;
	char * name;
} MII_reg_desc_t;

MII_reg_desc_t reg_0_5_desc_tbl[] = {
	{ 0,   "PHY control register"                },
	{ 1,   "PHY status register"                 },
	{ 2,   "PHY ID 1 register"                   },
	{ 3,   "PHY ID 2 register"                   },
	{ 4,   "Autonegotiation advertisement register" },
	{ 5,   "Autonegotiation partner abilities register" },
};

typedef struct _MII_field_desc_t {
	ushort hi;
	ushort lo;
	ushort mask;
	char * name;
} MII_field_desc_t;

MII_field_desc_t reg_0_desc_tbl[] = {
	{ 15, 15, 0x01, "reset"                        },
	{ 14, 14, 0x01, "loopback"                     },
	{ 13,  6, 0x81, "speed selection"              }, /* special */
	{ 12, 12, 0x01, "A/N enable"                   },
	{ 11, 11, 0x01, "power-down"                   },
	{ 10, 10, 0x01, "isolate"                      },
	{  9,  9, 0x01, "restart A/N"                  },
	{  8,  8, 0x01, "duplex"                       }, /* special */
	{  7,  7, 0x01, "collision test enable"        },
	{  5,  0, 0x3f, "(reserved)"                   }
};

MII_field_desc_t reg_1_desc_tbl[] = {
	{ 15, 15, 0x01, "100BASE-T4 able"              },
	{ 14, 14, 0x01, "100BASE-X  full duplex able"  },
	{ 13, 13, 0x01, "100BASE-X  half duplex able"  },
	{ 12, 12, 0x01, "10 Mbps    full duplex able"  },
	{ 11, 11, 0x01, "10 Mbps    half duplex able"  },
	{ 10, 10, 0x01, "100BASE-T2 full duplex able"  },
	{  9,  9, 0x01, "100BASE-T2 half duplex able"  },
	{  8,  8, 0x01, "extended status"              },
	{  7,  7, 0x01, "(reserved)"                   },
	{  6,  6, 0x01, "MF preamble suppression"      },
	{  5,  5, 0x01, "A/N complete"                 },
	{  4,  4, 0x01, "remote fault"                 },
	{  3,  3, 0x01, "A/N able"                     },
	{  2,  2, 0x01, "link status"                  },
	{  1,  1, 0x01, "jabber detect"                },
	{  0,  0, 0x01, "extended capabilities"        },
};

MII_field_desc_t reg_2_desc_tbl[] = {
	{ 15,  0, 0xffff, "OUI portion"                },
};

MII_field_desc_t reg_3_desc_tbl[] = {
	{ 15, 10, 0x3f, "OUI portion"                },
	{  9,  4, 0x3f, "manufacturer part number"   },
	{  3,  0, 0x0f, "manufacturer rev. number"   },
};

MII_field_desc_t reg_4_desc_tbl[] = {
	{ 15, 15, 0x01, "next page able"               },
	{ 14, 14, 0x01, "reserved"                     },
	{ 13, 13, 0x01, "remote fault"                 },
	{ 12, 12, 0x01, "reserved"                     },
	{ 11, 11, 0x01, "asymmetric pause"             },
	{ 10, 10, 0x01, "pause enable"                 },
	{  9,  9, 0x01, "100BASE-T4 able"              },
	{  8,  8, 0x01, "100BASE-TX full duplex able"  },
	{  7,  7, 0x01, "100BASE-TX able"              },
	{  6,  6, 0x01, "10BASE-T   full duplex able"  },
	{  5,  5, 0x01, "10BASE-T   able"              },
	{  4,  0, 0x1f, "xxx to do"                    },
};

MII_field_desc_t reg_5_desc_tbl[] = {
	{ 15, 15, 0x01, "next page able"               },
	{ 14, 14, 0x01, "acknowledge"                  },
	{ 13, 13, 0x01, "remote fault"                 },
	{ 12, 12, 0x01, "(reserved)"                   },
	{ 11, 11, 0x01, "asymmetric pause able"        },
	{ 10, 10, 0x01, "pause able"                   },
	{  9,  9, 0x01, "100BASE-T4 able"              },
	{  8,  8, 0x01, "100BASE-X full duplex able"   },
	{  7,  7, 0x01, "100BASE-TX able"              },
	{  6,  6, 0x01, "10BASE-T full duplex able"    },
	{  5,  5, 0x01, "10BASE-T able"                },
	{  4,  0, 0x1f, "xxx to do"                    },
};

#define DESC0LEN (sizeof(reg_0_desc_tbl)/sizeof(reg_0_desc_tbl[0]))
#define DESC1LEN (sizeof(reg_1_desc_tbl)/sizeof(reg_1_desc_tbl[0]))
#define DESC2LEN (sizeof(reg_2_desc_tbl)/sizeof(reg_2_desc_tbl[0]))
#define DESC3LEN (sizeof(reg_3_desc_tbl)/sizeof(reg_3_desc_tbl[0]))
#define DESC4LEN (sizeof(reg_4_desc_tbl)/sizeof(reg_4_desc_tbl[0]))
#define DESC5LEN (sizeof(reg_5_desc_tbl)/sizeof(reg_5_desc_tbl[0]))

typedef struct _MII_field_desc_and_len_t {
	MII_field_desc_t * pdesc;
	ushort len;
} MII_field_desc_and_len_t;

MII_field_desc_and_len_t desc_and_len_tbl[] = {
	{ reg_0_desc_tbl, DESC0LEN },
	{ reg_1_desc_tbl, DESC1LEN },
	{ reg_2_desc_tbl, DESC2LEN },
	{ reg_3_desc_tbl, DESC3LEN },
	{ reg_4_desc_tbl, DESC4LEN },
	{ reg_5_desc_tbl, DESC5LEN },
};

static void dump_reg(
	ushort             regval,
	MII_reg_desc_t   * prd,
	MII_field_desc_and_len_t * pdl);

static int special_field(
	ushort regno,
	MII_field_desc_t * pdesc,
	ushort regval);

void MII_dump_0_to_5(
	ushort regvals[6],
	uchar reglo,
	uchar reghi)
{
	ulong i;

	for (i = 0; i < 6; i++) {
		if ((reglo <= i) && (i <= reghi))
			dump_reg(regvals[i], &reg_0_5_desc_tbl[i],
				&desc_and_len_tbl[i]);
	}
}

static void dump_reg(
	ushort             regval,
	MII_reg_desc_t   * prd,
	MII_field_desc_and_len_t * pdl)
{
	ulong i;
	ushort mask_in_place;
	MII_field_desc_t * pdesc;

	printf("%u.     (%04hx)                 -- %s --\n",
		prd->regno, regval, prd->name);

	for (i = 0; i < pdl->len; i++) {
		pdesc = &pdl->pdesc[i];

		mask_in_place = pdesc->mask << pdesc->lo;

		printf("  (%04hx:%04hx) %u.",
			mask_in_place,
			regval & mask_in_place,
			prd->regno);

		if (special_field(prd->regno, pdesc, regval)) {
		}
		else {
			if (pdesc->hi == pdesc->lo)
				printf("%2u   ", pdesc->lo);
			else
				printf("%2u-%2u", pdesc->hi, pdesc->lo);
			printf(" = %5u    %s",
				(regval & mask_in_place) >> pdesc->lo,
				pdesc->name);
		}
		printf("\n");

	}
	printf("\n");
}

/* Special fields:
** 0.6,13
** 0.8
** 2.15-0
** 3.15-0
** 4.4-0
** 5.4-0
*/

static int special_field(
	ushort regno,
	MII_field_desc_t * pdesc,
	ushort regval)
{
	if ((regno == 0) && (pdesc->lo == 6)) {
		ushort speed_bits = regval & PHY_BMCR_SPEED_MASK;
		printf("%2u,%2u =   b%u%u    speed selection = %s Mbps",
			6, 13,
			(regval >>  6) & 1,
			(regval >> 13) & 1,
			speed_bits == PHY_BMCR_1000_MBPS ? "1000" :
			speed_bits == PHY_BMCR_100_MBPS  ? "100" :
			speed_bits == PHY_BMCR_10_MBPS   ? "10" :
			"???");
		return 1;
	}

	else if ((regno == 0) && (pdesc->lo == 8)) {
		printf("%2u    = %5u    duplex = %s",
			pdesc->lo,
			(regval >>  pdesc->lo) & 1,
			((regval >> pdesc->lo) & 1) ? "full" : "half");
		return 1;
	}

	else if ((regno == 4) && (pdesc->lo == 0)) {
		ushort sel_bits = (regval >> pdesc->lo) & pdesc->mask;
		printf("%2u-%2u = %5u    selector = %s",
			pdesc->hi, pdesc->lo, sel_bits,
			sel_bits == PHY_ANLPAR_PSB_802_3 ?
				"IEEE 802.3" :
			sel_bits == PHY_ANLPAR_PSB_802_9 ?
				"IEEE 802.9 ISLAN-16T" :
			"???");
		return 1;
	}

	else if ((regno == 5) && (pdesc->lo == 0)) {
		ushort sel_bits = (regval >> pdesc->lo) & pdesc->mask;
		printf("%2u-%2u =     %u    selector = %s",
			pdesc->hi, pdesc->lo, sel_bits,
			sel_bits == PHY_ANLPAR_PSB_802_3 ?
				"IEEE 802.3" :
			sel_bits == PHY_ANLPAR_PSB_802_9 ?
				"IEEE 802.9 ISLAN-16T" :
			"???");
		return 1;
	}

	return 0;
}

char last_op[2];
uint last_data;
uint last_addr_lo;
uint last_addr_hi;
uint last_reg_lo;
uint last_reg_hi;

static void extract_range(
	char * input,
	unsigned char * plo,
	unsigned char * phi)
{
	char * end;
	*plo = simple_strtoul(input, &end, 16);
	if (*end == '-') {
		end++;
		*phi = simple_strtoul(end, NULL, 16);
	}
	else {
		*phi = *plo;
	}
}

/* ---------------------------------------------------------------- */
int do_mii (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char		op[2];
	unsigned char	addrlo, addrhi, reglo, reghi;
	unsigned char	addr, reg;
	unsigned short	data;
	int		rcode = 0;
	char		*devname;

#ifdef CONFIG_8xx
	mii_init ();
#endif

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op[0] = last_op[0];
	op[1] = last_op[1];
	addrlo = last_addr_lo;
	addrhi = last_addr_hi;
	reglo  = last_reg_lo;
	reghi  = last_reg_hi;
	data   = last_data;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		op[0] = argv[1][0];
		if (strlen(argv[1]) > 1)
			op[1] = argv[1][1];
		else
			op[1] = '\0';

		if (argc >= 3)
			extract_range(argv[2], &addrlo, &addrhi);
		if (argc >= 4)
			extract_range(argv[3], &reglo, &reghi);
		if (argc >= 5)
			data = simple_strtoul (argv[4], NULL, 16);
	}

	/* use current device */
	devname = miiphy_get_current_dev();

	/*
	 * check info/read/write.
	 */
	if (op[0] == 'i') {
		unsigned char j, start, end;
		unsigned int oui;
		unsigned char model;
		unsigned char rev;

		/*
		 * Look for any and all PHYs.  Valid addresses are 0..31.
		 */
		if (argc >= 3) {
			start = addrlo; end = addrhi;
		} else {
			start = 0; end = 31;
		}

		for (j = start; j <= end; j++) {
			if (miiphy_info (devname, j, &oui, &model, &rev) == 0) {
				printf("PHY 0x%02X: "
					"OUI = 0x%04X, "
					"Model = 0x%02X, "
					"Rev = 0x%02X, "
					"%3dbaseT, %s\n",
					j, oui, model, rev,
					miiphy_speed (devname, j),
					(miiphy_duplex (devname, j) == FULL)
						? "FDX" : "HDX");
			} else {
				puts ("Error reading info from the PHY\n");
			}
		}
	} else if (op[0] == 'r') {
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				data = 0xffff;
				if (miiphy_read (devname, addr, reg, &data) != 0) {
					printf(
					"Error reading from the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				} else {
					if ((addrlo != addrhi) || (reglo != reghi))
						printf("addr=%02x reg=%02x data=",
							(uint)addr, (uint)reg);
					printf("%04X\n", data & 0x0000FFFF);
				}
			}
			if ((addrlo != addrhi) && (reglo != reghi))
				printf("\n");
		}
	} else if (op[0] == 'w') {
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				if (miiphy_write (devname, addr, reg, data) != 0) {
					printf("Error writing to the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				}
			}
		}
	} else if (strncmp(op, "du", 2) == 0) {
		ushort regs[6];
		int ok = 1;
		if ((reglo > 5) || (reghi > 5)) {
			printf(
				"The MII dump command only formats the "
				"standard MII registers, 0-5.\n");
			return 1;
		}
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg < reghi + 1; reg++) {
				if (miiphy_read(devname, addr, reg, &regs[reg]) != 0) {
					ok = 0;
					printf(
					"Error reading from the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				}
			}
			if (ok)
				MII_dump_0_to_5(regs, reglo, reghi);
			printf("\n");
		}
	} else if (strncmp(op, "de", 2) == 0) {
		if (argc == 2)
			miiphy_listdev ();
		else
			miiphy_set_current_dev (argv[2]);
	} else {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op[0] = op[0];
	last_op[1] = op[1];
	last_addr_lo = addrlo;
	last_addr_hi = addrhi;
	last_reg_lo  = reglo;
	last_reg_hi  = reghi;
	last_data    = data;

	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	mii,	5,	1,	do_mii,
	"mii     - MII utility commands\n",
	"device                     - list available devices\n"
	"mii device <devname>           - set current device\n"
	"mii info   <addr>              - display MII PHY info\n"
	"mii read   <addr> <reg>        - read  MII PHY <addr> register <reg>\n"
	"mii write  <addr> <reg> <data> - write MII PHY <addr> register <reg>\n"
	"mii dump   <addr> <reg>        - pretty-print <addr> <reg> (0-5 only)\n"
	"Addr and/or reg may be ranges, e.g. 2-7.\n"
);

#endif /* CONFIG_TERSE_MII */

#ifdef BOARDCAL
/**********************************************************************************
** do_mac_setting
**
** This is the executable portion of the progmac command.  This will process the
** MAC address strings, and program them into the appropriate flash sector..
**
*/

int do_mac (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    char    sectorBuff[CFG_FLASH_SECTOR_SIZE];
    int     serno;

    /*
    ** Argv[1] contains the value string.  Convert to binary, and program the
    ** values in flash
    */
    
    serno = simple_strtoul(argv[1],0,10);
    
    /*
    ** If the serial number is less than 0, or greater than 0x1fff, it's out of range
    */
    
    if(serno < 0 || serno > 0x1fff)
    {
        printf("Serno out of range\n",serno);
        return 1;
    }
    
    /*
    ** Create the 24 bit number that composes the lower 3 bytes of the MAC address
    */
    
    serno = 0xFFFFFF & ( (ATHEROS_PRODUCT_ID << 13) | (serno & 0x1fff));
    
    /*
    ** Get the values from flash, and program into the MAC address registers
    */
    
    memcpy(sectorBuff,(void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
    
    /*
    ** Set the first and second values
    */
    
    sectorBuff[0] = 0x00;
    sectorBuff[1] = 0x03;
    sectorBuff[2] = 0x7f;

    sectorBuff[3] = 0xFF & (serno >> 16);
    sectorBuff[4] = 0xFF & (serno >> 8);
    sectorBuff[5] = 0xFF &  serno;
    
    /*
    ** Increment by 1 for the second MAC address
    */

    serno++;    
    memcpy(&sectorBuff[6],&sectorBuff[0],3);
    sectorBuff[9]  = 0xFF & (serno >> 16);
    sectorBuff[10] = 0xFF & (serno >> 8);
    sectorBuff[11] = 0xFF &  serno;
    
    flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
    write_buff(flash_info,sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);
    
    return 0;
}

U_BOOT_CMD(
    progmac, 2, 0, do_mac,
    "progmac - Set ethernet MAC addresses\n",
    "progmac <serno> - Program the MAC addresses\n"
    "                <serno> is the value of the last\n"
    "                4 digits (decimal) of the serial number\n"
);

void get_board_data(int offset, int len, u8* buf)
{
	memcpy(buf, (void *)(BOARDCAL + offset), len);
}

/*function set_board_data()
 *description:
 *write data to the flash.
 * return value: 0 (success), 1 (fail)
 */
int set_board_data(int offset, int len, u8 *buf)
{
	char sectorBuff[CFG_FLASH_SECTOR_SIZE];

	memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
	memcpy(sectorBuff + offset, buf, len);
	flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
	write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	return 0;
}

#if defined(LAN_MAC_OFFSET) && defined(WAN_MAC_OFFSET) && defined(WLAN_MAC_OFFSET) && (defined(CONFIG_WNDR3700U) || defined(CONFIG_WNDR3700V1H2))
int do_wndr3700_macset(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    char    sectorBuff[CFG_FLASH_SECTOR_SIZE];
    char    mac[6] = {255, 255, 255, 255, 255, 255}; // 255*6 = 1530
    int     mac_offset, i=0, j=0, val=0, sum=0;

    if(3 != argc)
        goto error;

    if(0 == strcmp(argv[1],"lan"))
        mac_offset = LAN_MAC_OFFSET;
    else if(0 == strcmp(argv[1],"wan"))
        mac_offset = WAN_MAC_OFFSET;
    else if(0 == strcmp(argv[1],"wlan5g"))
        mac_offset = WLAN_MAC_OFFSET;
    else
    {
        printf("unknown interface: %s\n",argv[1]);
        return 1;
    }

    while(argv[2][i])
    {
        if(':' == argv[2][i])
        {
            mac[j++] = val;
            i++;
            sum += val;
            val = 0;
            continue;
        }
        if((argv[2][i] >= '0') && (argv[2][i] <= '9'))
            val = val*16 + (argv[2][i] - '0');
        else if((argv[2][i] >='a') && (argv[2][i] <= 'f'))
            val = val*16 + (argv[2][i] - 'a') + 10;
        else if((argv[2][i] >= 'A') && (argv[2][i] <= 'F'))
            val = val*16 + (argv[2][i] - 'A') + 10;
        else
            goto error;
        i++;
    }
    mac[j] = val;
    sum += val;

    if(j != 5  || 0 == sum || 1530 == sum)
        goto error;

    memcpy(sectorBuff,(void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    memcpy(sectorBuff + mac_offset, mac, 6);

    flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
    write_buff(flash_info,sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    return 0;

error:
    printf("\nUBOOT-1.1.4 MACSET TOOL copyright.\n");
    printf("Usage:\n  macset lan(wan,wlan5g) address\n");
    printf("  For instance : macset lan 00:03:7F:EF:77:87\n");
    printf("  The MAC address can not be all 0x00 or all 0xFF\n");
    return 1;
}

U_BOOT_CMD(
    macset, 3, 0, do_wndr3700_macset,
    "macset - Set ethernet MAC address\n",
    "macset <interface> <address> - Program the MAC address of <interface>\n"
    "                               <interfcae> should be lan, wan or wlan5g\n"
    "                               <address> should be the format as 00:03:7F:EF:77:87\n"
);

int do_wndr3700_macshow(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    unsigned char mac[CFG_FLASH_SECTOR_SIZE];

    memcpy(mac, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
    printf("lan mac: %02x:%02x:%02x:%02x:%02x:%02x\n",mac[LAN_MAC_OFFSET],mac[LAN_MAC_OFFSET+1],mac[LAN_MAC_OFFSET+2],mac[LAN_MAC_OFFSET+3],mac[LAN_MAC_OFFSET+4],mac[LAN_MAC_OFFSET+5]);
    printf("wan mac: %02x:%02x:%02x:%02x:%02x:%02x\n",mac[WAN_MAC_OFFSET],mac[WAN_MAC_OFFSET+1],mac[WAN_MAC_OFFSET+2],mac[WAN_MAC_OFFSET+3],mac[WAN_MAC_OFFSET+4],mac[WAN_MAC_OFFSET+5]);
    printf("wlan5g mac: %02x:%02x:%02x:%02x:%02x:%02x\n",mac[WLAN_MAC_OFFSET],mac[WLAN_MAC_OFFSET+1],mac[WLAN_MAC_OFFSET+2],mac[WLAN_MAC_OFFSET+3],mac[WLAN_MAC_OFFSET+4],mac[WLAN_MAC_OFFSET+5]);

    return 0;
}

U_BOOT_CMD(
    macshow, 1, 0, do_wndr3700_macshow,
    "macshow - Show ethernet MAC addresses\n",
    "macshow - Display all the ethernet MAC addresses\n"
    "          for instance: the MAC of lan and wan\n"
);
#endif

#if defined(LAN_MAC_OFFSET) && defined(WAN_MAC_OFFSET) && !defined(CONFIG_WNDR3700U) && !defined(CONFIG_WNDR3700V1H2)
int do_macset(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    char    sectorBuff[CFG_FLASH_SECTOR_SIZE];
    char    mac[6] = {255, 255, 255, 255, 255, 255}; // 255*6 = 1530
    int     mac_offset, i=0, j=0, val=0, sum=0;

    if(3 != argc)
        goto error;

    if(0 == strcmp(argv[1],"lan"))
        mac_offset = LAN_MAC_OFFSET;
    else if(0 == strcmp(argv[1],"wan"))
        mac_offset = WAN_MAC_OFFSET;
    else
    {
        printf("unknown interface: %s\n",argv[1]);
        return 1;
    }

    while(argv[2][i])
    {
        if(':' == argv[2][i])
        {
            mac[j++] = val;
            i++;
            sum += val;
            val = 0;
            continue;
        }
        if((argv[2][i] >= '0') && (argv[2][i] <= '9'))
            val = val*16 + (argv[2][i] - '0');
        else if((argv[2][i] >='a') && (argv[2][i] <= 'f'))
            val = val*16 + (argv[2][i] - 'a') + 10;
        else if((argv[2][i] >= 'A') && (argv[2][i] <= 'F'))
            val = val*16 + (argv[2][i] - 'A') + 10;
        else
            goto error;
        i++;
    }
    mac[j] = val;
    sum += val;

    if(j != 5  || 0 == sum || 1530 == sum)
        goto error;

    memcpy(sectorBuff,(void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    memcpy(sectorBuff + mac_offset, mac, 6);

    flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
    write_buff(flash_info,sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

    return 0;

error:
    printf("\nUBOOT-1.1.4 MACSET TOOL copyright.\n");
    printf("Usage:\n  macset lan(wan) address\n");
    printf("  For instance : macset lan 00:03:7F:EF:77:87\n");
    printf("  The MAC address can not be all 0x00 or all 0xFF\n");
    return 1;
}

U_BOOT_CMD(
    macset, 3, 0, do_macset,
    "macset - Set ethernet MAC address\n",
    "macset <interface> <address> - Program the MAC address of <interface>\n"
    "                               <interfcae> should be lan or wan\n"
    "                               <address> should be the format as 00:03:7F:EF:77:87\n"
);

int do_macshow(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned char mac[CFG_FLASH_SECTOR_SIZE];

    memcpy(mac, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
    printf("lan mac: %02x:%02x:%02x:%02x:%02x:%02x\n",mac[LAN_MAC_OFFSET],mac[LAN_MAC_OFFSET+1],mac[LAN_MAC_OFFSET+2],mac[LAN_MAC_OFFSET+3],mac[LAN_MAC_OFFSET+4],mac[LAN_MAC_OFFSET+5]);
    printf("wan mac: %02x:%02x:%02x:%02x:%02x:%02x\n",mac[WAN_MAC_OFFSET],mac[WAN_MAC_OFFSET+1],mac[WAN_MAC_OFFSET+2],mac[WAN_MAC_OFFSET+3],mac[WAN_MAC_OFFSET+4],mac[WAN_MAC_OFFSET+5]);
    return 0; 
}

U_BOOT_CMD(
    macshow, 1, 0, do_macshow,
    "macshow - Show ethernet MAC addresses\n",
    "macshow - Display all the ethernet MAC addresses\n"
    "          for instance: the MAC of lan and wan\n"
);
#endif

#if defined(WPSPIN_OFFSET) && defined(WPSPIN_LENGTH)
int do_wpspinset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char sectorBuff[CFG_FLASH_SECTOR_SIZE];
	char wpspin[WPSPIN_LENGTH] = {0};

	if (2 != argc) {
		printf("%s\n", cmdtp->usage);
		return 1;
	}

	strncpy(wpspin, argv[1], WPSPIN_LENGTH);
	memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
	memcpy(sectorBuff + WPSPIN_OFFSET, wpspin, WPSPIN_LENGTH);

	printf("Burn wpspin into ART block.\n");
	flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
	write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	puts ("done\n");
	return 0;
}

U_BOOT_CMD(
	wpspinset, 2, 0, do_wpspinset,
	"wpspinset - set wpspin number\n",
	"Usage:\n wpspinset number\n"
	" For instance: wpspinset 12345678\n"
);

#endif

#if defined(SERIAL_NUMBER_OFFSET) && defined(SERIAL_NUMBER_LENGTH)
/*function do_snset()
 *description:
 *write the Serial Number to the flash.
 * return value:
 * 0:success
 * 1:fail
 */
int do_snset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char sectorBuff[CFG_FLASH_SECTOR_SIZE];
	char sn[SERIAL_NUMBER_LENGTH] = {0};
	int sn_len = 0, i = 0;

	if (2 != argc) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}

	sn_len = strlen(argv[1]);   /*check the SN's length*/
	if (sn_len != SERIAL_NUMBER_LENGTH) {
		printf ("SN's len is wrong,it's lenth is %d\n ", SERIAL_NUMBER_LENGTH);
		return 1;
	}

	strncpy(sn, argv[1], SERIAL_NUMBER_LENGTH);
	for (i=0; i<SERIAL_NUMBER_LENGTH; ++i)/*check seria naumber is 0~9 or A~Z*/
	{
		if (!(((sn[i]>=0x30) && (sn[i]<=0x39)) || ((sn[i]>=0x41) && (sn[i]<=0x5a))))    /*sn is 0~9 or A~Z*/
		{
			puts ("the SN only is 0~9 or A~Z\n");
			break;
		}
	}

	if (i < SERIAL_NUMBER_LENGTH)       /*because SN is not 0~9 or A~Z*/
		return 1;

	memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
	memcpy(sectorBuff + SERIAL_NUMBER_OFFSET, sn, SERIAL_NUMBER_LENGTH);

	puts("Burn SN into ART block.\n");
	flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
	write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	puts("Done.\n");
	return 0;
}

U_BOOT_CMD(
	snset, 2, 0, do_snset,
	"snset   - set serial number\n",
	"Usage:\n snset number (13 digit)\n"
	" For instance: snset 1ML1747D0000B\n"
);

int do_snshow(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 sn[SERIAL_NUMBER_LENGTH + 1];
	memset(sn, 0, sizeof(sn));
	get_board_data(SERIAL_NUMBER_OFFSET, SERIAL_NUMBER_LENGTH, sn);
	printf("serial number : %s\n", sn);
	return 0;
}

U_BOOT_CMD(
	snshow, 1, 0, do_snshow,
	"snshow  - show serial number\n",
	"\n  show device board serial number stored in ART area\n"
);

#endif

#if defined(REGION_NUMBER_OFFSET) && defined(REGION_NUMBER_LENGTH)
/*function set_region()
 *description:
 *write the Region Number to the flash.
 * return value:
 * 0:success
 * 1:fail
 */
int set_region(u16 host_region_number)
{
	char sectorBuff[CFG_FLASH_SECTOR_SIZE];

	/* Always save region number as network order */
	u16 region_number = htons(host_region_number);

	/* Region Number range: 0x0001 ~ 0x0009 */
	if (!((region_number >= 0x0001) && (region_number <= 0x0009))) {
		printf("Wrong Region Number\n");
		return 1;
	}
	memcpy(sectorBuff, (void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);
	memcpy(sectorBuff + REGION_NUMBER_OFFSET, &region_number, REGION_NUMBER_LENGTH);

	puts("Burn Region Number into ART block.\n");
	flash_erase(flash_info, CAL_SECTOR, CAL_SECTOR);
	write_buff(flash_info, sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	puts("Done.\n");
	return 0;
}

/*function do_rnset()
 *description:
 * read command input and translate to u16,
 * then call set_region() to write to flash.
 * return value:
 * 0:success
 * 1:fail
 */
int do_rnset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char rn[REGION_NUMBER_LENGTH] = {0};
	unsigned short region = 0;

	int rn_len = 0, i = 0;

	if (2 != argc) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}
	rn_len = strlen(argv[1]);   /*check the RN's length*/
	if (rn_len != REGION_NUMBER_LENGTH) {
		printf ("Region Number's len is wrong,it's lenth is %d\n ",\
			REGION_NUMBER_LENGTH);
		return 1;
	}

	strncpy(rn, argv[1], REGION_NUMBER_LENGTH);
	for (i=0; i<REGION_NUMBER_LENGTH; ++i)/*check region number is 0~9*/
	{
		if (!((rn[i]>=0x30) && (rn[i]<=0x39)))    /*rn is 01~09*/
		{
			puts ("Input 01 ~ 09 for REGION Number\n");
			break;
		}
	}
	if (i < REGION_NUMBER_LENGTH)       /*because RN is not 0~9*/
		return 1;

	if ((rn[0] != '0') || (rn[1] == '0')) {
		puts ("REGION Number is 01~09\n");
		return 1;
	}


	for (i = 0; i < REGION_NUMBER_LENGTH; ++i)/*translate "01" to 0x0001 */
		region = 10*region + (rn[i] - 0x30);

	printf("write 0x%04x to board region\n", region);

	return set_region(region);

}

U_BOOT_CMD(
	rnset, 2, 0, do_rnset,
	"rnset   - set region number\n",
	"Usage:\n rnset number (2 digit)\n"
	" For instance: rnset 01\n"
);

int do_rnshow(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u16 rn;

	memcpy(&rn, (void *)(BOARDCAL + REGION_NUMBER_OFFSET), sizeof(rn));
	printf("region on board: 0x%04x\n", rn);

	return 0;

}

U_BOOT_CMD(
	rnshow, 1, 0, do_rnshow,
	"rnshow \t- Show Region Number on Board\n",
	"Usage:\n rnshow\n"
	" For instance: rnshow\n"
);
#endif

#if defined(BOARD_HW_ID_OFFSET) && defined(BOARD_HW_ID_LENGTH) && defined(BOARD_HW_ID_DEFAULT)
/*function do_board_hw_id_set()
 *description:
 * read in board_hw_id, then call set_board_data() to write to flash.
 * return value: 0 (success), 1 (fail)
 */
int do_board_hw_id_set(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_hw_id[BOARD_HW_ID_LENGTH + 1];
	int board_hw_id_len = 0;

	if (argc != 2) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}
	if ((board_hw_id_len = strlen(argv[1])) > BOARD_HW_ID_LENGTH) {
		printf ("the length of BOARD_HW_ID can't > %d\n", BOARD_HW_ID_LENGTH);
		return 1;
	}

	memset(board_hw_id, 0, sizeof(board_hw_id));
	memcpy(board_hw_id, argv[1], board_hw_id_len);

	printf("Burn board_hw_id (= %s) into ART block\n", board_hw_id);
	set_board_data(BOARD_HW_ID_OFFSET, BOARD_HW_ID_LENGTH, board_hw_id);
	puts("Done.\n");
	return 0;
}

U_BOOT_CMD(
	board_hw_id_set, 2, 0, do_board_hw_id_set,
	"board_hw_id_set \t- set board_hw_id\n",
	"Usage:\n"
	" board_hw_id_set XXXXXX\n"
	" For instance: board_hw_id_set " BOARD_HW_ID_DEFAULT "\n"
);

void get_board_hw_id(u8* buf) /* sizeof(buf) must > BOARD_HW_ID_LENGTH */
{
	get_board_data(BOARD_HW_ID_OFFSET, BOARD_HW_ID_LENGTH, buf);
}

int do_board_hw_id_show(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_hw_id[BOARD_HW_ID_LENGTH + 1];

	memset(board_hw_id, 0, sizeof(board_hw_id));
	get_board_hw_id(board_hw_id);
	printf("board_hw_id : %s\n", board_hw_id);
	return 0;
}

U_BOOT_CMD(
	board_hw_id_show, 1, 0, do_board_hw_id_show,
	"board_hw_id_show \t- Show board_hw_id\n",
	"Usage:\n"
	" board_hw_id_show\n"
	" For instance: board_hw_id_show\n"
);

#if defined(BOARD_MODEL_ID_OFFSET) && defined(BOARD_MODEL_ID_LENGTH) && defined(BOARD_MODEL_ID_DEFAULT)
/*function do_board_model_id_set()
 *description:
 * read in board_model_id, then call set_board_data() to write to flash.
 * return value: 0 (success), 1 (fail)
 */
int do_board_model_id_set(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_model_id[BOARD_MODEL_ID_LENGTH + 1];
	int board_model_id_len = 0;

	if (argc != 2) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}
	if ((board_model_id_len = strlen(argv[1])) > BOARD_MODEL_ID_LENGTH) {
		printf ("the length of BOARD_MODEL_ID can't > %d\n", BOARD_MODEL_ID_LENGTH);
		return 1;
	}

	memset(board_model_id, 0, sizeof(board_model_id));
	memcpy(board_model_id, argv[1], board_model_id_len);

	printf("Burn board_model_id (= %s) into ART block\n", board_model_id);
	set_board_data(BOARD_MODEL_ID_OFFSET, BOARD_MODEL_ID_LENGTH, board_model_id);
	puts("Done.\n");
	return 0;
}

U_BOOT_CMD(
	board_model_id_set, 2, 0, do_board_model_id_set,
	"board_model_id_set \t- set board_model_id\n",
	"Usage:\n"
	" board_model_id_set XXXXXX\n"
	" For instance: board_model_id_set " BOARD_MODEL_ID_DEFAULT "\n"
);

void get_board_model_id(u8* buf) /* sizeof(buf) must > BOARD_MODEL_ID_LENGTH */
{
	get_board_data(BOARD_MODEL_ID_OFFSET, BOARD_MODEL_ID_LENGTH, buf);
}

int do_board_model_id_show(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_model_id[BOARD_MODEL_ID_LENGTH + 1];

	memset(board_model_id, 0, sizeof(board_model_id));
	get_board_model_id(board_model_id);
	printf("board_model_id : %s\n", board_model_id);
	return 0;
}

U_BOOT_CMD(
	board_model_id_show, 1, 0, do_board_model_id_show,
	"board_model_id_show \t- Show board_model_id\n",
	"Usage:\n"
	" board_model_id_show\n"
	" For instance: board_model_id_show\n"
);
#endif	/* MODEL_ID */
#endif	/* HW_ID */

#if defined(BOARD_SSID_OFFSET) && defined(BOARD_SSID_LENGTH)
/*function do_board_ssid_set()
 *description:
 * read in ssid, then call set_board_data() to write to flash.
 * return value: 0 (success), 1 (fail)
 */
int do_board_ssid_set(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_ssid[BOARD_SSID_LENGTH + 1];
	int board_ssid_len = 0;

	if (argc != 2) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}
	if ((board_ssid_len = strlen(argv[1])) > BOARD_SSID_LENGTH) {
		printf ("the length of SSID can't > %d\n", BOARD_SSID_LENGTH);
		return 1;
	}

	memset(board_ssid, 0, sizeof(board_ssid));
	memcpy(board_ssid, argv[1], board_ssid_len);

	printf("Burn SSID (= %s) into ART block\n", board_ssid);
	set_board_data(BOARD_SSID_OFFSET, BOARD_SSID_LENGTH, board_ssid);
	puts("Done.\n");
	return 0;
}

U_BOOT_CMD(
	board_ssid_set, 2, 0, do_board_ssid_set,
	"board_ssid_set \t- set ssid on board\n",
	"Usage:\n"
	" board_ssid_set XXXXXX\n"
	" For instance: board_ssid_set NETGEAR \n"
);

void get_board_ssid(u8* buf) /* sizeof(buf) must > BOARD_SSID_LENGTH */
{
	get_board_data(BOARD_SSID_OFFSET, BOARD_SSID_LENGTH, buf);
}

int do_board_ssid_show(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_ssid[BOARD_SSID_LENGTH + 1];

	memset(board_ssid, 0, sizeof(board_ssid));
	get_board_ssid(board_ssid);
	printf("board_ssid : %s\n", board_ssid);
	return 0;
}

U_BOOT_CMD(
	board_ssid_show, 1, 0, do_board_ssid_show,
	"board_ssid_show \t- Show board_ssid\n",
	"Usage:\n"
	" board_ssid_show\n"
	" For instance: board_ssid_show\n"
);

#endif	/* BOARD_SSID */

#if defined(BOARD_PASSPHRASE_OFFSET) && defined(BOARD_PASSPHRASE_LENGTH)
/*function do_board_passphrase_set()
 *description:
 * read in passphrase, then call set_board_data() to write to flash.
 * return value: 0 (success), 1 (fail)
 */
int do_board_passphrase_set(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_passphrase[BOARD_PASSPHRASE_LENGTH + 1];
	int board_passphrase_len = 0;

	if (argc != 2) {
		printf("%s\n",cmdtp->usage);
		return 1;
	}
	if ((board_passphrase_len = strlen(argv[1])) > BOARD_PASSPHRASE_LENGTH) {
		printf ("the length of PASSPHRASE can't > %d\n", BOARD_PASSPHRASE_LENGTH);
		return 1;
	}

	memset(board_passphrase, 0, sizeof(board_passphrase));
	memcpy(board_passphrase, argv[1], board_passphrase_len);

	printf("Burn PASSPHRASE (= %s) into ART block\n", board_passphrase);
	set_board_data(BOARD_PASSPHRASE_OFFSET, BOARD_PASSPHRASE_LENGTH, board_passphrase);
	puts("Done.\n");
	return 0;
}

U_BOOT_CMD(
	board_passphrase_set, 2, 0, do_board_passphrase_set,
	"board_passphrase_set \t- set passphrase on board\n",
	"Usage:\n"
	" board_passphrase_set XXXXXX\n"
	" For instance: board_passphrase_set 1234567890 \n"
);

void get_board_passphrase(u8* buf) /* sizeof(buf) must > BOARD_PASSPHRASE_LENGTH */
{
	get_board_data(BOARD_PASSPHRASE_OFFSET, BOARD_PASSPHRASE_LENGTH, buf);
}

int do_board_passphrase_show(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u8 board_passphrase[BOARD_PASSPHRASE_LENGTH + 1];

	memset(board_passphrase, 0, sizeof(board_passphrase));
	get_board_passphrase(board_passphrase);
	printf("board_passphrase : %s\n", board_passphrase);
	return 0;
}

U_BOOT_CMD(
	board_passphrase_show, 1, 0, do_board_passphrase_show,
	"board_passphrase_show \t- Show board_passphrase\n",
	"Usage:\n"
	" board_passphrase_show\n"
	" For instance: board_passphrase_show\n"
);
#endif	/* BOARD_PASSPHRASE */

#if defined(WLAN_MAC_OFFSET) && !defined(CONFIG_WNDR3700U) && !defined(CONFIG_WNDR3700V1H2)
/*******************************************************************************
** function do_wmacset()
** description:
** set wlan mac address into the flash.
** return value:
** 0:success
** 1:fail
*/
int do_wmacset(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char    sectorBuff[CFG_FLASH_SECTOR_SIZE];
	char    mac[6] = {255, 255, 255, 255, 255, 255}; // 255*6 = 1530
	int     wmac_offset, i=0, j=0, val=0, sum=0;

	if(2 != argc)
		goto error;

	wmac_offset = WLAN_MAC_OFFSET;

	while(argv[1][i])
	{
		if(':' == argv[1][i])
		{
			mac[j++] = val;
			i++;
			sum += val;
			val = 0;
			continue;
		}
		if((argv[1][i] >= '0') && (argv[1][i] <= '9'))
			val = val*16 + (argv[1][i] - '0');
		else if((argv[1][i] >='a') && (argv[1][i] <= 'f'))
			val = val*16 + (argv[1][i] - 'a') + 10;
		else if((argv[1][i] >= 'A') && (argv[1][i] <= 'F'))
			val = val*16 + (argv[1][i] - 'A') + 10;
		else
			goto error;
		i++;
	}
	mac[j] = val;
	sum += val;

	if(j != 5  || 0 == sum || 1530 == sum)
		goto error;

	memcpy(sectorBuff,(void *)BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	memcpy(sectorBuff + wmac_offset, mac, 6);

	flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
	write_buff(flash_info,sectorBuff, BOARDCAL, CFG_FLASH_SECTOR_SIZE);

	return 0;

error:
	return 1;
}

U_BOOT_CMD(
	wmacset, 2, 0, do_wmacset,
	"wmacset - Set wlan MAC address\n",
	"wmacset <address> - Program the MAC address of <interface>\n"
	"                    <address> should be the format as 00:03:7F:EF:77:87\n"
);
#endif

#endif /* BOARDCAL */

#endif /* CFG_CMD_MII */
#endif /* #ifndef COMPRESSED_UBOOT */
