#objdump: -d --prefix-addresses
#name: nds32 usr-spe-reg instructions
#as:

# Test user specail register instructions

.*:     file format .*

Disassembly of section .text:
0+0000 <[^>]*> mfusr	\$r0, \$d0.lo
0+0004 <[^>]*> mfusr	\$r0, \$d0.hi
0+0008 <[^>]*> mfusr	\$r0, \$d1.lo
0+000c <[^>]*> mfusr	\$r0, \$d1.hi
0+0010 <[^>]*> mfusr	\$r0, \$pc
0+0014 <[^>]*> mfusr	\$r0, \$dma_cfg
0+0018 <[^>]*> mfusr	\$r0, \$dma_gcsw
0+001c <[^>]*> mfusr	\$r0, \$dma_chnsel
0+0020 <[^>]*> mfusr	\$r0, \$dma_act
0+0024 <[^>]*> mfusr	\$r0, \$dma_setup
0+0028 <[^>]*> mfusr	\$r0, \$dma_isaddr
0+002c <[^>]*> mfusr	\$r0, \$dma_esaddr
0+0030 <[^>]*> mfusr	\$r0, \$dma_tcnt
0+0034 <[^>]*> mfusr	\$r0, \$dma_status
0+0038 <[^>]*> mfusr	\$r0, \$dma_2dset
0+003c <[^>]*> mfusr	\$r0, \$dma_2dsctl
0+0040 <[^>]*> mfusr	\$r0, \$pfmc0
0+0044 <[^>]*> mfusr	\$r0, \$pfmc1
0+0048 <[^>]*> mfusr	\$r0, \$pfmc2
0+004c <[^>]*> mfusr	\$r0, \$pfm_ctl
