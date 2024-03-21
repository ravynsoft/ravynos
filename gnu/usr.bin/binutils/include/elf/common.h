/* ELF support for BFD.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   Written by Fred Fish @ Cygnus Support, from information published
   in "UNIX System V Release 4, Programmers Guide: ANSI C and
   Programming Support Tools".

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* This file is part of ELF support for BFD, and contains the portions
   that are common to both the internal and external representations.
   For example, ELFMAG0 is the byte 0x7F in both the internal (in-memory)
   and external (in-file) representations.  */

#ifndef _ELF_COMMON_H
#define _ELF_COMMON_H

/* Fields in e_ident[].  */

#define EI_MAG0		0	/* File identification byte 0 index */
#define ELFMAG0		   0x7F	/* Magic number byte 0 */

#define EI_MAG1		1	/* File identification byte 1 index */
#define ELFMAG1		    'E'	/* Magic number byte 1 */

#define EI_MAG2		2	/* File identification byte 2 index */
#define ELFMAG2		    'L'	/* Magic number byte 2 */

#define EI_MAG3		3	/* File identification byte 3 index */
#define ELFMAG3		    'F'	/* Magic number byte 3 */

#define EI_CLASS	4	/* File class */
#define ELFCLASSNONE	      0	/* Invalid class */
#define ELFCLASS32	      1	/* 32-bit objects */
#define ELFCLASS64	      2	/* 64-bit objects */

#define EI_DATA		5	/* Data encoding */
#define ELFDATANONE	      0	/* Invalid data encoding */
#define ELFDATA2LSB	      1	/* 2's complement, little endian */
#define ELFDATA2MSB	      2	/* 2's complement, big endian */

#define EI_VERSION	6	/* File version */

#define EI_OSABI	7	/* Operating System/ABI indication */
#define ELFOSABI_NONE	      0	/* UNIX System V ABI */
#define ELFOSABI_HPUX	      1	/* HP-UX operating system */
#define ELFOSABI_NETBSD	      2	/* NetBSD */
#define ELFOSABI_GNU	      3	/* GNU */
#define ELFOSABI_LINUX	      3	/* Alias for ELFOSABI_GNU */
#define ELFOSABI_SOLARIS      6	/* Solaris */
#define ELFOSABI_AIX	      7	/* AIX */
#define ELFOSABI_IRIX	      8	/* IRIX */
#define ELFOSABI_FREEBSD      9	/* FreeBSD */
#define ELFOSABI_TRU64	     10	/* TRU64 UNIX */
#define ELFOSABI_MODESTO     11	/* Novell Modesto */
#define ELFOSABI_OPENBSD     12	/* OpenBSD */
#define ELFOSABI_OPENVMS     13	/* OpenVMS */
#define ELFOSABI_NSK	     14	/* Hewlett-Packard Non-Stop Kernel */
#define ELFOSABI_AROS	     15	/* AROS */
#define ELFOSABI_FENIXOS     16 /* FenixOS */
#define ELFOSABI_CLOUDABI    17 /* Nuxi CloudABI */
#define ELFOSABI_OPENVOS     18 /* Stratus Technologies OpenVOS */

#define ELFOSABI_C6000_ELFABI 64 /* Bare-metal TMS320C6000 */
#define ELFOSABI_AMDGPU_HSA  64 /* AMD HSA Runtime */
#define ELFOSABI_C6000_LINUX 65 /* Linux TMS320C6000 */
#define ELFOSABI_AMDGPU_PAL  65 /* AMD PAL Runtime */
#define ELFOSABI_ARM_FDPIC   65 /* ARM FDPIC */
#define ELFOSABI_AMDGPU_MESA3D 66 /* AMD Mesa3D Runtime */
#define ELFOSABI_ARM	     97	/* ARM */
#define ELFOSABI_STANDALONE 255	/* Standalone (embedded) application */

#define EI_ABIVERSION	8	/* ABI version */

#define EI_PAD		9	/* Start of padding bytes */


/* Values for e_type, which identifies the object file type.  */

#define ET_NONE		0	/* No file type */
#define ET_REL		1	/* Relocatable file */
#define ET_EXEC		2	/* Position-dependent executable file */
#define ET_DYN		3	/* Position-independent executable or
				   shared object file */
#define ET_CORE		4	/* Core file */
#define ET_LOOS		0xFE00	/* Operating system-specific */
#define ET_HIOS		0xFEFF	/* Operating system-specific */
#define ET_LOPROC	0xFF00	/* Processor-specific */
#define ET_HIPROC	0xFFFF	/* Processor-specific */

/* Values for e_machine, which identifies the architecture.  These numbers
   are officially assigned by registry@sco.com.  See below for a list of
   ad-hoc numbers used during initial development.  */

#define EM_NONE		  0	/* No machine */
#define EM_M32		  1	/* AT&T WE 32100 */
#define EM_SPARC	  2	/* SUN SPARC */
#define EM_386		  3	/* Intel 80386 */
#define EM_68K		  4	/* Motorola m68k family */
#define EM_88K		  5	/* Motorola m88k family */
#define EM_IAMCU	  6	/* Intel MCU */
#define EM_860		  7	/* Intel 80860 */
#define EM_MIPS		  8	/* MIPS R3000 (officially, big-endian only) */
#define EM_S370		  9	/* IBM System/370 */
#define EM_MIPS_RS3_LE	 10	/* MIPS R3000 little-endian (Oct 4 1999 Draft).  Deprecated.  */
#define EM_OLD_SPARCV9	 11	/* Old version of Sparc v9, from before the ABI.  Deprecated.  */
#define EM_res011	 11	/* Reserved */
#define EM_res012	 12	/* Reserved */
#define EM_res013	 13	/* Reserved */
#define EM_res014	 14	/* Reserved */
#define EM_PARISC	 15	/* HPPA */
#define EM_res016	 16	/* Reserved */
#define EM_PPC_OLD	 17	/* Old version of PowerPC.  Deprecated.  */
#define EM_VPP550	 17	/* Fujitsu VPP500 */
#define EM_SPARC32PLUS	 18	/* Sun's "v8plus" */
#define EM_960		 19	/* Intel 80960 */
#define EM_PPC		 20	/* PowerPC */
#define EM_PPC64	 21	/* 64-bit PowerPC */
#define EM_S390		 22	/* IBM S/390 */
#define EM_SPU		 23	/* Sony/Toshiba/IBM SPU */
#define EM_res024	 24	/* Reserved */
#define EM_res025	 25	/* Reserved */
#define EM_res026	 26	/* Reserved */
#define EM_res027	 27	/* Reserved */
#define EM_res028	 28	/* Reserved */
#define EM_res029	 29	/* Reserved */
#define EM_res030	 30	/* Reserved */
#define EM_res031	 31	/* Reserved */
#define EM_res032	 32	/* Reserved */
#define EM_res033	 33	/* Reserved */
#define EM_res034	 34	/* Reserved */
#define EM_res035	 35	/* Reserved */
#define EM_V800		 36	/* NEC V800 series */
#define EM_FR20		 37	/* Fujitsu FR20 */
#define EM_RH32		 38	/* TRW RH32 */
#define EM_MCORE	 39	/* Motorola M*Core */ /* May also be taken by Fujitsu MMA */
#define EM_RCE		 39	/* Old name for MCore */
#define EM_ARM		 40	/* ARM */
#define EM_OLD_ALPHA	 41	/* Digital Alpha */
#define EM_SH		 42	/* Renesas (formerly Hitachi) / SuperH SH */
#define EM_SPARCV9	 43	/* SPARC v9 64-bit */
#define EM_TRICORE	 44	/* Siemens Tricore embedded processor */
#define EM_ARC		 45	/* ARC Cores */
#define EM_H8_300	 46	/* Renesas (formerly Hitachi) H8/300 */
#define EM_H8_300H	 47	/* Renesas (formerly Hitachi) H8/300H */
#define EM_H8S		 48	/* Renesas (formerly Hitachi) H8S */
#define EM_H8_500	 49	/* Renesas (formerly Hitachi) H8/500 */
#define EM_IA_64	 50	/* Intel IA-64 Processor */
#define EM_MIPS_X	 51	/* Stanford MIPS-X */
#define EM_COLDFIRE	 52	/* Motorola Coldfire */
#define EM_68HC12	 53	/* Motorola M68HC12 */
#define EM_MMA		 54	/* Fujitsu Multimedia Accelerator */
#define EM_PCP		 55	/* Siemens PCP */
#define EM_NCPU		 56	/* Sony nCPU embedded RISC processor */
#define EM_NDR1		 57	/* Denso NDR1 microprocessor */
#define EM_STARCORE	 58	/* Motorola Star*Core processor */
#define EM_ME16		 59	/* Toyota ME16 processor */
#define EM_ST100	 60	/* STMicroelectronics ST100 processor */
#define EM_TINYJ	 61	/* Advanced Logic Corp. TinyJ embedded processor */
#define EM_X86_64	 62	/* Advanced Micro Devices X86-64 processor */
#define EM_PDSP		 63	/* Sony DSP Processor */
#define EM_PDP10	 64	/* Digital Equipment Corp. PDP-10 */
#define EM_PDP11	 65	/* Digital Equipment Corp. PDP-11 */
#define EM_FX66		 66	/* Siemens FX66 microcontroller */
#define EM_ST9PLUS	 67	/* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7		 68	/* STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16	 69	/* Motorola MC68HC16 Microcontroller */
#define EM_68HC11	 70	/* Motorola MC68HC11 Microcontroller */
#define EM_68HC08	 71	/* Motorola MC68HC08 Microcontroller */
#define EM_68HC05	 72	/* Motorola MC68HC05 Microcontroller */
#define EM_SVX		 73	/* Silicon Graphics SVx */
#define EM_ST19		 74	/* STMicroelectronics ST19 8-bit cpu */
#define EM_VAX		 75	/* Digital VAX */
#define EM_CRIS		 76	/* Axis Communications 32-bit embedded processor */
#define EM_JAVELIN	 77	/* Infineon Technologies 32-bit embedded cpu */
#define EM_FIREPATH	 78	/* Element 14 64-bit DSP processor */
#define EM_ZSP		 79	/* LSI Logic's 16-bit DSP processor */
#define EM_MMIX		 80	/* Donald Knuth's educational 64-bit processor */
#define EM_HUANY	 81	/* Harvard's machine-independent format */
#define EM_PRISM	 82	/* SiTera Prism */
#define EM_AVR		 83	/* Atmel AVR 8-bit microcontroller */
#define EM_FR30		 84	/* Fujitsu FR30 */
#define EM_D10V		 85	/* Mitsubishi D10V */
#define EM_D30V		 86	/* Mitsubishi D30V */
#define EM_V850		 87	/* Renesas V850 (formerly NEC V850) */
#define EM_M32R		 88	/* Renesas M32R (formerly Mitsubishi M32R) */
#define EM_MN10300	 89	/* Matsushita MN10300 */
#define EM_MN10200	 90	/* Matsushita MN10200 */
#define EM_PJ		 91	/* picoJava */
#define EM_OR1K		 92	/* OpenRISC 1000 32-bit embedded processor */
#define EM_ARC_COMPACT	 93	/* ARC International ARCompact processor */
#define EM_XTENSA	 94	/* Tensilica Xtensa Architecture */
#define EM_SCORE_OLD	 95	/* Old Sunplus S+core7 backend magic number. Written in the absence of an ABI.  */
#define EM_VIDEOCORE	 95	/* Alphamosaic VideoCore processor */
#define EM_TMM_GPP	 96	/* Thompson Multimedia General Purpose Processor */
#define EM_NS32K	 97	/* National Semiconductor 32000 series */
#define EM_TPC		 98	/* Tenor Network TPC processor */
#define EM_PJ_OLD	 99	/* Old value for picoJava.  Deprecated.  */
#define EM_SNP1K	 99	/* Trebia SNP 1000 processor */
#define EM_ST200	100	/* STMicroelectronics ST200 microcontroller */
#define EM_IP2K		101	/* Ubicom IP2022 micro controller */
#define EM_MAX		102	/* MAX Processor */
#define EM_CR		103	/* National Semiconductor CompactRISC */
#define EM_F2MC16	104	/* Fujitsu F2MC16 */
#define EM_MSP430	105	/* TI msp430 micro controller */
#define EM_BLACKFIN	106	/* ADI Blackfin */
#define EM_SE_C33	107	/* S1C33 Family of Seiko Epson processors */
#define EM_SEP		108	/* Sharp embedded microprocessor */
#define EM_ARCA		109	/* Arca RISC Microprocessor */
#define EM_UNICORE	110	/* Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University */
#define EM_EXCESS	111	/* eXcess: 16/32/64-bit configurable embedded CPU */
#define EM_DXP		112	/* Icera Semiconductor Inc. Deep Execution Processor */
#define EM_ALTERA_NIOS2	113	/* Altera Nios II soft-core processor */
#define EM_CRX		114	/* National Semiconductor CRX */
#define EM_CR16_OLD	115	/* Old, value for National Semiconductor CompactRISC.  Deprecated.  */
#define EM_XGATE	115	/* Motorola XGATE embedded processor */
#define EM_C166		116	/* Infineon C16x/XC16x processor */
#define EM_M16C		117	/* Renesas M16C series microprocessors */
#define EM_DSPIC30F	118	/* Microchip Technology dsPIC30F Digital Signal Controller */
#define EM_CE		119	/* Freescale Communication Engine RISC core */
#define EM_M32C		120	/* Renesas M32C series microprocessors */
#define EM_res121	121	/* Reserved */
#define EM_res122	122	/* Reserved */
#define EM_res123	123	/* Reserved */
#define EM_res124	124	/* Reserved */
#define EM_res125	125	/* Reserved */
#define EM_res126	126	/* Reserved */
#define EM_res127	127	/* Reserved */
#define EM_res128	128	/* Reserved */
#define EM_res129	129	/* Reserved */
#define EM_res130	130	/* Reserved */
#define EM_TSK3000	131	/* Altium TSK3000 core */
#define EM_RS08		132	/* Freescale RS08 embedded processor */
#define EM_res133	133	/* Reserved */
#define EM_ECOG2	134	/* Cyan Technology eCOG2 microprocessor */
#define EM_SCORE	135	/* Sunplus Score */
#define EM_SCORE7	135	/* Sunplus S+core7 RISC processor */
#define EM_DSP24	136	/* New Japan Radio (NJR) 24-bit DSP Processor */
#define EM_VIDEOCORE3	137	/* Broadcom VideoCore III processor */
#define EM_LATTICEMICO32 138	/* RISC processor for Lattice FPGA architecture */
#define EM_SE_C17	139	/* Seiko Epson C17 family */
#define EM_TI_C6000	140	/* Texas Instruments TMS320C6000 DSP family */
#define EM_TI_C2000	141	/* Texas Instruments TMS320C2000 DSP family */
#define EM_TI_C5500	142	/* Texas Instruments TMS320C55x DSP family */
#define EM_res143	143	/* Reserved */
#define EM_TI_PRU	144	/* Texas Instruments Programmable Realtime Unit */
#define EM_res145	145	/* Reserved */
#define EM_res146	146	/* Reserved */
#define EM_res147	147	/* Reserved */
#define EM_res148	148	/* Reserved */
#define EM_res149	149	/* Reserved */
#define EM_res150	150	/* Reserved */
#define EM_res151	151	/* Reserved */
#define EM_res152	152	/* Reserved */
#define EM_res153	153	/* Reserved */
#define EM_res154	154	/* Reserved */
#define EM_res155	155	/* Reserved */
#define EM_res156	156	/* Reserved */
#define EM_res157	157	/* Reserved */
#define EM_res158	158	/* Reserved */
#define EM_res159	159	/* Reserved */
#define EM_MMDSP_PLUS	160	/* STMicroelectronics 64bit VLIW Data Signal Processor */
#define EM_CYPRESS_M8C	161	/* Cypress M8C microprocessor */
#define EM_R32C		162	/* Renesas R32C series microprocessors */
#define EM_TRIMEDIA	163	/* NXP Semiconductors TriMedia architecture family */
#define EM_QDSP6	164	/* QUALCOMM DSP6 Processor */
#define EM_8051		165	/* Intel 8051 and variants */
#define EM_STXP7X	166	/* STMicroelectronics STxP7x family */
#define EM_NDS32	167	/* Andes Technology compact code size embedded RISC processor family */
#define EM_ECOG1	168	/* Cyan Technology eCOG1X family */
#define EM_ECOG1X	168	/* Cyan Technology eCOG1X family */
#define EM_MAXQ30	169	/* Dallas Semiconductor MAXQ30 Core Micro-controllers */
#define EM_XIMO16	170	/* New Japan Radio (NJR) 16-bit DSP Processor */
#define EM_MANIK	171	/* M2000 Reconfigurable RISC Microprocessor */
#define EM_CRAYNV2	172	/* Cray Inc. NV2 vector architecture */
#define EM_RX		173	/* Renesas RX family */
#define EM_METAG	174	/* Imagination Technologies Meta processor architecture */
#define EM_MCST_ELBRUS	175	/* MCST Elbrus general purpose hardware architecture */
#define EM_ECOG16	176	/* Cyan Technology eCOG16 family */
#define EM_CR16		177	/* National Semiconductor CompactRISC 16-bit processor */
#define EM_ETPU		178	/* Freescale Extended Time Processing Unit */
#define EM_SLE9X	179	/* Infineon Technologies SLE9X core */
#define EM_L1OM		180	/* Intel L1OM */
#define EM_K1OM		181	/* Intel K1OM */
#define EM_INTEL182	182	/* Reserved by Intel */
#define EM_AARCH64	183	/* ARM 64-bit architecture */
#define EM_ARM184	184	/* Reserved by ARM */
#define EM_AVR32	185	/* Atmel Corporation 32-bit microprocessor family */
#define EM_STM8		186	/* STMicroeletronics STM8 8-bit microcontroller */
#define EM_TILE64	187	/* Tilera TILE64 multicore architecture family */
#define EM_TILEPRO	188	/* Tilera TILEPro multicore architecture family */
#define EM_MICROBLAZE	189	/* Xilinx MicroBlaze 32-bit RISC soft processor core */
#define EM_CUDA		190	/* NVIDIA CUDA architecture */
#define EM_TILEGX	191	/* Tilera TILE-Gx multicore architecture family */
#define EM_CLOUDSHIELD 	192 	/* CloudShield architecture family */
#define EM_COREA_1ST 	193 	/* KIPO-KAIST Core-A 1st generation processor family */
#define EM_COREA_2ND 	194 	/* KIPO-KAIST Core-A 2nd generation processor family */
#define EM_ARC_COMPACT2 195	/* Synopsys ARCompact V2 */
#define EM_OPEN8 	196 	/* Open8 8-bit RISC soft processor core */
#define EM_RL78		197	/* Renesas RL78 family.  */
#define EM_VIDEOCORE5 	198 	/* Broadcom VideoCore V processor */
#define EM_78K0R	199	/* Renesas 78K0R.  */
#define EM_56800EX 	200 	/* Freescale 56800EX Digital Signal Controller (DSC) */
#define EM_BA1 		201 	/* Beyond BA1 CPU architecture */
#define EM_BA2 		202 	/* Beyond BA2 CPU architecture */
#define EM_XCORE 	203 	/* XMOS xCORE processor family */
#define EM_MCHP_PIC 	204 	/* Microchip 8-bit PIC(r) family */
#define EM_INTELGT	205	/* Intel Graphics Technology */
#define EM_INTEL206	206	/* Reserved by Intel */
#define EM_INTEL207	207	/* Reserved by Intel */
#define EM_INTEL208	208	/* Reserved by Intel */
#define EM_INTEL209	209	/* Reserved by Intel */
#define EM_KM32 	210 	/* KM211 KM32 32-bit processor */
#define EM_KMX32 	211 	/* KM211 KMX32 32-bit processor */
#define EM_KMX16 	212 	/* KM211 KMX16 16-bit processor */
#define EM_KMX8 	213 	/* KM211 KMX8 8-bit processor */
#define EM_KVARC 	214 	/* KM211 KVARC processor */
#define EM_CDP 		215 	/* Paneve CDP architecture family */
#define EM_COGE 	216 	/* Cognitive Smart Memory Processor */
#define EM_COOL 	217 	/* Bluechip Systems CoolEngine */
#define EM_NORC 	218 	/* Nanoradio Optimized RISC */
#define EM_CSR_KALIMBA 	219 	/* CSR Kalimba architecture family */
#define EM_Z80 		220 	/* Zilog Z80 */
#define EM_VISIUM	221	/* Controls and Data Services VISIUMcore processor */
#define EM_FT32         222     /* FTDI Chip FT32 high performance 32-bit RISC architecture */
#define EM_MOXIE        223     /* Moxie processor family */
#define EM_AMDGPU 	224 	/* AMD GPU architecture */
#define EM_RISCV 	243 	/* RISC-V */
#define EM_LANAI	244	/* Lanai 32-bit processor.  */
#define EM_CEVA		245	/* CEVA Processor Architecture Family */
#define EM_CEVA_X2	246	/* CEVA X2 Processor Family */
#define EM_BPF		247	/* Linux BPF – in-kernel virtual machine.  */
#define EM_GRAPHCORE_IPU 248	/* Graphcore Intelligent Processing Unit */
#define EM_IMG1		249	/* Imagination Technologies */
#define EM_NFP		250	/* Netronome Flow Processor.  */
#define EM_VE		251	/* NEC Vector Engine */
#define EM_CSKY		252	/* C-SKY processor family.  */
#define EM_ARC_COMPACT3_64 253	/* Synopsys ARCv2.3 64-bit */
#define EM_MCS6502	254	/* MOS Technology MCS 6502 processor */
#define EM_ARC_COMPACT3	255	/* Synopsys ARCv2.3 32-bit */
#define EM_KVX		256	/* Kalray VLIW core of the MPPA processor family */
#define EM_65816	257	/* WDC 65816/65C816 */
#define EM_LOONGARCH	258	/* LoongArch */
#define EM_KF32		259	/* ChipON KungFu32 */
#define EM_U16_U8CORE	260	/* LAPIS nX-U16/U8 */
#define EM_TACHYUM	261	/* Tachyum */
#define EM_56800EF	262	/* NXP 56800EF Digital Signal Controller (DSC) */

/* If it is necessary to assign new unofficial EM_* values, please pick large
   random numbers (0x8523, 0xa7f2, etc.) to minimize the chances of collision
   with official or non-GNU unofficial values.

   NOTE: Do not just increment the most recent number by one.
   Somebody else somewhere will do exactly the same thing, and you
   will have a collision.  Instead, pick a random number.

   Normally, each entity or maintainer responsible for a machine with an
   unofficial e_machine number should eventually ask registry@sco.com for
   an officially blessed number to be added to the list above.	*/

/* AVR magic number.  Written in the absense of an ABI.  */
#define EM_AVR_OLD		0x1057

/* MSP430 magic number.  Written in the absense of everything.  */
#define EM_MSP430_OLD		0x1059

/* Morpho MT.   Written in the absense of an ABI.  */
#define EM_MT			0x2530

/* FR30 magic number - no EABI available.  */
#define EM_CYGNUS_FR30		0x3330

/* Unofficial value for Web Assembly binaries, as used by LLVM.  */
#define EM_WEBASSEMBLY		0x4157

/* Freescale S12Z.   The Freescale toolchain generates elf files with this value.  */
#define EM_S12Z               0x4DEF

/* DLX magic number.  Written in the absense of an ABI.  */
#define EM_DLX			0x5aa5

/* FRV magic number - no EABI available??.  */
#define EM_CYGNUS_FRV		0x5441

/* Infineon Technologies 16-bit microcontroller with C166-V2 core.  */
#define EM_XC16X		0x4688

/* D10V backend magic number.  Written in the absence of an ABI.  */
#define EM_CYGNUS_D10V		0x7650

/* D30V backend magic number.  Written in the absence of an ABI.  */
#define EM_CYGNUS_D30V		0x7676

/* Ubicom IP2xxx;   Written in the absense of an ABI.  */
#define EM_IP2K_OLD		0x8217

/* Cygnus PowerPC ELF backend.  Written in the absence of an ABI.  */
#define EM_CYGNUS_POWERPC	0x9025

/* Alpha backend magic number.  Written in the absence of an ABI.  */
#define EM_ALPHA		0x9026

/* Cygnus M32R ELF backend.  Written in the absence of an ABI.  */
#define EM_CYGNUS_M32R		0x9041

/* V850 backend magic number.  Written in the absense of an ABI.  */
#define EM_CYGNUS_V850		0x9080

/* old S/390 backend magic number. Written in the absence of an ABI.  */
#define EM_S390_OLD		0xa390

/* Old, unofficial value for Xtensa.  */
#define EM_XTENSA_OLD		0xabc7

#define EM_XSTORMY16		0xad45

/* mn10200 and mn10300 backend magic numbers.
   Written in the absense of an ABI.  */
#define EM_CYGNUS_MN10300	0xbeef
#define EM_CYGNUS_MN10200	0xdead

/* Renesas M32C and M16C.  */
#define EM_M32C_OLD		0xFEB0

/* Vitesse IQ2000.  */
#define EM_IQ2000		0xFEBA

/* NIOS magic number - no EABI available.  */
#define EM_NIOS32		0xFEBB

#define EM_CYGNUS_MEP		0xF00D  /* Toshiba MeP */

/* Old, unofficial value for Moxie.  */
#define EM_MOXIE_OLD            0xFEED

#define EM_MICROBLAZE_OLD	0xbaab	/* Old MicroBlaze */

#define EM_ADAPTEVA_EPIPHANY	0x1223  /* Adapteva's Epiphany architecture.  */

/* Old constant that might be in use by some software. */
#define EM_OPENRISC		EM_OR1K

/* C-SKY historically used 39, the same value as MCORE, from which the
   architecture was derived.  */
#define EM_CSKY_OLD		EM_MCORE

/* See the above comment before you add a new EM_* value here.  */

/* Values for e_version.  */

#define EV_NONE		0		/* Invalid ELF version */
#define EV_CURRENT	1		/* Current version */

/* Value for e_phnum. */
#define PN_XNUM		0xffff		/* Extended numbering */

/* Values for program header, p_type field.  */

#define PT_NULL		0		/* Program header table entry unused */
#define PT_LOAD		1		/* Loadable program segment */
#define PT_DYNAMIC	2		/* Dynamic linking information */
#define PT_INTERP	3		/* Program interpreter */
#define PT_NOTE		4		/* Auxiliary information */
#define PT_SHLIB	5		/* Reserved, unspecified semantics */
#define PT_PHDR		6		/* Entry for header table itself */
#define PT_TLS		7		/* Thread local storage segment */
#define PT_LOOS		0x60000000	/* OS-specific */
#define PT_HIOS		0x6fffffff	/* OS-specific */
#define PT_LOPROC	0x70000000	/* Processor-specific */
#define PT_HIPROC	0x7FFFFFFF	/* Processor-specific */

#define PT_GNU_EH_FRAME	(PT_LOOS + 0x474e550) /* Frame unwind information */
#define PT_SUNW_EH_FRAME PT_GNU_EH_FRAME      /* Solaris uses the same value */
#define PT_GNU_STACK	(PT_LOOS + 0x474e551) /* Stack flags */
#define PT_GNU_RELRO	(PT_LOOS + 0x474e552) /* Read-only after relocation */
#define PT_GNU_PROPERTY	(PT_LOOS + 0x474e553) /* GNU property */
#define PT_GNU_SFRAME	(PT_LOOS + 0x474e554) /* SFrame stack trace information */

/* OpenBSD segment types.  */
#define PT_OPENBSD_MUTABLE   (PT_LOOS + 0x5a3dbe5)  /* Like bss, but not immutable.  */
#define PT_OPENBSD_RANDOMIZE (PT_LOOS + 0x5a3dbe6)  /* Fill with random data.  */
#define PT_OPENBSD_WXNEEDED  (PT_LOOS + 0x5a3dbe7)  /* Program does W^X violations.  */
#define PT_OPENBSD_BOOTDATA  (PT_LOOS + 0x5a41be6)  /* Section for boot arguments.  */

/* Mbind segments */
#define PT_GNU_MBIND_NUM 4096
#define PT_GNU_MBIND_LO (PT_LOOS + 0x474e555)
#define PT_GNU_MBIND_HI (PT_GNU_MBIND_LO + PT_GNU_MBIND_NUM - 1)

/* Program segment permissions, in program header p_flags field.  */

#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_W		(1 << 1)	/* Segment is writable */
#define PF_R		(1 << 2)	/* Segment is readable */
/* #define PF_MASKOS	0x0F000000    *//* OS-specific reserved bits */
#define PF_MASKOS	0x0FF00000	/* New value, Oct 4, 1999 Draft */
#define PF_MASKPROC	0xF0000000	/* Processor-specific reserved bits */

/* Values for section header, sh_type field.  */

#define SHT_NULL	0		/* Section header table entry unused */
#define SHT_PROGBITS	1		/* Program specific (private) data */
#define SHT_SYMTAB	2		/* Link editing symbol table */
#define SHT_STRTAB	3		/* A string table */
#define SHT_RELA	4		/* Relocation entries with addends */
#define SHT_HASH	5		/* A symbol hash table */
#define SHT_DYNAMIC	6		/* Information for dynamic linking */
#define SHT_NOTE	7		/* Information that marks file */
#define SHT_NOBITS	8		/* Section occupies no space in file */
#define SHT_REL		9		/* Relocation entries, no addends */
#define SHT_SHLIB	10		/* Reserved, unspecified semantics */
#define SHT_DYNSYM	11		/* Dynamic linking symbol table */

#define SHT_INIT_ARRAY	  14		/* Array of ptrs to init functions */
#define SHT_FINI_ARRAY	  15		/* Array of ptrs to finish functions */
#define SHT_PREINIT_ARRAY 16		/* Array of ptrs to pre-init funcs */
#define SHT_GROUP	  17		/* Section contains a section group */
#define SHT_SYMTAB_SHNDX  18		/* Indices for SHN_XINDEX entries */
#define SHT_RELR	  19		/* RELR relative relocations */

#define SHT_LOOS	0x60000000	/* First of OS specific semantics */
#define SHT_HIOS	0x6fffffff	/* Last of OS specific semantics */

#define SHT_GNU_INCREMENTAL_INPUTS 0x6fff4700   /* incremental build data */
#define SHT_GNU_ATTRIBUTES 0x6ffffff5	/* Object attributes */
#define SHT_GNU_HASH	0x6ffffff6	/* GNU style symbol hash table */
#define SHT_GNU_LIBLIST	0x6ffffff7	/* List of prelink dependencies */

/* The next three section types are defined by Solaris, and are named
   SHT_SUNW*.  We use them in GNU code, so we also define SHT_GNU*
   versions.  */
#define SHT_SUNW_verdef	0x6ffffffd	/* Versions defined by file */
#define SHT_SUNW_verneed 0x6ffffffe	/* Versions needed by file */
#define SHT_SUNW_versym	0x6fffffff	/* Symbol versions */

#define SHT_GNU_verdef	SHT_SUNW_verdef
#define SHT_GNU_verneed	SHT_SUNW_verneed
#define SHT_GNU_versym	SHT_SUNW_versym

#define SHT_LOPROC	0x70000000	/* Processor-specific semantics, lo */
#define SHT_HIPROC	0x7FFFFFFF	/* Processor-specific semantics, hi */
#define SHT_LOUSER	0x80000000	/* Application-specific semantics */
/* #define SHT_HIUSER	0x8FFFFFFF    *//* Application-specific semantics */
#define SHT_HIUSER	0xFFFFFFFF	/* New value, defined in Oct 4, 1999 Draft */

/* Values for section header, sh_flags field.  */

#define SHF_WRITE	(1 << 0)	/* Writable data during execution */
#define SHF_ALLOC	(1 << 1)	/* Occupies memory during execution */
#define SHF_EXECINSTR	(1 << 2)	/* Executable machine instructions */
#define SHF_MERGE	(1 << 4)	/* Data in this section can be merged */
#define SHF_STRINGS	(1 << 5)	/* Contains null terminated character strings */
#define SHF_INFO_LINK	(1 << 6)	/* sh_info holds section header table index */
#define SHF_LINK_ORDER	(1 << 7)	/* Preserve section ordering when linking */
#define SHF_OS_NONCONFORMING (1 << 8)	/* OS specific processing required */
#define SHF_GROUP	(1 << 9)	/* Member of a section group */
#define SHF_TLS		(1 << 10)	/* Thread local storage section */
#define SHF_COMPRESSED	(1 << 11)	/* Section with compressed data */

/* #define SHF_MASKOS	0x0F000000    *//* OS-specific semantics */
#define SHF_MASKOS	0x0FF00000	/* New value, Oct 4, 1999 Draft */
#define SHF_GNU_RETAIN	      (1 << 21)	/* Section should not be garbage collected by the linker.  */
#define SHF_MASKPROC	0xF0000000	/* Processor-specific semantics */

/* This used to be implemented as a processor specific section flag.
   We just make it generic.  */
#define SHF_EXCLUDE	0x80000000	/* Link editor is to exclude
					   this section from executable
					   and shared library that it
					   builds when those objects
					   are not to be further
					   relocated.  */

#define SHF_GNU_MBIND	0x01000000	/* Mbind section.  */

/* Compression types.  */
#define ELFCOMPRESS_ZLIB   1		/* Compressed with zlib.  */
#define ELFCOMPRESS_ZSTD   2		/* Compressed with zstd  */
					/* (see http://www.zstandard.org). */
#define ELFCOMPRESS_LOOS   0x60000000	/* OS-specific semantics, lo */
#define ELFCOMPRESS_HIOS   0x6FFFFFFF	/* OS-specific semantics, hi */
#define ELFCOMPRESS_LOPROC 0x70000000	/* Processor-specific semantics, lo */
#define ELFCOMPRESS_HIPROC 0x7FFFFFFF	/* Processor-specific semantics, hi */

/* Values of note segment descriptor types for core files.  */

#define NT_PRSTATUS	1		/* Contains copy of prstatus struct */
#define NT_FPREGSET	2		/* Contains copy of fpregset struct */
#define NT_PRPSINFO	3		/* Contains copy of prpsinfo struct */
#define NT_TASKSTRUCT	4		/* Contains copy of task struct */
#define NT_AUXV		6		/* Contains copy of Elfxx_auxv_t */
#define NT_PRXFPREG	0x46e62b7f	/* Contains a user_xfpregs_struct; */
					/*   note name must be "LINUX".  */
#define NT_PPC_VMX	0x100		/* PowerPC Altivec/VMX registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_VSX	0x102		/* PowerPC VSX registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TAR	0x103		/* PowerPC Target Address Register */
					/*   note name must be "LINUX".  */
#define NT_PPC_PPR	0x104		/* PowerPC Program Priority Register */
					/*   note name must be "LINUX".  */
#define NT_PPC_DSCR	0x105		/* PowerPC Data Stream Control Register */
					/*   note name must be "LINUX".  */
#define NT_PPC_EBB	0x106		/* PowerPC Event Based Branch Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_PMU	0x107		/* PowerPC Performance Monitor Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CGPR	0x108		/* PowerPC TM checkpointed GPR Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CFPR	0x109		/* PowerPC TM checkpointed FPR Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CVMX	0x10a		/* PowerPC TM checkpointed VMX Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CVSX	0x10b		/* PowerPC TM checkpointed VSX Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_SPR	0x10c		/* PowerPC TM Special Purpose Registers */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CTAR	0x10d		/* PowerPC TM checkpointed TAR */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CPPR	0x10e		/* PowerPC TM checkpointed PPR */
					/*   note name must be "LINUX".  */
#define NT_PPC_TM_CDSCR	0x10f		/* PowerPC TM checkpointed Data SCR */
					/*   note name must be "LINUX".  */
#define NT_386_TLS	0x200		/* x86 TLS information */
					/*   note name must be "LINUX".  */
#define NT_386_IOPERM	0x201		/* x86 io permissions */
					/*   note name must be "LINUX".  */
#define NT_X86_XSTATE	0x202		/* x86 XSAVE extended state */
					/*   note name must be "LINUX".  */
#define NT_X86_CET	0x203		/* x86 CET state.  */
					/*   note name must be "LINUX".  */
#define NT_S390_HIGH_GPRS 0x300		/* S/390 upper halves of GPRs  */
					/*   note name must be "LINUX".  */
#define NT_S390_TIMER	0x301		/* S390 timer */
					/*   note name must be "LINUX".  */
#define NT_S390_TODCMP	0x302		/* S390 TOD clock comparator */
					/*   note name must be "LINUX".  */
#define NT_S390_TODPREG	0x303		/* S390 TOD programmable register */
					/*   note name must be "LINUX".  */
#define NT_S390_CTRS	0x304		/* S390 control registers */
					/*   note name must be "LINUX".  */
#define NT_S390_PREFIX	0x305		/* S390 prefix register */
					/*   note name must be "LINUX".  */
#define NT_S390_LAST_BREAK      0x306   /* S390 breaking event address */
					/*   note name must be "LINUX".  */
#define NT_S390_SYSTEM_CALL     0x307   /* S390 system call restart data */
					/*   note name must be "LINUX".  */
#define NT_S390_TDB	0x308		/* S390 transaction diagnostic block */
					/*   note name must be "LINUX".  */
#define NT_S390_VXRS_LOW	0x309	/* S390 vector registers 0-15 upper half */
					/*   note name must be "LINUX".  */
#define NT_S390_VXRS_HIGH	0x30a	/* S390 vector registers 16-31 */
					/*   note name must be "LINUX".  */
#define NT_S390_GS_CB	0x30b		/* s390 guarded storage registers */
					/*   note name must be "LINUX".  */
#define NT_S390_GS_BC	0x30c		/* s390 guarded storage broadcast control block */
					/*   note name must be "LINUX".  */
#define NT_ARM_VFP	0x400		/* ARM VFP registers */
/* The following definitions should really use NT_AARCH_..., but defined
   this way for compatibility with Linux.  */
#define NT_ARM_TLS	0x401		/* AArch TLS registers */
					/*   note name must be "LINUX".  */
#define NT_ARM_HW_BREAK	0x402		/* AArch hardware breakpoint registers */
					/*   note name must be "LINUX".  */
#define NT_ARM_HW_WATCH	0x403		/* AArch hardware watchpoint registers */
					/*   note name must be "LINUX".  */
#define NT_ARM_SYSTEM_CALL      0x404   /* AArch ARM system call number */
					/*   note name must be "LINUX".  */
#define NT_ARM_SVE	0x405		/* AArch SVE registers.  */
					/*   note name must be "LINUX".  */
#define NT_ARM_PAC_MASK	0x406		/* AArch pointer authentication code masks */
					/*   note name must be "LINUX".  */
#define NT_ARM_PACA_KEYS  0x407		/* ARM pointer authentication address
					   keys */
					/*   note name must be "LINUX".  */
#define NT_ARM_PACG_KEYS  0x408		/* ARM pointer authentication generic
					   keys */
					/*  note name must be "LINUX".  */
#define NT_ARM_TAGGED_ADDR_CTRL	0x409	/* AArch64 tagged address control
					   (prctl()) */
					/*   note name must be "LINUX".  */
#define NT_ARM_PAC_ENABLED_KEYS	0x40a	/* AArch64 pointer authentication
					   enabled keys (prctl()) */
					/*   note name must be "LINUX".  */
#define NT_ARM_SSVE     0x40b        	/* AArch64 SME streaming SVE registers.  */
					/*   Note: name must be "LINUX".  */
#define NT_ARM_ZA       0x40c           /* AArch64 SME ZA register.  */
					/*   Note: name must be "LINUX".  */
#define NT_ARC_V2	0x600		/* ARC HS accumulator/extra registers.  */
					/*   note name must be "LINUX".  */
#define NT_LARCH_CPUCFG 0xa00		/* LoongArch CPU config registers */
					/*   note name must be "LINUX".  */
#define NT_LARCH_CSR    0xa01		/* LoongArch Control State Registers */
					/*   note name must be "LINUX".  */
#define NT_LARCH_LSX    0xa02		/* LoongArch SIMD eXtension registers */
					/*   note name must be "LINUX".  */
#define NT_LARCH_LASX   0xa03		/* LoongArch Advanced SIMD eXtension registers */
					/*   note name must be "LINUX".  */
#define NT_LARCH_LBT    0xa04		/* LoongArch Binary Translation registers */
					/*   note name must be "CORE".  */
#define NT_RISCV_CSR    0x900		/* RISC-V Control and Status Registers */
					/*   note name must be "LINUX".  */
#define NT_SIGINFO	0x53494749	/* Fields of siginfo_t.  */
#define NT_FILE		0x46494c45	/* Description of mapped files.  */

/* The range 0xff000000 to 0xffffffff is set aside for notes that don't
   originate from any particular operating system.  */
#define NT_GDB_TDESC	0xff000000	/* Contains copy of GDB's target description XML.  */

/* Note segments for core files on dir-style procfs systems.  */

#define NT_PSTATUS	10		/* Has a struct pstatus */
#define NT_FPREGS	12		/* Has a struct fpregset */
#define NT_PSINFO	13		/* Has a struct psinfo */
#define NT_LWPSTATUS	16		/* Has a struct lwpstatus_t */
#define NT_LWPSINFO	17		/* Has a struct lwpsinfo_t */
#define NT_WIN32PSTATUS	18		/* Has a struct win32_pstatus */

/* Note segment for SystemTap probes.  */
#define NT_STAPSDT	3

/* Note segments for core files on FreeBSD systems.  Note name is
   "FreeBSD".  */

#define	NT_FREEBSD_THRMISC	7	/* Thread miscellaneous info. */
#define	NT_FREEBSD_PROCSTAT_PROC	8	/* Procstat proc data. */
#define	NT_FREEBSD_PROCSTAT_FILES	9	/* Procstat files data. */
#define	NT_FREEBSD_PROCSTAT_VMMAP	10	/* Procstat vmmap data. */
#define	NT_FREEBSD_PROCSTAT_GROUPS	11	/* Procstat groups data. */
#define	NT_FREEBSD_PROCSTAT_UMASK	12	/* Procstat umask data. */
#define	NT_FREEBSD_PROCSTAT_RLIMIT	13	/* Procstat rlimit data. */
#define	NT_FREEBSD_PROCSTAT_OSREL	14	/* Procstat osreldate data. */
#define	NT_FREEBSD_PROCSTAT_PSSTRINGS	15	/* Procstat ps_strings data. */
#define	NT_FREEBSD_PROCSTAT_AUXV	16	/* Procstat auxv data. */
#define	NT_FREEBSD_PTLWPINFO	17	/* Thread ptrace miscellaneous info. */
#define	NT_FREEBSD_X86_SEGBASES	0x200	/* x86 segment base registers */

/* Note segments for core files on NetBSD systems.  Note name
   must start with "NetBSD-CORE".  */

#define NT_NETBSDCORE_PROCINFO	1	/* Has a struct procinfo */
#define NT_NETBSDCORE_AUXV	2	/* Has auxv data */
#define NT_NETBSDCORE_LWPSTATUS	24	/* Has LWPSTATUS data */
#define NT_NETBSDCORE_FIRSTMACH	32	/* start of machdep note types */


/* Note segments for core files on OpenBSD systems.  Note name is
   "OpenBSD".  */

#define NT_OPENBSD_PROCINFO	10
#define NT_OPENBSD_AUXV		11
#define NT_OPENBSD_REGS		20
#define NT_OPENBSD_FPREGS	21
#define NT_OPENBSD_XFPREGS	22
#define NT_OPENBSD_WCOOKIE	23

/* Note segments for core files on QNX systems.  Note name
   must start with "QNX".  */
#define QNT_DEBUG_FULLPATH 1
#define QNT_DEBUG_RELOC    2
#define QNT_STACK          3
#define QNT_GENERATOR      4
#define QNT_DEFAULT_LIB    5
#define QNT_CORE_SYSINFO   6
#define QNT_CORE_INFO      7
#define QNT_CORE_STATUS    8
#define QNT_CORE_GREG      9
#define QNT_CORE_FPREG     10
#define QNT_LINK_MAP       11

/* Note segments for core files on Solaris systems.  Note name
   must start with "CORE".  */
#define SOLARIS_NT_PRSTATUS    1
#define SOLARIS_NT_PRFPREG     2
#define SOLARIS_NT_PRPSINFO    3
#define SOLARIS_NT_PRXREG      4
#define SOLARIS_NT_PLATFORM    5
#define SOLARIS_NT_AUXV        6
#define SOLARIS_NT_GWINDOWS    7
#define SOLARIS_NT_ASRS        8
#define SOLARIS_NT_LDT         9
#define SOLARIS_NT_PSTATUS    10
#define SOLARIS_NT_PSINFO     13
#define SOLARIS_NT_PRCRED     14
#define SOLARIS_NT_UTSNAME    15
#define SOLARIS_NT_LWPSTATUS  16
#define SOLARIS_NT_LWPSINFO   17
#define SOLARIS_NT_PRPRIV     18
#define SOLARIS_NT_PRPRIVINFO 19
#define SOLARIS_NT_CONTENT    20
#define SOLARIS_NT_ZONENAME   21
#define SOLARIS_NT_PRCPUXREG  22

/* Note segments for core files on SPU systems.  Note name
   must start with "SPU/".  */

#define NT_SPU		1

/* Values of note segment descriptor types for object files.  */

#define NT_VERSION	1		/* Contains a version string.  */
#define NT_ARCH		2		/* Contains an architecture string.  */
#define NT_GO_BUILDID	4		/* Contains GO buildid data.  */

/* Values for notes in non-core files using name "GNU".  */

#define NT_GNU_ABI_TAG		1
#define NT_GNU_HWCAP		2	/* Used by ld.so and kernel vDSO.  */
#define NT_GNU_BUILD_ID		3	/* Generated by ld --build-id.  */
#define NT_GNU_GOLD_VERSION	4	/* Generated by gold.  */
#define NT_GNU_PROPERTY_TYPE_0  5	/* Generated by gcc.  */

#define NT_GNU_BUILD_ATTRIBUTE_OPEN	0x100
#define NT_GNU_BUILD_ATTRIBUTE_FUNC	0x101

#define GNU_BUILD_ATTRIBUTE_TYPE_NUMERIC	'*'
#define GNU_BUILD_ATTRIBUTE_TYPE_STRING		'$'
#define GNU_BUILD_ATTRIBUTE_TYPE_BOOL_TRUE	'+'
#define GNU_BUILD_ATTRIBUTE_TYPE_BOOL_FALSE	'!'

#define GNU_BUILD_ATTRIBUTE_VERSION	1
#define GNU_BUILD_ATTRIBUTE_STACK_PROT	2
#define GNU_BUILD_ATTRIBUTE_RELRO	3
#define GNU_BUILD_ATTRIBUTE_STACK_SIZE	4
#define GNU_BUILD_ATTRIBUTE_TOOL	5
#define GNU_BUILD_ATTRIBUTE_ABI		6
#define GNU_BUILD_ATTRIBUTE_PIC		7
#define GNU_BUILD_ATTRIBUTE_SHORT_ENUM	8

#define NOTE_GNU_PROPERTY_SECTION_NAME	".note.gnu.property"
#define GNU_BUILD_ATTRS_SECTION_NAME	".gnu.build.attributes"

/* Values used in GNU .note.gnu.property notes (NT_GNU_PROPERTY_TYPE_0).  */
#define GNU_PROPERTY_STACK_SIZE			1
#define GNU_PROPERTY_NO_COPY_ON_PROTECTED	2

/* A 4-byte unsigned integer property: A bit is set if it is set in all
   relocatable inputs.  */
#define GNU_PROPERTY_UINT32_AND_LO	0xb0000000
#define GNU_PROPERTY_UINT32_AND_HI	0xb0007fff

/* A 4-byte unsigned integer property: A bit is set if it is set in any
   relocatable inputs.  */
#define GNU_PROPERTY_UINT32_OR_LO	0xb0008000
#define GNU_PROPERTY_UINT32_OR_HI	0xb000ffff

/* The needed properties by the object file.  */
#define GNU_PROPERTY_1_NEEDED		GNU_PROPERTY_UINT32_OR_LO

/* Set if the object file requires canonical function pointers and
   cannot be used with copy relocation.  */
#define GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS	(1U << 0)

/* Processor-specific semantics, lo */
#define GNU_PROPERTY_LOPROC  0xc0000000
/* Processor-specific semantics, hi */
#define GNU_PROPERTY_HIPROC  0xdfffffff
/* Application-specific semantics, lo */
#define GNU_PROPERTY_LOUSER  0xe0000000
/* Application-specific semantics, hi */
#define GNU_PROPERTY_HIUSER  0xffffffff

#define GNU_PROPERTY_X86_COMPAT_ISA_1_USED	0xc0000000
#define GNU_PROPERTY_X86_COMPAT_ISA_1_NEEDED	0xc0000001

#define GNU_PROPERTY_X86_COMPAT_ISA_1_486	(1U << 0)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_586	(1U << 1)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_686	(1U << 2)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSE	(1U << 3)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSE2	(1U << 4)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSE3	(1U << 5)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSSE3	(1U << 6)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSE4_1	(1U << 7)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_SSE4_2	(1U << 8)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX	(1U << 9)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX2	(1U << 10)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512F	(1U << 11)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512CD	(1U << 12)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512ER	(1U << 13)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512PF	(1U << 14)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512VL	(1U << 15)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512DQ	(1U << 16)
#define GNU_PROPERTY_X86_COMPAT_ISA_1_AVX512BW	(1U << 17)

/* A 4-byte unsigned integer property: A bit is set if it is set in all
   relocatable inputs.  */
#define GNU_PROPERTY_X86_UINT32_AND_LO		0xc0000002
#define GNU_PROPERTY_X86_UINT32_AND_HI		0xc0007fff

/* A 4-byte unsigned integer property: A bit is set if it is set in any
   relocatable inputs.  */
#define GNU_PROPERTY_X86_UINT32_OR_LO		0xc0008000
#define GNU_PROPERTY_X86_UINT32_OR_HI		0xc000ffff

/* A 4-byte unsigned integer property: A bit is set if it is set in any
   relocatable inputs and the property is present in all relocatable
   inputs.  */
#define GNU_PROPERTY_X86_UINT32_OR_AND_LO	0xc0010000
#define GNU_PROPERTY_X86_UINT32_OR_AND_HI	0xc0017fff

#define GNU_PROPERTY_X86_FEATURE_1_AND \
  (GNU_PROPERTY_X86_UINT32_AND_LO + 0)

#define GNU_PROPERTY_X86_ISA_1_NEEDED \
  (GNU_PROPERTY_X86_UINT32_OR_LO + 2)
#define GNU_PROPERTY_X86_FEATURE_2_NEEDED \
  (GNU_PROPERTY_X86_UINT32_OR_LO + 1)

#define GNU_PROPERTY_X86_ISA_1_USED \
  (GNU_PROPERTY_X86_UINT32_OR_AND_LO + 2)
#define GNU_PROPERTY_X86_FEATURE_2_USED \
  (GNU_PROPERTY_X86_UINT32_OR_AND_LO + 1)

/* GNU_PROPERTY_X86_ISA_1_BASELINE: CMOV, CX8 (cmpxchg8b), FPU (fld),
   MMX, OSFXSR (fxsave), SCE (syscall), SSE and SSE2.  */
#define GNU_PROPERTY_X86_ISA_1_BASELINE		(1U << 0)
/* GNU_PROPERTY_X86_ISA_1_V2: GNU_PROPERTY_X86_ISA_1_BASELINE,
   CMPXCHG16B (cmpxchg16b), LAHF-SAHF (lahf), POPCNT (popcnt), SSE3,
   SSSE3, SSE4.1 and SSE4.2.  */
#define GNU_PROPERTY_X86_ISA_1_V2		(1U << 1)
/* GNU_PROPERTY_X86_ISA_1_V3: GNU_PROPERTY_X86_ISA_1_V2, AVX, AVX2, BMI1,
   BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE.  */
#define GNU_PROPERTY_X86_ISA_1_V3		(1U << 2)
/* GNU_PROPERTY_X86_ISA_1_V4: GNU_PROPERTY_X86_ISA_1_V3, AVX512F,
   AVX512BW, AVX512CD, AVX512DQ and AVX512VL.  */
#define GNU_PROPERTY_X86_ISA_1_V4		(1U << 3)

#define GNU_PROPERTY_X86_FEATURE_1_IBT		(1U << 0)
#define GNU_PROPERTY_X86_FEATURE_1_SHSTK	(1U << 1)
#define GNU_PROPERTY_X86_FEATURE_1_LAM_U48	(1U << 2)
#define GNU_PROPERTY_X86_FEATURE_1_LAM_U57	(1U << 3)

#define GNU_PROPERTY_X86_FEATURE_2_X86		(1U << 0)
#define GNU_PROPERTY_X86_FEATURE_2_X87		(1U << 1)
#define GNU_PROPERTY_X86_FEATURE_2_MMX		(1U << 2)
#define GNU_PROPERTY_X86_FEATURE_2_XMM		(1U << 3)
#define GNU_PROPERTY_X86_FEATURE_2_YMM		(1U << 4)
#define GNU_PROPERTY_X86_FEATURE_2_ZMM		(1U << 5)
#define GNU_PROPERTY_X86_FEATURE_2_FXSR		(1U << 6)
#define GNU_PROPERTY_X86_FEATURE_2_XSAVE	(1U << 7)
#define GNU_PROPERTY_X86_FEATURE_2_XSAVEOPT	(1U << 8)
#define GNU_PROPERTY_X86_FEATURE_2_XSAVEC	(1U << 9)
#define GNU_PROPERTY_X86_FEATURE_2_TMM		(1U << 10)
#define GNU_PROPERTY_X86_FEATURE_2_MASK		(1U << 11)

#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_NEEDED \
  (GNU_PROPERTY_X86_UINT32_OR_LO + 0)

#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_USED \
  (GNU_PROPERTY_X86_UINT32_OR_AND_LO + 0)

#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_CMOV		(1U << 0)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSE		(1U << 1)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSE2		(1U << 2)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSE3		(1U << 3)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSSE3		(1U << 4)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSE4_1		(1U << 5)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_SSE4_2		(1U << 6)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX		(1U << 7)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX2		(1U << 8)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_FMA		(1U << 9)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512F		(1U << 10)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512CD	(1U << 11)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512ER	(1U << 12)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512PF	(1U << 13)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512VL	(1U << 14)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512DQ	(1U << 15)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512BW	(1U << 16)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_4FMAPS	(1U << 17)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_4VNNIW	(1U << 18)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_BITALG	(1U << 19)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_IFMA	(1U << 20)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_VBMI	(1U << 21)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_VBMI2	(1U << 22)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_VNNI	(1U << 23)
#define GNU_PROPERTY_X86_COMPAT_2_ISA_1_AVX512_BF16	(1U << 24)

/* AArch64 specific GNU PROPERTY.  */
#define GNU_PROPERTY_AARCH64_FEATURE_1_AND	0xc0000000

#define GNU_PROPERTY_AARCH64_FEATURE_1_BTI	(1U << 0)
#define GNU_PROPERTY_AARCH64_FEATURE_1_PAC	(1U << 1)

/* Values used in GNU .note.ABI-tag notes (NT_GNU_ABI_TAG).  */
#define GNU_ABI_TAG_LINUX	0
#define GNU_ABI_TAG_HURD	1
#define GNU_ABI_TAG_SOLARIS	2
#define GNU_ABI_TAG_FREEBSD	3
#define GNU_ABI_TAG_NETBSD	4
#define GNU_ABI_TAG_SYLLABLE	5
#define GNU_ABI_TAG_NACL	6

/* Values for NetBSD .note.netbsd.ident notes.  Note name is "NetBSD".  */

#define NT_NETBSD_IDENT		1
#define NT_NETBSD_MARCH		5

/* Values for NetBSD .note.netbsd.ident notes.  Note name is "PaX".  */
#define NT_NETBSD_PAX		3
#define NT_NETBSD_PAX_MPROTECT		0x01	/* Force enable Mprotect.  */
#define NT_NETBSD_PAX_NOMPROTECT	0x02	/* Force disable Mprotect.  */
#define NT_NETBSD_PAX_GUARD		0x04	/* Force enable Segvguard.  */
#define NT_NETBSD_PAX_NOGUARD		0x08	/* Force disable Segvguard.  */
#define NT_NETBSD_PAX_ASLR		0x10	/* Force enable ASLR.  */
#define NT_NETBSD_PAX_NOASLR		0x20	/* Force disable ASLR.  */

/* Values for OpenBSD .note.openbsd.ident notes.  Note name is "OpenBSD".  */

#define NT_OPENBSD_IDENT	1

/* Values for FreeBSD .note.ABI-tag notes.  Note name is "FreeBSD".  */

#define NT_FREEBSD_ABI_TAG	1

/* Values for FDO .note.package notes as defined on https://systemd.io/COREDUMP_PACKAGE_METADATA/  */
#define FDO_PACKAGING_METADATA	0xcafe1a7e

/* These three macros disassemble and assemble a symbol table st_info field,
   which contains the symbol binding and symbol type.  The STB_ and STT_
   defines identify the binding and type.  */

#define ELF_ST_BIND(val)		(((unsigned int)(val)) >> 4)
#define ELF_ST_TYPE(val)		((val) & 0xF)
#define ELF_ST_INFO(bind,type)		(((bind) << 4) + ((type) & 0xF))

/* The 64bit and 32bit versions of these macros are identical, but
   the ELF spec defines them, so here they are.  */
#define ELF32_ST_BIND  ELF_ST_BIND
#define ELF32_ST_TYPE  ELF_ST_TYPE
#define ELF32_ST_INFO  ELF_ST_INFO
#define ELF64_ST_BIND  ELF_ST_BIND
#define ELF64_ST_TYPE  ELF_ST_TYPE
#define ELF64_ST_INFO  ELF_ST_INFO

/* This macro disassembles and assembles a symbol's visibility into
   the st_other field.  The STV_ defines specify the actual visibility.  */

#define ELF_ST_VISIBILITY(v)		((v) & 0x3)
/* The remaining bits in the st_other field are not currently used.
   They should be set to zero.  */

#define ELF32_ST_VISIBILITY  ELF_ST_VISIBILITY
#define ELF64_ST_VISIBILITY  ELF_ST_VISIBILITY


#define STN_UNDEF	0		/* Undefined symbol index */

#define STB_LOCAL	0		/* Symbol not visible outside obj */
#define STB_GLOBAL	1		/* Symbol visible outside obj */
#define STB_WEAK	2		/* Like globals, lower precedence */
#define STB_LOOS	10		/* OS-specific semantics */
#define STB_GNU_UNIQUE	10		/* Symbol is unique in namespace */
#define STB_HIOS	12		/* OS-specific semantics */
#define STB_LOPROC	13		/* Processor-specific semantics */
#define STB_HIPROC	15		/* Processor-specific semantics */

#define STT_NOTYPE	0		/* Symbol type is unspecified */
#define STT_OBJECT	1		/* Symbol is a data object */
#define STT_FUNC	2		/* Symbol is a code object */
#define STT_SECTION	3		/* Symbol associated with a section */
#define STT_FILE	4		/* Symbol gives a file name */
#define STT_COMMON	5		/* An uninitialised common block */
#define STT_TLS		6		/* Thread local data object */
#define STT_RELC	8		/* Complex relocation expression */
#define STT_SRELC	9		/* Signed Complex relocation expression */
#define STT_LOOS	10		/* OS-specific semantics */
#define STT_GNU_IFUNC	10		/* Symbol is an indirect code object */
#define STT_HIOS	12		/* OS-specific semantics */
#define STT_LOPROC	13		/* Processor-specific semantics */
#define STT_HIPROC	15		/* Processor-specific semantics */

/* The following constants control how a symbol may be accessed once it has
   become part of an executable or shared library.  */

#define STV_DEFAULT	0		/* Visibility is specified by binding type */
#define STV_INTERNAL	1		/* OS specific version of STV_HIDDEN */
#define STV_HIDDEN	2		/* Can only be seen inside currect component */
#define STV_PROTECTED	3		/* Treat as STB_LOCAL inside current component */

/* Relocation info handling macros.  */

#define ELF32_R_SYM(i)		((i) >> 8)
#define ELF32_R_TYPE(i)		((i) & 0xff)
#define ELF32_R_INFO(s,t)	(((unsigned) (s) << 8) + ((t) & 0xff))

#define ELF64_R_SYM(i)		((i) >> 32)
#define ELF64_R_TYPE(i)		((i) & 0xffffffff)
#define ELF64_R_INFO(s,t)	(((bfd_vma) (s) << 31 << 1) + (bfd_vma) (t))

/* Dynamic section tags.  */

#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH	15
#define DT_SYMBOLIC	16
#define DT_REL		17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23
#define DT_BIND_NOW	24
#define DT_INIT_ARRAY	25
#define DT_FINI_ARRAY	26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_RUNPATH	29
#define DT_FLAGS	30

/* Values in the range [DT_ENCODING, DT_LOOS) use d_un.d_ptr if the
   value is even, d_un.d_val if odd.  */
#define DT_ENCODING	32
#define DT_PREINIT_ARRAY   32
#define DT_PREINIT_ARRAYSZ 33
#define DT_SYMTAB_SHNDX    34
#define DT_RELRSZ	35
#define DT_RELR		36
#define DT_RELRENT	37

/* Note, the Oct 4, 1999 draft of the ELF ABI changed the values
   for DT_LOOS and DT_HIOS.  Some implementations however, use
   values outside of the new range (see below).	 */
#define OLD_DT_LOOS	0x60000000
#define DT_LOOS		0x6000000d
#define DT_HIOS		0x6ffff000
#define OLD_DT_HIOS	0x6fffffff

#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff

/* The next 2 dynamic tag ranges, integer value range (DT_VALRNGLO to
   DT_VALRNGHI) and virtual address range (DT_ADDRRNGLO to DT_ADDRRNGHI),
   are used on Solaris.  We support them everywhere.  Note these values
   lie outside of the (new) range for OS specific values.  This is a
   deliberate special case and we maintain it for backwards compatability.
 */
#define DT_VALRNGLO	0x6ffffd00
#define DT_GNU_FLAGS_1  0x6ffffdf4
#define DT_GNU_PRELINKED 0x6ffffdf5
#define DT_GNU_CONFLICTSZ 0x6ffffdf6
#define DT_GNU_LIBLISTSZ 0x6ffffdf7
#define DT_CHECKSUM	0x6ffffdf8
#define DT_PLTPADSZ	0x6ffffdf9
#define DT_MOVEENT	0x6ffffdfa
#define DT_MOVESZ	0x6ffffdfb
#define DT_FEATURE	0x6ffffdfc
#define DT_POSFLAG_1	0x6ffffdfd
#define DT_SYMINSZ	0x6ffffdfe
#define DT_SYMINENT	0x6ffffdff
#define DT_VALRNGHI	0x6ffffdff

#define DT_ADDRRNGLO	0x6ffffe00
#define DT_GNU_HASH	0x6ffffef5
#define DT_TLSDESC_PLT	0x6ffffef6
#define DT_TLSDESC_GOT	0x6ffffef7
#define DT_GNU_CONFLICT	0x6ffffef8
#define DT_GNU_LIBLIST	0x6ffffef9
#define DT_CONFIG	0x6ffffefa
#define DT_DEPAUDIT	0x6ffffefb
#define DT_AUDIT	0x6ffffefc
#define DT_PLTPAD	0x6ffffefd
#define DT_MOVETAB	0x6ffffefe
#define DT_SYMINFO	0x6ffffeff
#define DT_ADDRRNGHI	0x6ffffeff

#define DT_RELACOUNT	0x6ffffff9
#define DT_RELCOUNT	0x6ffffffa
#define DT_FLAGS_1	0x6ffffffb
#define DT_VERDEF	0x6ffffffc
#define DT_VERDEFNUM	0x6ffffffd
#define DT_VERNEED	0x6ffffffe
#define DT_VERNEEDNUM	0x6fffffff

/* This tag is a GNU extension to the Solaris version scheme.  */
#define DT_VERSYM	0x6ffffff0

#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff

/* These section tags are used on Solaris.  We support them
   everywhere, and hope they do not conflict.  */

#define DT_AUXILIARY	0x7ffffffd
#define DT_USED		0x7ffffffe
#define DT_FILTER	0x7fffffff


/* Values used in DT_FEATURE .dynamic entry.  */
#define DTF_1_PARINIT	0x00000001
/* From

   http://docs.sun.com:80/ab2/coll.45.13/LLM/@Ab2PageView/21165?Ab2Lang=C&Ab2Enc=iso-8859-1

   DTF_1_CONFEXP is the same as DTF_1_PARINIT. It is a typo. The value
   defined here is the same as the one in <sys/link.h> on Solaris 8.  */
#define DTF_1_CONFEXP	0x00000002

/* Flag values used in the DT_POSFLAG_1 .dynamic entry.	 */
#define DF_P1_LAZYLOAD	0x00000001
#define DF_P1_GROUPPERM	0x00000002

/* Flag value in the DT_GNU_FLAGS_1 /dynamic entry.  */
#define DF_GNU_1_UNIQUE 0x00000001

/* Flag value in in the DT_FLAGS_1 .dynamic entry.  */
#define DF_1_NOW	0x00000001
#define DF_1_GLOBAL	0x00000002
#define DF_1_GROUP	0x00000004
#define DF_1_NODELETE	0x00000008
#define DF_1_LOADFLTR	0x00000010
#define DF_1_INITFIRST	0x00000020
#define DF_1_NOOPEN	0x00000040
#define DF_1_ORIGIN	0x00000080
#define DF_1_DIRECT	0x00000100
#define DF_1_TRANS	0x00000200
#define DF_1_INTERPOSE	0x00000400
#define DF_1_NODEFLIB	0x00000800
#define DF_1_NODUMP	0x00001000
#define DF_1_CONFALT	0x00002000
#define DF_1_ENDFILTEE	0x00004000
#define	DF_1_DISPRELDNE	0x00008000
#define	DF_1_DISPRELPND	0x00010000
#define	DF_1_NODIRECT	0x00020000
#define	DF_1_IGNMULDEF	0x00040000
#define	DF_1_NOKSYMS	0x00080000
#define	DF_1_NOHDR	0x00100000
#define	DF_1_EDITED	0x00200000
#define	DF_1_NORELOC	0x00400000
#define	DF_1_SYMINTPOSE	0x00800000
#define	DF_1_GLOBAUDIT	0x01000000
#define	DF_1_SINGLETON	0x02000000
#define	DF_1_STUB	0x04000000
#define	DF_1_PIE	0x08000000
#define	DF_1_KMOD	0x10000000
#define	DF_1_WEAKFILTER	0x20000000
#define	DF_1_NOCOMMON	0x40000000

/* Flag values for the DT_FLAGS entry.	*/
#define DF_ORIGIN	(1 << 0)
#define DF_SYMBOLIC	(1 << 1)
#define DF_TEXTREL	(1 << 2)
#define DF_BIND_NOW	(1 << 3)
#define DF_STATIC_TLS	(1 << 4)

/* These constants are used for the version number of a Elf32_Verdef
   structure.  */

#define VER_DEF_NONE		0
#define VER_DEF_CURRENT		1

/* These constants appear in the vd_flags field of a Elf32_Verdef
   structure.

   Cf. the Solaris Linker and Libraries Guide, Ch. 7, Object File Format,
   Versioning Sections, for a description:

   http://docs.sun.com/app/docs/doc/819-0690/chapter6-93046?l=en&a=view  */

#define VER_FLG_BASE		0x1
#define VER_FLG_WEAK		0x2
#define VER_FLG_INFO		0x4

/* These special constants can be found in an Elf32_Versym field.  */

#define VER_NDX_LOCAL		0
#define VER_NDX_GLOBAL		1

/* These constants are used for the version number of a Elf32_Verneed
   structure.  */

#define VER_NEED_NONE		0
#define VER_NEED_CURRENT	1

/* This flag appears in a Versym structure.  It means that the symbol
   is hidden, and is only visible with an explicit version number.
   This is a GNU extension.  */

#define VERSYM_HIDDEN		0x8000

/* This is the mask for the rest of the Versym information.  */

#define VERSYM_VERSION		0x7fff

/* This is a special token which appears as part of a symbol name.  It
   indictes that the rest of the name is actually the name of a
   version node, and is not part of the actual name.  This is a GNU
   extension.  For example, the symbol name `stat@ver2' is taken to
   mean the symbol `stat' in version `ver2'.  */

#define ELF_VER_CHR	'@'

/* Possible values for si_boundto.  */

#define SYMINFO_BT_SELF		0xffff	/* Symbol bound to self */
#define SYMINFO_BT_PARENT	0xfffe	/* Symbol bound to parent */
#define SYMINFO_BT_LOWRESERVE	0xff00	/* Beginning of reserved entries */

/* Possible bitmasks for si_flags.  */

#define SYMINFO_FLG_DIRECT	0x0001	/* Direct bound symbol */
#define SYMINFO_FLG_PASSTHRU	0x0002	/* Pass-thru symbol for translator */
#define SYMINFO_FLG_COPY	0x0004	/* Symbol is a copy-reloc */
#define SYMINFO_FLG_LAZYLOAD	0x0008	/* Symbol bound to object to be lazy loaded */

/* Syminfo version values.  */

#define SYMINFO_NONE		0
#define SYMINFO_CURRENT		1
#define SYMINFO_NUM		2

/* Section Group Flags.	 */

#define GRP_COMDAT		0x1	/* A COMDAT group */
#define GRP_MASKOS 	 0x0ff00000	/* Bits in this range reserved for OS specific use.  */
#define GRP_MASKPROC 	 0xf0000000	/* Bits in this range reserved for processor use.  */

/* Auxv a_type values.  */

#define AT_NULL		0		/* End of vector */
#define AT_IGNORE	1		/* Entry should be ignored */
#define AT_EXECFD	2		/* File descriptor of program */
#define AT_PHDR		3		/* Program headers for program */
#define AT_PHENT	4		/* Size of program header entry */
#define AT_PHNUM	5		/* Number of program headers */
#define AT_PAGESZ	6		/* System page size */
#define AT_BASE		7		/* Base address of interpreter */
#define AT_FLAGS	8		/* Flags */
#define AT_ENTRY	9		/* Entry point of program */
#define AT_NOTELF	10		/* Program is not ELF */
#define AT_UID		11		/* Real uid */
#define AT_EUID		12		/* Effective uid */
#define AT_GID		13		/* Real gid */
#define AT_EGID		14		/* Effective gid */
#define AT_CLKTCK	17		/* Frequency of times() */
#define AT_PLATFORM	15		/* String identifying platform.  */
#define AT_HWCAP	16		/* Machine dependent hints about
					   processor capabilities.  */
#define AT_FPUCW	18		/* Used FPU control word.  */
#define AT_DCACHEBSIZE	19		/* Data cache block size.  */
#define AT_ICACHEBSIZE	20		/* Instruction cache block size.  */
#define AT_UCACHEBSIZE	21		/* Unified cache block size.  */
#define AT_IGNOREPPC	22		/* Entry should be ignored */
#define	AT_SECURE	23		/* Boolean, was exec setuid-like?  */
#define AT_BASE_PLATFORM 24		/* String identifying real platform,
					   may differ from AT_PLATFORM.  */
#define AT_RANDOM	25		/* Address of 16 random bytes.  */
#define AT_HWCAP2	26		/* Extension of AT_HWCAP.  */
#define AT_EXECFN	31		/* Filename of executable.  */
/* Pointer to the global system page used for system calls and other
   nice things.  */
#define AT_SYSINFO	32
#define AT_SYSINFO_EHDR	33 /* Pointer to ELF header of system-supplied DSO.  */

/* More complete cache descriptions than AT_[DIU]CACHEBSIZE.  If the
   value is -1, then the cache doesn't exist.  Otherwise:

   bit 0-3:  Cache set-associativity; 0 means fully associative.
   bit 4-7:  Log2 of cacheline size.
   bit 8-31:  Size of the entire cache >> 8.  */

#define AT_L1I_CACHESHAPE 34
#define AT_L1D_CACHESHAPE 35
#define AT_L2_CACHESHAPE  36
#define AT_L3_CACHESHAPE  37

/* Shapes of the caches, with more room to describe them.
   *GEOMETRY are comprised of cache line size in bytes in the bottom 16 bits
   and the cache associativity in the next 16 bits.  */
#define AT_L1I_CACHESIZE	40
#define AT_L1I_CACHEGEOMETRY	41
#define AT_L1D_CACHESIZE	42
#define AT_L1D_CACHEGEOMETRY	43
#define AT_L2_CACHESIZE		44
#define AT_L2_CACHEGEOMETRY	45
#define AT_L3_CACHESIZE		46
#define AT_L3_CACHEGEOMETRY	47

#define AT_MINSIGSTKSZ		51 /* Stack needed for signal delivery
				      (AArch64).  */

#define AT_FREEBSD_EXECPATH     15      /* Path to the executable. */
#define AT_FREEBSD_CANARY       16      /* Canary for SSP. */
#define AT_FREEBSD_CANARYLEN    17      /* Length of the canary. */
#define AT_FREEBSD_OSRELDATE    18      /* OSRELDATE. */
#define AT_FREEBSD_NCPUS        19      /* Number of CPUs. */
#define AT_FREEBSD_PAGESIZES    20      /* Pagesizes. */
#define AT_FREEBSD_PAGESIZESLEN 21      /* Number of pagesizes. */
#define AT_FREEBSD_TIMEKEEP     22      /* Pointer to timehands. */
#define AT_FREEBSD_STACKPROT    23      /* Initial stack protection. */
#define AT_FREEBSD_EHDRFLAGS    24      /* e_flags field from ELF header. */
#define AT_FREEBSD_HWCAP        25      /* CPU feature flags. */
#define AT_FREEBSD_HWCAP2       26      /* CPU feature flags 2. */
#define AT_FREEBSD_BSDFLAGS     27      /* ELF BSD Flags. */
#define AT_FREEBSD_ARGC         28      /* Argument count. */
#define AT_FREEBSD_ARGV         29      /* Argument vector. */
#define AT_FREEBSD_ENVC         30      /* Environment count. */
#define AT_FREEBSD_ENVV         31      /* Environment vvector. */
#define AT_FREEBSD_PS_STRINGS   32      /* struct ps_strings. */
#define AT_FREEBSD_FXRNG        33      /* Pointer to root RNG seed version. */
#define AT_FREEBSD_KPRELOAD     34      /* Base of vdso. */
#define AT_FREEBSD_USRSTACKBASE 35      /* Top of user stack. */
#define AT_FREEBSD_USRSTACKLIM  36      /* Grow limit of user stack. */

#define AT_SUN_UID      2000    /* Effective user ID.  */
#define AT_SUN_RUID     2001    /* Real user ID.  */
#define AT_SUN_GID      2002    /* Effective group ID.  */
#define AT_SUN_RGID     2003    /* Real group ID.  */
#define AT_SUN_LDELF    2004    /* Dynamic linker's ELF header.  */
#define AT_SUN_LDSHDR   2005    /* Dynamic linker's section headers.  */
#define AT_SUN_LDNAME   2006    /* String giving name of dynamic linker.  */
#define AT_SUN_LPAGESZ  2007    /* Large pagesize.   */
#define AT_SUN_PLATFORM 2008    /* Platform name string.  */
#define AT_SUN_CAP_HW1	2009	/* Machine dependent hints about
				   processor capabilities.  */
#ifndef AT_SUN_HWCAP
#define AT_SUN_HWCAP	AT_SUN_CAP_HW1 /* For backward compat only.  */
#endif
#define AT_SUN_IFLUSH   2010    /* Should flush icache? */
#define AT_SUN_CPU      2011    /* CPU name string.  */
#define AT_SUN_EMUL_ENTRY 2012	/* COFF entry point address.  */
#define AT_SUN_EMUL_EXECFD 2013	/* COFF executable file descriptor.  */
#define AT_SUN_EXECNAME 2014    /* Canonicalized file name given to execve.  */
#define AT_SUN_MMU      2015    /* String for name of MMU module.   */
#define AT_SUN_LDDATA   2016    /* Dynamic linker's data segment address.  */
#define AT_SUN_AUXFLAGS	2017	/* AF_SUN_ flags passed from the kernel.  */
#define	AT_SUN_EMULATOR	2018	/* Name of emulation binary for runtime
				   linker.  */
#define	AT_SUN_BRANDNAME 2019	/* Name of brand library.  */
#define	AT_SUN_BRAND_AUX1 2020	/* Aux vectors for brand modules.  */
#define	AT_SUN_BRAND_AUX2 2021
#define	AT_SUN_BRAND_AUX3 2022
#define	AT_SUN_CAP_HW2	2023	/* Extension of AT_SUN_CAP_HW1.  */

#endif /* _ELF_COMMON_H */
