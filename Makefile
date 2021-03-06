#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

VERSION = 1
PATCHLEVEL = 1
SUBLEVEL = 4
EXTRAVERSION =
U_BOOT_VERSION = $(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)
VERSION_FILE = include/version_autogenerated.h

HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/i386/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/powerpc/ppc/ \
	    -e s/macppc/ppc/)

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/cygwin/')

export	HOSTARCH HOSTOS

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	TOPDIR

ifndef COMPRESSED_UBOOT
COMPRESSED_UBOOT = 0
endif 

ifeq (include/config.mk,$(wildcard include/config.mk))
# load ARCH, BOARD, and CPU configuration
include include/config.mk
export	ARCH CPU BOARD VENDOR SOC
ifndef CROSS_COMPILE
ifeq ($(HOSTARCH),ppc)
CROSS_COMPILE =
else
ifeq ($(ARCH),ppc)
CROSS_COMPILE = powerpc-linux-
endif
ifeq ($(ARCH),arm)
CROSS_COMPILE = arm-linux-
endif
ifeq ($(ARCH),i386)
ifeq ($(HOSTARCH),i386)
CROSS_COMPILE =
else
CROSS_COMPILE = i386-linux-
endif
endif
ifeq ($(ARCH),mips)
CROSS_COMPILE = mips-linux-
endif
ifeq ($(ARCH),nios)
CROSS_COMPILE = nios-elf-
endif
ifeq ($(ARCH),nios2)
CROSS_COMPILE = nios2-elf-
endif
ifeq ($(ARCH),m68k)
CROSS_COMPILE = m68k-elf-
endif
ifeq ($(ARCH),microblaze)
CROSS_COMPILE = mb-
endif
ifeq ($(ARCH),blackfin)
CROSS_COMPILE = bfin-elf-
endif
endif
endif

export	CROSS_COMPILE

# load other configuration
include $(TOPDIR)/config.mk


#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

OBJS  = cpu/$(CPU)/start.o

ifeq ($(COMPRESSED_UBOOT),1)
OBJS_BOOTSTRAP  = cpu/$(CPU)/start_bootstrap.o
endif

ifeq ($(CPU),i386)
OBJS += cpu/$(CPU)/start16.o
OBJS += cpu/$(CPU)/reset.o
endif
ifeq ($(CPU),ppc4xx)
OBJS += cpu/$(CPU)/resetvec.o
endif
ifeq ($(CPU),mpc83xx)
OBJS += cpu/$(CPU)/resetvec.o
endif
ifeq ($(CPU),mpc85xx)
OBJS += cpu/$(CPU)/resetvec.o
endif
ifeq ($(CPU),bf533)
OBJS += cpu/$(CPU)/start1.o	cpu/$(CPU)/interrupt.o	cpu/$(CPU)/cache.o
OBJS += cpu/$(CPU)/cplbhdlr.o	cpu/$(CPU)/cplbmgr.o	cpu/$(CPU)/flush.o
endif

LIBS  = lib_generic/libgeneric.a
LIBS += common/libcommon.a
LIBS += board/$(BOARDDIR)/lib$(BOARD).a
LIBS += cpu/$(CPU)/lib$(CPU).a
ifdef SOC
LIBS += cpu/$(CPU)/$(SOC)/lib$(SOC).a
endif
LIBS += lib_$(ARCH)/lib$(ARCH).a

ifeq ($(KERNELVER),2.6.31)
LIBS += drivers/libdrivers.a
endif
ifneq ($(COMPRESSED_UBOOT),1)
LIBS += fs/cramfs/libcramfs.a fs/fat/libfat.a fs/fdos/libfdos.a fs/jffs2/libjffs2.a \
	fs/reiserfs/libreiserfs.a fs/ext2/libext2fs.a fs/squashfs/libsquashfs.a
LIBS += disk/libdisk.a
LIBS += dtt/libdtt.a
ifneq ($(KERNELVER),2.6.31)
LIBS += drivers/libdrivers.a
endif
LIBS += drivers/sk98lin/libsk98lin.a
endif

LIBS += net/libnet.a
LIBS += rtc/librtc.a
LIBS += post/libpost.a post/cpu/libcpu.a
LIBS += $(BOARDLIBS)

ifeq ($(COMPRESSED_UBOOT),1)
LIBS_BOOTSTRAP = lib_bootstrap/libbootstrap.a 
#LIBS_BOOTSTRAP += lib_$(CPU)/lib$(CPU).a
LIBS_BOOTSTRAP += board/$(BOARDDIR)/lib$(BOARD).a 
LIBS_BOOTSTRAP += cpu/$(CPU)/lib$(CPU).a
LIBS_BOOTSTRAP += cpu/$(CPU)/$(SOC)/lib$(SOC).a
endif

.PHONY : $(LIBS)

ifeq ($(COMPRESSED_UBOOT),1)
.PHONY : $(LIBS_BOOTSTRAP)
endif

# Add GCC lib
PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc


# The "tools" are needed early, so put this first
# Don't include stuff already done in $(LIBS)
SUBDIRS	= tools \
	  post \
	  post/cpu
.PHONY : $(SUBDIRS)

.PHONY : help

#########################################################################
#########################################################################

ALL = u-boot.srec u-boot.bin System.map

ifeq ($(COMPRESSED_UBOOT),1)
all:		$(ALL) tuboot.bin
else
all:		$(ALL)
endif

u-boot.hex:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

u-boot.srec:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

u-boot.bin:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

u-boot.img:	u-boot.bin
		./tools/mkimage -A $(ARCH) -T firmware -C none \
		-a $(TEXT_BASE) -e 0 \
		-n $(shell sed -n -e 's/.*U_BOOT_VERSION//p' $(VERSION_FILE) | \
			sed -e 's/"[	 ]*$$/ for $(BOARD) board"/') \
		-d $< $@

u-boot.dis:	u-boot
		$(OBJDUMP) -d $< > $@

u-boot:		depend version $(SUBDIRS) $(OBJS) $(LIBS) $(LDSCRIPT)
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		$(LD) $(LDFLAGS) $$UNDEF_SYM $(OBJS) \
			--start-group $(LIBS) --end-group $(PLATFORM_LIBS) \
			-Map u-boot.map -o u-boot

$(LIBS):
		$(MAKE) -C `dirname $@`

$(SUBDIRS):
		$(MAKE) -C $@ all

ifeq ($(COMPRESSED_UBOOT),1)

LZMA = $(BUILD_DIR)/util/lzma

tuboot.bin:	System.map bootstrap.bin u-boot.lzimg	
		@cat bootstrap.bin > $@
		@cat u-boot.lzimg >> $@

u-boot.lzimg: $(obj)u-boot.bin System.map 
		@$(LZMA) e $(obj)u-boot.bin u-boot.bin.lzma
		@./tools/mkimage -A mips -T firmware -C lzma \
		-a 0x$(shell grep "T _start" $(TOPDIR)/System.map | awk '{ printf "%s", $$1 }') \
		-e 0x$(shell grep "T _start" $(TOPDIR)/System.map | awk '{ printf "%s", $$1 }') \
		-n 'u-boot image' -d $(obj)u-boot.bin.lzma $@

bootstrap.bin:	bootstrap
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

bootstrap:	depend version $(SUBDIRS) $(OBJS_BOOTSTRAP) $(LIBS_BOOTSTRAP) $(LDSCRIPT_BOOTSTRAP)
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBS_BOOTSTRAP) |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		$(LD) $(LDFLAGS_BOOTSTRAP) $$UNDEF_SYM $(OBJS_BOOTSTRAP) \
			--start-group $(LIBS_BOOTSTRAP) --end-group $(PLATFORM_LIBS) \
			-Map bootstrap.map -o bootstrap

$(LIBS_BOOTSTRAP):
		$(MAKE) -C `dirname $@`
endif

version:
		@echo -n "#define U_BOOT_VERSION \"U-Boot " > $(VERSION_FILE); \
		echo -n "$(U_BOOT_VERSION)" >> $(VERSION_FILE); \
		# Skip generating git-based version substring
		##echo -n $(shell $(CONFIG_SHELL) $(TOPDIR)/tools/setlocalversion \
		##	 $(TOPDIR)) >> $(VERSION_FILE); \
		echo "\"" >> $(VERSION_FILE)

gdbtools:
		$(MAKE) -C tools/gdb || exit 1

depend dep:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

tags:
		ctags -w `find $(SUBDIRS) include \
				lib_generic board/$(BOARDDIR) cpu/$(CPU) lib_$(ARCH) \
				fs/cramfs fs/fat fs/fdos fs/jffs2 \
				net disk rtc dtt drivers drivers/sk98lin common \
			\( -name CVS -prune \) -o \( -name '*.[ch]' -print \)`

etags:
		etags -a `find $(SUBDIRS) include \
				lib_generic board/$(BOARDDIR) cpu/$(CPU) lib_$(ARCH) \
				fs/cramfs fs/fat fs/fdos fs/jffs2 \
				net disk rtc dtt drivers drivers/sk98lin common \
			\( -name CVS -prune \) -o \( -name '*.[ch]' -print \)`

System.map:	u-boot
		@$(NM) $< | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort > System.map

#########################################################################
else
all install u-boot u-boot.srec depend dep:
	@echo "System not configured - see README" >&2
	@ exit 1
endif

#########################################################################

unconfig:
	@rm -f include/config.h include/config.mk board/*/config.tmp

wnr1000v2_config: 	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR1000V2 1" >>include/config.h
	@./mkconfig -a wnr1000v2 mips mips wnr1000v2 ar7240 ar7240
wnr1000v2-vc_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR1000V2 1" >>include/config.h
	@echo "#define CONFIG_WNR1000V2_VC 1" >>include/config.h
	@./mkconfig -a wnr1000v2 mips mips wnr1000v2 ar7240 ar7240
wnr1000v2-vm_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR1000V2 1" >>include/config.h
	@echo "#define CONFIG_WNR1000V2_VM 1" >>include/config.h
	@./mkconfig -a wnr1000v2 mips mips wnr1000v2 ar7240 ar7240
wnr612v2_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR612 1" >>include/config.h
ifdef FLASH_SIZE
	@echo "#define FLASH_SIZE" $(FLASH_SIZE) >>include/config.h
endif
	@./mkconfig -a wnr612v2 mips mips wnr612 ar7240 ar7240
wnr612_config:		wnr612v2_config
wnr2200_config:		unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR2200 1" >>include/config.h
ifdef FLASH_SIZE
	@echo "#define FLASH_SIZE" $(FLASH_SIZE) >>include/config.h
endif
	@./mkconfig -a wnr2200 mips mips wnr2200 ar7240 ar7240
wnr2000v3_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR2000V3 1" >>include/config.h
ifdef FLASH_SIZE
	@echo "#define FLASH_SIZE" $(FLASH_SIZE) >>include/config.h
endif
	@./mkconfig -a wnr2000v3 mips mips wnr2000v3 ar7240 ar7240
wnr1100_config:		unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7240 1" >>include/config.h
	@echo "#define CONFIG_WNR1100 1" >>include/config.h
	@./mkconfig -a wnr1100 mips mips wnr1100 ar7240 ar7240
wndr3700v1h2_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7100 1" >>include/config.h
	@echo "#define CONFIG_WNDR3700V1H2 1" >>include/config.h
	@echo "#define CONFIG_WNDR3700V1H2_LED 1" >>include/config.h
ifdef FLASH_SIZE
	@echo "#define FLASH_SIZE" $(FLASH_SIZE) >>include/config.h
endif
ifdef COMPACT_UBOOT
	@echo "#define COMPACT_UBOOT" $(COMPACT_UBOOT) >>include/config.h
endif
	@./mkconfig -a wndr3700v1h2 mips mips wndr3700v1h2 ar7100 ar7100
wndr3700u_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_AR7100 1" >>include/config.h
	@echo "#define CONFIG_WNDR3700U 1" >>include/config.h
	@echo "#define CONFIG_WNDR3700U_LED 1" >>include/config.h
ifdef FLASH_SIZE
	@echo "#define FLASH_SIZE" $(FLASH_SIZE) >>include/config.h
endif
ifdef COMPACT_UBOOT
	@echo "#define COMPACT_UBOOT" $(COMPACT_UBOOT) >>include/config.h
endif
	@./mkconfig -a wndr3700u mips mips wndr3700u ar7100 ar7100

wndr3700v1_config:	wndr3700u_config

wndr3700v2_config:	wndr3700v1h2_config

clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a' -o -name .depend  \) -print \
		| xargs rm -f
	rm -f examples/hello_world examples/timer \
	      examples/eepro100_eeprom examples/sched \
	      examples/mem_to_mem_idma2intr examples/82559_eeprom \
	      examples/smc91111_eeprom \
	      examples/test_burst
	rm -f tools/img2srec tools/mkimage tools/envcrc tools/gen_eth_addr
	rm -f tools/mpc86x_clk tools/ncb
	rm -f tools/easylogo/easylogo tools/bmp_logo
	rm -f tools/gdb/astest tools/gdb/gdbcont tools/gdb/gdbsend
	rm -f tools/env/fw_printenv tools/env/fw_setenv
	rm -f board/cray/L1/bootscript.c board/cray/L1/bootscript.image
	rm -f board/netstar/eeprom board/netstar/crcek
	rm -f board/netstar/*.srec board/netstar/*.bin
	rm -f board/trab/trab_fkt board/voiceblue/eeprom
	rm -f board/integratorap/u-boot.lds board/integratorcp/u-boot.lds
ifeq ($(COMPRESSED_UBOOT),1)
	rm -f lib_bootstrap/*.o
	rm -f lib_bootstrap/*.a
	rm -f bootstrap bootstrap.bin tuboot.bin u-boot.lzimg
endif

clobber:	clean
	find . -type f \( -name .depend \
		-o -name '*.srec' -o -name '*.bin' -o -name u-boot.img \) \
		-print0 \
		| xargs -0 rm -f
	rm -f $(OBJS) *.bak tags TAGS include/version_autogenerated.h
	rm -fr *.*~
	rm -f u-boot u-boot.map u-boot.hex $(ALL)
	rm -f tools/crc32.c tools/environment.c tools/env/crc32.c
	rm -f tools/inca-swap-bytes cpu/mpc824x/bedbug_603e.c
	rm -f include/asm/proc include/asm/arch include/asm

mrproper \
distclean:	clobber unconfig

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

help:
	@cat README.netgear


#########################################################################
