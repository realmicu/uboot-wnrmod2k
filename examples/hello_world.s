	.section .mdebug.abi32
	.previous
	.abicalls
	.section	.debug_abbrev,"",@progbits
$Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
$Ldebug_info0:
	.section	.debug_line,"",@progbits
$Ldebug_line0:
	.text
$Ltext0:
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"Example expects ABI version %d\n\000"
	.align	2
$LC1:
	.ascii	"Actual U-Boot ABI version %d\n\000"
	.align	2
$LC2:
	.ascii	"Hello World\n\000"
	.align	2
$LC3:
	.ascii	"argc = %d\n\000"
	.align	2
$LC4:
	.ascii	"argv[%d] = \"%s\"\n\000"
	.align	2
$LC5:
	.ascii	"<NULL>\000"
	.align	2
$LC6:
	.ascii	"Hit any key to exit ... \000"
	.align	2
$LC7:
	.ascii	"\n\n\000"
	.text
	.align	2
	.globl	hello_world
	.ent	hello_world
	.type	hello_world, @function
hello_world:
$LFB44:
	.file 1 "hello_world.c"
	.loc 1 28 0
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 4/0, args= 16, gp= 8
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	nomacro
	
	addiu	$sp,$sp,-40
$LCFI0:
	sw	$31,36($sp)
$LCFI1:
	sw	$18,32($sp)
$LCFI2:
	sw	$17,28($sp)
$LCFI3:
	sw	$16,24($sp)
$LCFI4:
	.cprestore	16
	.loc 1 32 0
	lw	$25,%call16(app_startup)($28)
	.loc 1 28 0
	move	$18,$4
	.loc 1 32 0
	move	$4,$5
	.loc 1 28 0
	.loc 1 32 0
	jalr	$25
	move	$16,$5

	lw	$28,16($sp)
	.loc 1 33 0
	li	$5,2			# 0x2
	lw	$4,%got($LC0)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC0)

	lw	$28,16($sp)
	.loc 1 34 0
	lw	$25,%call16(get_version)($28)
	.loc 1 40 0
	.loc 1 34 0
	jalr	$25
	move	$17,$0

	lw	$28,16($sp)
	move	$5,$2
	lw	$4,%got($LC1)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC1)

	lw	$28,16($sp)
	.loc 1 36 0
	lw	$4,%got($LC2)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC2)

	lw	$28,16($sp)
	.loc 1 38 0
	move	$5,$18
	lw	$4,%got($LC3)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC3)

	.loc 1 40 0
	bltz	$18,$L11
	lw	$28,16($sp)

$L7:
	.loc 1 41 0
	lw	$6,0($16)
	lw	$4,%got($LC4)($28)
	move	$5,$17
	lw	$25,%call16(printf)($28)
	.loc 1 40 0
	addiu	$17,$17,1
	addiu	$16,$16,4
	.loc 1 41 0
	bne	$6,$0,$L6
	addiu	$4,$4,%lo($LC4)

	lw	$2,%got($LC5)($28)
	addiu	$6,$2,%lo($LC5)
$L6:
	jalr	$25
	nop

	.loc 1 40 0
	slt	$2,$18,$17
	.loc 1 41 0
	.loc 1 40 0
	beq	$2,$0,$L7
	lw	$28,16($sp)

$L11:
	.loc 1 46 0
	lw	$4,%got($LC6)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC6)

	lw	$28,16($sp)
$L8:
	.loc 1 47 0
	lw	$25,%call16(tstc)($28)
	jalr	$25
	nop

	beq	$2,$0,$L8
	lw	$28,16($sp)

	.loc 1 50 0
	lw	$25,%call16(getc)($28)
	jalr	$25
	nop

	lw	$28,16($sp)
	.loc 1 52 0
	lw	$4,%got($LC7)($28)
	lw	$25,%call16(printf)($28)
	jalr	$25
	addiu	$4,$4,%lo($LC7)

	lw	$28,16($sp)
	.loc 1 54 0
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
$LFE44:
	.end	hello_world
	.section	.debug_frame,"",@progbits
$Lframe0:
	.4byte	$LECIE0-$LSCIE0
$LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 4
	.byte	0x1f
	.byte	0xc
	.uleb128 0x1d
	.uleb128 0x0
	.align	2
$LECIE0:
$LSFDE0:
	.4byte	$LEFDE0-$LASFDE0
$LASFDE0:
	.4byte	$Lframe0
	.4byte	$LFB44
	.4byte	$LFE44-$LFB44
	.byte	0x4
	.4byte	$LCFI0-$LFB44
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.4byte	$LCFI4-$LCFI0
	.byte	0x11
	.uleb128 0x10
	.sleb128 -4
	.byte	0x11
	.uleb128 0x11
	.sleb128 -3
	.byte	0x11
	.uleb128 0x12
	.sleb128 -2
	.byte	0x11
	.uleb128 0x1f
	.sleb128 -1
	.align	2
$LEFDE0:
	.align	0
	.text
$Letext0:
	.section	.debug_info
	.4byte	0xc7
	.2byte	0x2
	.4byte	$Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	$Ldebug_line0
	.4byte	$Letext0
	.4byte	$Ltext0
	.4byte	$LASF12
	.byte	0x1
	.4byte	$LASF13
	.4byte	$LASF14
	.uleb128 0x2
	.4byte	$LASF0
	.byte	0x1
	.byte	0x8
	.uleb128 0x2
	.4byte	$LASF1
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.4byte	$LASF2
	.byte	0x2
	.byte	0x7
	.uleb128 0x2
	.4byte	$LASF3
	.byte	0x4
	.byte	0x7
	.uleb128 0x3
	.ascii	"int\000"
	.byte	0x4
	.byte	0x5
	.uleb128 0x2
	.4byte	$LASF3
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.4byte	$LASF4
	.byte	0x4
	.byte	0x5
	.uleb128 0x4
	.byte	0x4
	.4byte	0x5c
	.uleb128 0x2
	.4byte	$LASF5
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.4byte	$LASF6
	.byte	0x8
	.byte	0x5
	.uleb128 0x2
	.4byte	$LASF7
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.4byte	$LASF8
	.byte	0x2
	.byte	0x5
	.uleb128 0x2
	.4byte	$LASF9
	.byte	0x8
	.byte	0x7
	.uleb128 0x5
	.4byte	0xc4
	.byte	0x1
	.4byte	$LASF15
	.byte	0x1
	.byte	0x1c
	.byte	0x1
	.4byte	0x41
	.4byte	$LFB44
	.4byte	$LFE44
	.4byte	$LSFDE0
	.byte	0x1
	.byte	0x6d
	.uleb128 0x6
	.4byte	$LASF10
	.byte	0x1
	.byte	0x1b
	.4byte	0x41
	.byte	0x1
	.byte	0x62
	.uleb128 0x6
	.4byte	$LASF11
	.byte	0x1
	.byte	0x1b
	.4byte	0xc4
	.byte	0x1
	.byte	0x60
	.uleb128 0x7
	.ascii	"i\000"
	.byte	0x1
	.byte	0x1d
	.4byte	0x41
	.byte	0x1
	.byte	0x61
	.byte	0x0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x56
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x2001
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.4byte	0x1e
	.2byte	0x2
	.4byte	$Ldebug_info0
	.4byte	0xcb
	.4byte	0x7f
	.ascii	"hello_world\000"
	.4byte	0x0
	.section	.debug_aranges,"",@progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	$Ldebug_info0
	.byte	0x4
	.byte	0x0
	.2byte	0x0
	.2byte	0x0
	.4byte	$Ltext0
	.4byte	$Letext0-$Ltext0
	.4byte	0x0
	.4byte	0x0
	.section	.debug_str,"MS",@progbits,1
$LASF14:
	.ascii	"/root/uboot/u-boot-2006-04-06-1725/examples\000"
$LASF9:
	.ascii	"long long unsigned int\000"
$LASF6:
	.ascii	"long long int\000"
$LASF7:
	.ascii	"signed char\000"
$LASF1:
	.ascii	"long unsigned int\000"
$LASF4:
	.ascii	"long int\000"
$LASF2:
	.ascii	"short unsigned int\000"
$LASF13:
	.ascii	"hello_world.c\000"
$LASF3:
	.ascii	"unsigned int\000"
$LASF8:
	.ascii	"short int\000"
$LASF5:
	.ascii	"char\000"
$LASF12:
	.ascii	"GNU C 3.4.4 mipssde-6.03.01-20051114 -g\000"
$LASF0:
	.ascii	"unsigned char\000"
$LASF11:
	.ascii	"argv\000"
$LASF15:
	.ascii	"hello_world\000"
$LASF10:
	.ascii	"argc\000"
	.ident	"GCC: (GNU) 3.4.4 mipssde-6.03.01-20051114"
