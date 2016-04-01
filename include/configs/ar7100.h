/*
 * This file contains the configuration 
 * parameters for the ar724x SoC.
 */

#ifndef __AR7100_H
#define __AR7100_H

/* 
 ************************
 *  General parameters  *
 ************************
 */
#define CONFIG_MIPS32		1	/* MIPS32 CPU core	*/

#define CONFIG_BAUDRATE		115200 
#define CFG_BAUDRATE_TABLE	{ 115200 }

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

/*
 ****************************************
 *  Miscellaneous configurable options  *
 ****************************************
 */
#define CFG_BOOTM_LEN		(16 << 20) /* 16 MB */

#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE		(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_SDRAM_BASE		0x80000000	/* Cached addr */
/* #define CFG_SDRAM_BASE	0xa0000000 */	/* Cached addr */

#define	CFG_LOAD_ADDR		0x81000000	/* default load address	*/
/* #define CFG_LOAD_ADDR	0xa1000000 */	/* default load address	*/

#define CFG_MEMTEST_START       0x80200000
#define CFG_MEMTEST_END		0x83800000

/*
 ******************************************************
 *  PLL Config for different CPU/DDR/AHB frequencies  *
 ******************************************************
*/
#define CFG_PLL_200_200_100	0
#define CFG_PLL_300_300_150	1
#define CFG_PLL_333_333_166	2
#define CFG_PLL_266_266_133	3
#define CFG_PLL_266_266_66	4
#define CFG_PLL_400_400_200	5
#define CFG_PLL_360_360_180	6
#define CFG_PLL_400_400_100	7
#define CFG_PLL_680_340_170	8

/*
 *************************
 *  Cache Configuration  *
 *************************
 */
#define CFG_DCACHE_SIZE		32768
#define CFG_ICACHE_SIZE		65536
#define CFG_CACHELINE_SIZE	32

#endif	/* __CONFIG_H */
