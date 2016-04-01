/*
 * uncompress.c
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 2000-2002 Transmeta Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * shamelessly stolen from fs/cramfs
 */

#include <common.h>
#include <malloc.h>
#include <watchdog.h>

#if (CONFIG_FS & CFG_FS_SQUASHFS)

#ifdef CONFIG_SQUASHFS_LZMA
#include "LzmaDecode.h"
//#include "LzmaDecode.c"
static CLzmaDecoderState state;
#else
#include <zlib.h>
static z_stream stream;
#endif

#define ZALLOC_ALIGNMENT	16

#ifndef CONFIG_SQUASHFS_LZMA
static void *zalloc (void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

static void zfree (void *x, void *addr, unsigned nb)
{
	free (addr);
}
#endif

int squashfs_uncompress_block (void *dst, int dstlen, void *src, int srclen)
{
	int err;
#ifdef CONFIG_SQUASHFS_LZMA
		SizeT InProcessed;
		int bytes;
		if((err = LzmaDecode(&state,
				src, srclen, &InProcessed,
				dst, dstlen, &bytes)) != LZMA_RESULT_OK) {
			printf("lzma_fs returned unexpected result 0x%x\n", err);
			return 0;
		}
		return bytes;
#else

	inflateReset (&stream);

	stream.next_in = src;
	stream.avail_in = srclen;

	stream.next_out = dst;
	stream.avail_out = dstlen;

	err = inflate (&stream, Z_FINISH);
	if ((err==Z_OK)||(err==Z_STREAM_END))
		return dstlen-stream.avail_out;
	else
		return 0;
#endif
}

int squashfs_uncompress_init (void)
{
#ifdef CONFIG_SQUASHFS_LZMA
	state.Properties.lc = 3/*CONFIG_SQUASHFS_LZMA_LC*/;
	state.Properties.lp = 0/*CONFIG_SQUASHFS_LZMA_LP*/;
	state.Properties.pb = 2/*CONFIG_SQUASHFS_LZMA_PB*/;
	if(!(state.Probs = (CProb *) malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb)))) {
		printf("Failed to allocate lzma workspace\n");
		return -1;
	}
#else
	int err;

	stream.zalloc = zalloc;
	stream.zfree = zfree;
	stream.next_in = 0;
	stream.avail_in = 0;

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	stream.outcb = (cb_func) WATCHDOG_RESET;
#else
	stream.outcb = Z_NULL;
#endif /* CONFIG_HW_WATCHDOG */

	err = inflateInit (&stream);
	if (err != Z_OK) {
		printf ("Error: inflateInit() returned %d\n", err);
		return -1;
	}
#endif
	return 0;
}

int squashfs_uncompress_exit (void)
{
#ifdef CONFIG_SQUASHFS_LZMA
	free(state.Probs);
#else
	inflateEnd (&stream);
#endif
	return 0;
}

#endif /* CFG_FS_SQUASHFS */
