/*
 * Squashfs - a compressed read only filesystem for Linux
 *
 * Copyright (c) 2002 Phillip Lougher <phillip@lougher.demon.co.uk>
 *
 * U-boot adaptation:
 * Copyright (c) 2004 Carsten Juttner <carjay@gmx.net>
 *
 * Modified for Squashfs2:
 * Copyright (c) 2004 Marcel Pommer <marsellus@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 */

/*
 * 2004-10-17:
 *
 * this code does not yet support fragments. so be sure to have
 * the kernel in a squashfs built without -always-use-fragments.
 * your kernel will probably not be < 64k in size ;-)
 *
 * squashfs version 1.x is not supported anymore, compatibility
 * code may be added.
 *
 *
 * 2004-10-22:
 *
 * fragments are now supported, so -always-use-fragments works.
 *
 * compatibility with squashfs 1.x has been declared nonsense.
 *
 */

#include <common.h>
#include <config.h>

#if (CONFIG_FS & CFG_FS_SQUASHFS)

#include <linux/types.h>
#include <squashfs/squashfs_fs.h>
#include <squashfs/global.h>
#include <zlib.h>
#include <malloc.h>
#include <jffs2/load_kernel.h>

//#define SQUASHFS_TRACE
#ifdef SQUASHFS_TRACE
#define TRACE(s, args...)				printf("SQUASHFS: "s, ## args)
#else
#define TRACE(s, args...)				{}
#endif

#define ERROR(s, args...)				printf("SQUASHFS error: "s, ## args)

#define SERROR(s, args...)				if(!silent) printf("SQUASHFS error: "s, ## args)
#define WARNING(s, args...)				printf("SQUASHFS: "s, ## args)

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];
#define PART_OFFSET(x)  (x->offset + flash_info[x->dev->id->num].start[0])

int read_fragment_table(struct part_info *info, squashfs_super_block *sBlk, squashfs_fragment_entry **fragment_table);
unsigned int uncompr_size;

int squashfs_read_super (struct part_info *info, squashfs_super_block *super, squashfs_fragment_entry **frag_tbl)
{
	if (info->size<sizeof(squashfs_super_block))
		return 0;

	/* superblock offset is actually index SQUASHFS_START which is defined as 0 */
	memcpy (super,(unsigned char*)PART_OFFSET(info),sizeof(squashfs_super_block));

	if (super->s_magic==SQUASHFS_MAGIC_SWAP)
	{
		ERROR ("non-native-endian squashfs is not supported\n");
		return 0;
	}
	
	if (super->s_magic!=SQUASHFS_MAGIC)
	{
		ERROR ("1 no squashfs_magic: %08x\n",super->s_magic);
		return 0;
	}
	
	if ((super->s_major!=SQUASHFS_MAJOR)||
		(super->s_minor!=SQUASHFS_MINOR))
	{
		ERROR ("unsupported squashfs version %d.%d\n",super->s_major,super->s_minor);
		return 0;
	}

	TRACE ("fragments: %d\n", super->fragments);
	TRACE ("fragment_table_start: 0x%x\n", super->fragment_table_start);
	TRACE ("inode_table_start: 0x%x\n", super->inode_table_start);
	TRACE ("directory_table_start: 0x%x\n", super->directory_table_start);

	if (frag_tbl && (!read_fragment_table(info, super, frag_tbl)))
	{
		ERROR ("error reading fragment table\n");
		return 0;
	}

	return 1;
}


/* read raw bytes from partition */
static void read_bytes(struct part_info *info, unsigned int offset, int size, char *buffer)
{
	TRACE("read_bytes @ 0x%x, size %d, buffer 0x%x\n", offset, size, buffer);
	/* grab them directly off the Flash */
	char *src = (char *)PART_OFFSET(info)+offset;
	memcpy (buffer,src,size);
}


/* read (un)compressed block / fragment of a block */
static int read_block(
	struct part_info *info,
	unsigned int start,		/* start address of block */
	unsigned int *next,		/* start + <number of read bytes> is put here, unless NULL is passed */
	unsigned char *block,		/* destination buffer */
	squashfs_super_block *sBlk,
	unsigned int *bytecount,	/* if this is a data block, pass the c_byte here. otherwise you must pass NULL */
	unsigned int frag_offset,	/* offset into the block if this is a fragment */
	unsigned int frag_size		/* fragment size, if this is no fragment, you must pass 0 here */
	)
{
	unsigned short int c_byte;
	int offset;
	unsigned char check_data;
	short int compressed;

	/* if this is a meta data block, the size is in the block's first 2 bytes */
	if (!bytecount)
	{
		read_bytes(info, start, 2, (char *)&c_byte);
		compressed = SQUASHFS_COMPRESSED(c_byte);
		uncompr_size = SQUASHFS_COMPRESSED_SIZE(c_byte);
		offset = 2;
		/* check_data needs one byte per block */
		if(SQUASHFS_CHECK_DATA(sBlk->flags))
		{
			offset = 3;
			read_bytes(info, start+2, 1, (char *)&check_data);
			TRACE("block @ 0x%x, check data: 0x%x\n", start, check_data);
		}
	}
	/* if this is a data block, the size is passed in *bytecount */
	else
	{
		compressed = SQUASHFS_COMPRESSED_BLOCK(*bytecount);
		uncompr_size = SQUASHFS_COMPRESSED_SIZE_BLOCK(*bytecount);
		offset = 0;
	}

	/* handle compressed block */
	if(compressed) 
	{
		unsigned char buffer[sBlk->block_size];
	
		/* if this is a fragment, we need a temporary decompression buffer */
		unsigned char uncompressed_buffer[sBlk->block_size];
		int res;
		long bytes = sBlk->block_size;		

		TRACE("compressed block @ 0x%x, compressed size %d\n", start, uncompr_size);
		read_bytes(info, start + offset, uncompr_size, buffer);
		squashfs_uncompress_init();

		/* inflate the block (to temporary buffer if we only need a fragment) */
		res = squashfs_uncompress_block(frag_size ? uncompressed_buffer : block, bytes, buffer, uncompr_size);

		TRACE("compressed block @ 0x%x, uncompressed size %d\n", start, res);
		squashfs_uncompress_exit();
		
		if(!res)
		    return 0;

		/* if this is a fragment, copy only that part of the block */
		if (frag_size)
		{
			memcpy(block, uncompressed_buffer + frag_offset, frag_size);
		}
		/* advance buffer pointer if requested */
		if(next && !frag_size)
		{
			*next = start + offset + uncompr_size;
		}
	
		return frag_size ? frag_size : res;
	} 
	/* handle uncompressed block */
	else 
	{
		TRACE("uncompressed block @ 0x%x, size %d\n", start, frag_size ? frag_size : uncompr_size);
		/* just copy the block (or a fragment of it) from partition to destination buffer */
		read_bytes(info, start + offset, frag_size ? frag_size : uncompr_size, block);
		if(next && !frag_size)
		{
			*next = start + offset + uncompr_size;
		}
		return frag_size ? frag_size : uncompr_size;
	}
}


/* reads the whole fragment table if there is one */
int read_fragment_table(struct part_info *info, squashfs_super_block *sBlk, squashfs_fragment_entry **fragment_table)
{
	int i, indexes = SQUASHFS_FRAGMENT_INDEXES(sBlk->fragments);
	squashfs_fragment_index fragment_table_index[indexes];

	TRACE("reading %d fragment indexes from 0x%x\n", indexes, sBlk->fragment_table_start);
	if(sBlk->fragments == 0)
		return 1;

	*fragment_table = (squashfs_fragment_entry *)malloc(sBlk->fragments*sizeof(squashfs_fragment_entry));
	if(!*fragment_table) 
	{
		ERROR("Failed to allocate memory for fragment table\n");
		return 0;
	}

	read_bytes(info, sBlk->fragment_table_start, SQUASHFS_FRAGMENT_INDEX_BYTES(sBlk->fragments), (char *) fragment_table_index);

	for(i = 0; i < indexes; i++) 
	{
		TRACE("reading fragment table block @ 0x%x\n", fragment_table_index[i]);
		read_block(info, fragment_table_index[i], NULL, ((unsigned char *) *fragment_table) + (i * SQUASHFS_METADATA_SIZE), sBlk, NULL, 0, 0);
	}

	return 1;
}

/* reads directory header(s) and entries and looks for a given name. Returns the inode if found */
static int squashfs_readdir(struct part_info *info, squashfs_dir_inode_header *diri,
			char *filename, squashfs_inode *inode, squashfs_super_block *sBlk)
{
	squashfs_dir_header *dirh;
	char buffer[sizeof(squashfs_dir_entry) + SQUASHFS_NAME_LEN + 1];
	squashfs_dir_entry *dire = (squashfs_dir_entry *) buffer;
	int dir_count , block_size;
	unsigned int initial_offset = diri->offset;


	int bytes = 0;
	int tmpbytes = 0;

	unsigned char *dirblock = malloc (SQUASHFS_METADATA_SIZE);
	unsigned char *tmpblock = malloc (SQUASHFS_METADATA_SIZE);
	
	unsigned int start = sBlk->directory_table_start + diri->start_block;
	unsigned int dirs=0;

	if (!dirblock)
	{
		ERROR ("squashfs_readdir: out of memory allocating dirblock\n");
		return 0;
	}

	if (!tmpblock)
	{
		free(dirblock);
		ERROR ("squashfs_readdir: out of memory allocating tmpblock\n");
		return 0;
	}
	
	TRACE("initial_offset 0x%x, table_start 0x%x\n", diri->offset, sBlk->directory_table_start);
	
	block_size = read_block (info, start, NULL, dirblock, sBlk, NULL, 0, 0);
	if (!block_size)
	{
		ERROR ("squashfs_readdir: read_block\n");
		free(tmpblock);
		free(dirblock);
		return 0;
	}

	memcpy(tmpblock, dirblock + initial_offset, block_size - initial_offset);

	/* check wether all entries are in the same block */
	tmpbytes = diri->file_size - (block_size - initial_offset);
	if(tmpbytes > 0)
	{
		/* check_data needs one byte more per block */
		if(SQUASHFS_CHECK_DATA(sBlk->flags))
		{
		    block_size = read_block (info, start + uncompr_size + 3, NULL, tmpblock + block_size - initial_offset, sBlk, NULL, 0 , 0);
		}
		else
		{
		    block_size = read_block (info, start + uncompr_size + 2, NULL, tmpblock + block_size - initial_offset, sBlk, NULL, 0 , 0);
		}    
		
		if (!block_size)
		{
			ERROR ("squashfs_readdir: read_block\n");
			free(tmpblock);
			free(dirblock);
		    return 0;
		}
	}	

	while ((bytes + 3)<diri->file_size /*&& dir_count !=0*/)
	{
 		dirh = (squashfs_dir_header*)(tmpblock + bytes);
		dir_count = dirh->count + 1;
		dirs+=dir_count;
		bytes += sizeof(squashfs_dir_header);


		while(dir_count--) 
		{
			memcpy((void *)dire, tmpblock + bytes, sizeof(dire));
			bytes += sizeof(*dire);
			memcpy((void *)dire->name, tmpblock + bytes, dire->size + 1);
			dire->name[dire->size + 1] = '\0';
			if (!filename)
			{
				printf ("entry is %s\n", dire->name);
			}
			if (filename && inode && !strncmp(dire->name,filename,dire->size+1))
			{
				TRACE ("entry found: %s\n", dire->name);
				*inode = SQUASHFS_MKINODE(dirh->start_block,dire->offset);
				free(tmpblock);
				free(dirblock);
				return 1;
			}
			bytes += dire->size + 1;
		}
		TRACE("\nTRACE here!! bytes %d file_size %d\n",bytes,diri->file_size);
	}

	if (!filename)
	{
		printf ("%d directory entries\n", dirs);
	}
	free (dirblock);
	free (tmpblock);
	return 0;
}

/*
 *  looks up a directory or file and displays information or loads the file
 */
#define SQUASHFS_FLAGS_LS	1
#define SQUASHFS_FLAGS_LOAD	2

static unsigned int squashfs_lookup (struct part_info *info, char *entryname, char flags,unsigned int loadoffset, unsigned int *size){
	squashfs_super_block sBlk;

	unsigned int root_inode_start;
	unsigned int root_inode_offset;

	squashfs_fragment_entry *frag_table = NULL;
	unsigned char *blockbuffer;
	unsigned int blocksize;
	unsigned int cur_ptr;
	unsigned int cur_offset;

	char *partname;
	unsigned char parsed=0;

	squashfs_inode inode;

	if (!squashfs_read_super(info,&sBlk,&frag_table))
	{
		return 0;
	}

	/* inode-block relates to the compressed data and offset to the uncompressed result */
	root_inode_start = sBlk.inode_table_start + SQUASHFS_INODE_BLK(sBlk.root_inode);
	root_inode_offset = SQUASHFS_INODE_OFFSET(sBlk.root_inode);

	TRACE ("root_inode @ %x:%x\n",root_inode_start,root_inode_offset);

	blockbuffer = malloc (SQUASHFS_METADATA_SIZE);
	if (!blockbuffer)
	{
		ERROR ("out of memory allocating blockbuffer\n");
		if (frag_table)
			free(frag_table);
		return 0;
	}

	/* we get the root-inode block as a starting point */
	cur_ptr = root_inode_start;
	cur_offset = root_inode_offset;

	/* get first part of entryname */
	partname = strtok (entryname, "/");
	while (1)
	{
	/* now get the correct directory table entry */
  		squashfs_dir_inode_header diri;

		/* first lookup the dir-inode (starting with root) */
		blocksize = read_block(info, cur_ptr, &cur_ptr, blockbuffer, &sBlk, NULL, 0, 0);
		
		if (!blocksize)
		{
			ERROR ("reading inode block\n");
			break;
		}
		memcpy (&diri,blockbuffer+cur_offset,sizeof(squashfs_dir_inode_header));

		if (diri.inode_type != SQUASHFS_DIR_TYPE)	/* oops, we tried to dive too deep */
		{
			break;
		}

		/* second we lookup the actual entry in the directory table, if it matches we get back the inode */
		if (!squashfs_readdir (info, &diri, partname, &inode, &sBlk))
		{
			/* entry not found or root dir was requested */
			if (partname==NULL) parsed = 1;
			break;
		}
		cur_ptr = sBlk.inode_table_start + SQUASHFS_INODE_BLK(inode);
		cur_offset = SQUASHFS_INODE_OFFSET(inode);
		TRACE ("inode @ %x:%x\n",cur_ptr,cur_offset);

		if (partname)
		{
			partname = strtok (NULL,"/");
		}
		if (!partname)	/* no more to parse, either we have a dir or a file */
		{
			/* get the inode block from the inode_table */
			squashfs_base_inode_header base;
			blocksize = read_block(info, cur_ptr, &cur_ptr, blockbuffer, &sBlk, NULL, 0, 0);
			if (!blocksize)
			{
				ERROR ("reading final block\n");
				break;
			}

			/* FIXME: find out how squashfs handles this in the VFS code */
			if (blocksize == SQUASHFS_METADATA_SIZE)
			{ /* uhoh, we might have to cross a block boundary, better get the next block, too */
				if (cur_ptr < sBlk.directory_table_start) 
				{
					blockbuffer = realloc(blockbuffer, 2 * SQUASHFS_METADATA_SIZE);
					blocksize = read_block(info, cur_ptr, &cur_ptr, blockbuffer + SQUASHFS_METADATA_SIZE, &sBlk, NULL, 0, 0);
					if (!blocksize)
					{
						ERROR ("reading overlong metablock\n");
						break;
					} 
				}
			}

			/* check what kind of inode it is */
			memcpy (&base, blockbuffer + cur_offset,sizeof (base));
			if (flags&SQUASHFS_FLAGS_LS)
			{
				switch (base.inode_type)
				{
				case SQUASHFS_DIR_TYPE:
				{
					squashfs_dir_inode_header diri;
					memcpy (&diri, blockbuffer + cur_offset,sizeof (diri));
    					cur_ptr = sBlk.directory_table_start + diri.start_block;
					squashfs_readdir (info, &diri, NULL, NULL, &sBlk);
					break;
				}
				case SQUASHFS_FILE_TYPE:
					break;
				case SQUASHFS_SYMLINK_TYPE:
					/* resolve */
					break;
				case SQUASHFS_LDIR_TYPE:
					/* resolve */
					break;
				default:
					break;
				}
				parsed = 1;
				break;
			} 
			else 
			{	/* SQUASHFS_FLAGS_LOAD */
				switch (base.inode_type)
				{
				case SQUASHFS_FILE_TYPE:
				{
					int i;
					unsigned int blocks;
					int frag_bytes;
					unsigned int *blocklist;
					unsigned int bytes = 0;
					
					squashfs_reg_inode_header dirreg;
					memcpy (&dirreg,blockbuffer + cur_offset, sizeof(dirreg));
					
					blocks = dirreg.fragment == SQUASHFS_INVALID_FRAG
									? (dirreg.file_size + sBlk.block_size - 1) >> sBlk.block_log
									: dirreg.file_size >> sBlk.block_log;
					frag_bytes = dirreg.fragment == SQUASHFS_INVALID_FRAG ? 0 : dirreg.file_size % sBlk.block_size;

					TRACE("regular file, size %d, blocks %d, start_block 0x%x\n", dirreg.file_size, blocks, dirreg.start_block);
					
					blocklist=(unsigned int*)malloc(blocks*sizeof(unsigned int));
					if (!blocklist)
					{
						ERROR("out of memory allocating blocklist\n");
						free (blockbuffer);
						if (frag_table)
							free(frag_table);
						return 0;
					}
					cur_offset += sizeof(dirreg);
					memcpy (blocklist,blockbuffer+cur_offset,blocks*sizeof(unsigned int));
					cur_ptr = dirreg.start_block;
					for (i=0;i<blocks;i++)
					{
						TRACE("reading block %d\n", i);
						bytes += read_block(info, cur_ptr, &cur_ptr, (unsigned char*)(loadoffset+bytes), &sBlk, blocklist+i, 0, 0);
					}
					if (frag_table && frag_bytes)
					{
	    				squashfs_fragment_entry *frag_entry = frag_table + dirreg.fragment;
				
						TRACE("%d bytes in fragment %d, offset %d\n",
							frag_bytes, dirreg.fragment, dirreg.offset);

						TRACE("fragment %d, start_block=0x%x, size=%d\n",
							dirreg.fragment, frag_entry->start_block, SQUASHFS_COMPRESSED_SIZE_BLOCK(frag_entry->size));
						bytes += read_block(info, frag_entry->start_block, NULL, (unsigned char*)(loadoffset+bytes), &sBlk, &(frag_entry->size), dirreg.offset, frag_bytes);
					}
					*size=bytes;
					free (blocklist);
					break;
				}
				case SQUASHFS_SYMLINK_TYPE:
				{
					printf ("loading symlinks is not supported\n");
					free (blockbuffer);
					if (frag_table)
						free(frag_table);
					return 0;
					break;
				}
				case SQUASHFS_LDIR_TYPE:
				{
					printf ("loading ldirs is not supported\n");
					free (blockbuffer);
					if (frag_table)
						free(frag_table);
					return 0;
					break;
				}
			}
			}
			parsed=1;
			break;
		}
	}

	free (blockbuffer);
	if (frag_table)
		free(frag_table);

	if (!parsed)
		return 0;
	else
		return 1;
}


int squashfs_ls (struct part_info *info, char *filename)
{
	char *name;
	
	TRACE("looking up %s\n", filename);
	
	if (!strncmp("/",filename,1)||*filename==0x00)
		name = NULL;
	else
		name = filename;
	if (filename[0]=='/')
		name = filename + 1;	/* ignore "/" at start - it's always root */

	if (!squashfs_lookup(info,name,SQUASHFS_FLAGS_LS,0,NULL))
	{
		ERROR ("name not found\n");
		return 0;
	}
	return 1;
}


int squashfs_info (struct part_info *info)
{
	squashfs_super_block super;

	if (!squashfs_read_super(info,&super,NULL))
	{
		ERROR ("reading superblock\n");
		return 0;
	}
	printf ("SquashFS version: %d.%d\n",super.s_major,super.s_minor);
	printf ("Files: %d\n",super.inodes);
	printf ("Bytes_used: %d\n",super.bytes_used);
	printf ("Block_size: %d\n",super.block_size);
	printf ("Inodes are %scompressed\n", SQUASHFS_UNCOMPRESSED_INODES(super.flags) ? "un" : "");
	printf ("Data is %scompressed\n", SQUASHFS_UNCOMPRESSED_DATA(super.flags) ? "un" : "");
	printf ("Fragments are %scompressed\n", SQUASHFS_UNCOMPRESSED_FRAGMENTS(super.flags) ? "un" : "");
	printf ("Check data is %spresent in the filesystem\n", SQUASHFS_CHECK_DATA(super.flags) ? "" : "not ");
	printf ("Fragments are %spresent in the filesystem\n", SQUASHFS_NO_FRAGMENTS(super.flags) ? "not " : "");
	printf ("Always_use_fragments option is %sspecified\n", SQUASHFS_ALWAYS_FRAGMENTS(super.flags) ? "" : "not ");
	printf ("Duplicates are %sremoved\n", SQUASHFS_DUPLICATES(super.flags) ? "" : "not ");
	printf ("Filesystem size %d bytes\n", super.bytes_used);
	printf ("Number of fragments %d\n", super.fragments);
	printf ("Number of inodes %d\n", super.inodes);
	printf ("Number of uids %d\n", super.no_uids);
	printf ("Number of gids %d\n", super.no_guids);

	return 1;
}


int squashfs_load (char *loadoffset, struct part_info *info, char *filename)
{
	unsigned int size = 0;

	if ((!strncmp("/",filename,1))||(filename[0]==0x00))
		return 0;
	if (!squashfs_lookup(info,filename,SQUASHFS_FLAGS_LOAD,(unsigned long)loadoffset,&size))
	{
		return 0;
	}
	return size;
}


int squashfs_check(struct part_info *info)
{
	if (info->size<sizeof(squashfs_super_block))
	{
//		printf("in squashfs_check\n");
        return 0;
	}
	

	struct squashfs_super_block *super = (struct squashfs_super_block *) PART_OFFSET(info);
    /* superblock offset is actually index SQUASHFS_START which is defined as 0 */
//	printf("squashfs_check %p\n",super);

    if (super->s_magic==SQUASHFS_MAGIC_SWAP)
    {
        ERROR ("non-native-endian squashfs is not supported\n");
        return 0;
    }

    if (super->s_magic!=SQUASHFS_MAGIC)
    {
        ERROR ("2 no squashfs_magic: %08x\n",super->s_magic);
        return 0;
    }


    return 1;

}


#endif /* (CONFIG_FS & CFG_FS_SQUASHFS) */
