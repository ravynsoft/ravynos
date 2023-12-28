#objdump: -dr --prefix-address --show-raw-insn
#as: -32 -I$srcdir/$subdir
#name: MIPS16 ISA subset disassembly
#source: mips16-sub.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 3b41      	.short	0x3b41
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 3b42      	.short	0x3b42
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 3b43      	.short	0x3b43
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 3b44      	.short	0x3b44
[0-9a-f]+ <[^>]*> 3b41      	.short	0x3b41
[0-9a-f]+ <[^>]*> 3b42      	.short	0x3b42
[0-9a-f]+ <[^>]*> 3b44      	.short	0x3b44
[0-9a-f]+ <[^>]*> 3b48      	.short	0x3b48
[0-9a-f]+ <[^>]*> 3b50      	.short	0x3b50
[0-9a-f]+ <[^>]*> f100      	extend	0x100
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f200      	extend	0x200
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f400      	extend	0x400
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b5f      	.short	0x3b5f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b5e      	.short	0x3b5e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b5d      	.short	0x3b5d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b5c      	.short	0x3b5c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b58      	.short	0x3b58
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b50      	.short	0x3b50
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> 3b40      	.short	0x3b40
[0-9a-f]+ <[^>]*> f7bf      	extend	0x7bf
[0-9a-f]+ <[^>]*> fc40      	.short	0xfc40
[0-9a-f]+ <[^>]*> f6a0      	extend	0x6a0
[0-9a-f]+ <[^>]*> fc54      	.short	0xfc54
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> fc40      	.short	0xfc40
[0-9a-f]+ <[^>]*> f0c1      	extend	0xc1
[0-9a-f]+ <[^>]*> fc40      	.short	0xfc40
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f841      	.short	0xf841
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f842      	.short	0xf842
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f843      	.short	0xf843
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f844      	.short	0xf844
[0-9a-f]+ <[^>]*> f841      	.short	0xf841
[0-9a-f]+ <[^>]*> f842      	.short	0xf842
[0-9a-f]+ <[^>]*> f844      	.short	0xf844
[0-9a-f]+ <[^>]*> f848      	.short	0xf848
[0-9a-f]+ <[^>]*> f850      	.short	0xf850
[0-9a-f]+ <[^>]*> f100      	extend	0x100
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f200      	extend	0x200
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f400      	extend	0x400
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f85f      	.short	0xf85f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f85e      	.short	0xf85e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f85d      	.short	0xf85d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f85c      	.short	0xf85c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f858      	.short	0xf858
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f850      	.short	0xf850
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> f840      	.short	0xf840
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> bb41      	.short	0xbb41
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> bb42      	.short	0xbb42
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> bb43      	.short	0xbb43
[0-9a-f]+ <[^>]*> bb41      	.short	0xbb41
[0-9a-f]+ <[^>]*> bb42      	.short	0xbb42
[0-9a-f]+ <[^>]*> bb44      	.short	0xbb44
[0-9a-f]+ <[^>]*> bb48      	.short	0xbb48
[0-9a-f]+ <[^>]*> bb50      	.short	0xbb50
[0-9a-f]+ <[^>]*> f080      	extend	0x80
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f100      	extend	0x100
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f200      	extend	0x200
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f400      	extend	0x400
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb5f      	.short	0xbb5f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb5e      	.short	0xbb5e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb5d      	.short	0xbb5d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb5c      	.short	0xbb5c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb58      	.short	0xbb58
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb50      	.short	0xbb50
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> bb40      	.short	0xbb40
[0-9a-f]+ <[^>]*> 9b40      	lw	v0,0\(v1\)
[0-9a-f]+ <[^>]*> f000 9b41 	lw	v0,1\(v1\)
[0-9a-f]+ <[^>]*> f000 9b42 	lw	v0,2\(v1\)
[0-9a-f]+ <[^>]*> f000 9b43 	lw	v0,3\(v1\)
[0-9a-f]+ <[^>]*> 9b41      	lw	v0,4\(v1\)
[0-9a-f]+ <[^>]*> 9b42      	lw	v0,8\(v1\)
[0-9a-f]+ <[^>]*> 9b44      	lw	v0,16\(v1\)
[0-9a-f]+ <[^>]*> 9b48      	lw	v0,32\(v1\)
[0-9a-f]+ <[^>]*> 9b50      	lw	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 9b40 	lw	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 9b40 	lw	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 9b40 	lw	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 9b40 	lw	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 9b40 	lw	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b5f 	lw	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b5e 	lw	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b5d 	lw	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b5c 	lw	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b58 	lw	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b50 	lw	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff 9b40 	lw	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df 9b40 	lw	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f 9b40 	lw	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f 9b40 	lw	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f 9b40 	lw	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f 9b40 	lw	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f 9b40 	lw	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> f67f b20c 	lw	v0,00000000 <data1>
[0-9a-f]+ <[^>]*> f580 b204 	lw	v0,0000071c <data2>
[0-9a-f]+ <[^>]*> f6c0 b20c 	lw	v0,00000868 <bar>
[0-9a-f]+ <[^>]*> f780 b210 	lw	v0,00000930 <iuux>
[0-9a-f]+ <[^>]*> 9200      	lw	v0,0\(sp\)
[0-9a-f]+ <[^>]*> f000 9201 	lw	v0,1\(sp\)
[0-9a-f]+ <[^>]*> f000 9202 	lw	v0,2\(sp\)
[0-9a-f]+ <[^>]*> f000 9203 	lw	v0,3\(sp\)
[0-9a-f]+ <[^>]*> 9201      	lw	v0,4\(sp\)
[0-9a-f]+ <[^>]*> 9202      	lw	v0,8\(sp\)
[0-9a-f]+ <[^>]*> 9204      	lw	v0,16\(sp\)
[0-9a-f]+ <[^>]*> 9208      	lw	v0,32\(sp\)
[0-9a-f]+ <[^>]*> 9210      	lw	v0,64\(sp\)
[0-9a-f]+ <[^>]*> 9220      	lw	v0,128\(sp\)
[0-9a-f]+ <[^>]*> 9240      	lw	v0,256\(sp\)
[0-9a-f]+ <[^>]*> 9280      	lw	v0,512\(sp\)
[0-9a-f]+ <[^>]*> f400 9200 	lw	v0,1024\(sp\)
[0-9a-f]+ <[^>]*> f001 9200 	lw	v0,2048\(sp\)
[0-9a-f]+ <[^>]*> f7ff 921f 	lw	v0,-1\(sp\)
[0-9a-f]+ <[^>]*> f7ff 921e 	lw	v0,-2\(sp\)
[0-9a-f]+ <[^>]*> f7ff 921d 	lw	v0,-3\(sp\)
[0-9a-f]+ <[^>]*> f7ff 921c 	lw	v0,-4\(sp\)
[0-9a-f]+ <[^>]*> f7ff 9218 	lw	v0,-8\(sp\)
[0-9a-f]+ <[^>]*> f7ff 9210 	lw	v0,-16\(sp\)
[0-9a-f]+ <[^>]*> f7ff 9200 	lw	v0,-32\(sp\)
[0-9a-f]+ <[^>]*> f7df 9200 	lw	v0,-64\(sp\)
[0-9a-f]+ <[^>]*> f79f 9200 	lw	v0,-128\(sp\)
[0-9a-f]+ <[^>]*> f71f 9200 	lw	v0,-256\(sp\)
[0-9a-f]+ <[^>]*> f61f 9200 	lw	v0,-512\(sp\)
[0-9a-f]+ <[^>]*> f41f 9200 	lw	v0,-1024\(sp\)
[0-9a-f]+ <[^>]*> f01f 9200 	lw	v0,-2048\(sp\)
[0-9a-f]+ <[^>]*> 8b40      	lh	v0,0\(v1\)
[0-9a-f]+ <[^>]*> f000 8b41 	lh	v0,1\(v1\)
[0-9a-f]+ <[^>]*> 8b41      	lh	v0,2\(v1\)
[0-9a-f]+ <[^>]*> f000 8b43 	lh	v0,3\(v1\)
[0-9a-f]+ <[^>]*> 8b42      	lh	v0,4\(v1\)
[0-9a-f]+ <[^>]*> 8b44      	lh	v0,8\(v1\)
[0-9a-f]+ <[^>]*> 8b48      	lh	v0,16\(v1\)
[0-9a-f]+ <[^>]*> 8b50      	lh	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 8b40 	lh	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 8b40 	lh	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 8b40 	lh	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 8b40 	lh	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 8b40 	lh	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 8b40 	lh	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b5f 	lh	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b5e 	lh	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b5d 	lh	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b5c 	lh	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b58 	lh	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b50 	lh	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8b40 	lh	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df 8b40 	lh	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f 8b40 	lh	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f 8b40 	lh	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f 8b40 	lh	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f 8b40 	lh	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f 8b40 	lh	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> ab40      	lhu	v0,0\(v1\)
[0-9a-f]+ <[^>]*> f000 ab41 	lhu	v0,1\(v1\)
[0-9a-f]+ <[^>]*> ab41      	lhu	v0,2\(v1\)
[0-9a-f]+ <[^>]*> f000 ab43 	lhu	v0,3\(v1\)
[0-9a-f]+ <[^>]*> ab42      	lhu	v0,4\(v1\)
[0-9a-f]+ <[^>]*> ab44      	lhu	v0,8\(v1\)
[0-9a-f]+ <[^>]*> ab48      	lhu	v0,16\(v1\)
[0-9a-f]+ <[^>]*> ab50      	lhu	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 ab40 	lhu	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 ab40 	lhu	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 ab40 	lhu	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 ab40 	lhu	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 ab40 	lhu	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 ab40 	lhu	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab5f 	lhu	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab5e 	lhu	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab5d 	lhu	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab5c 	lhu	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab58 	lhu	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab50 	lhu	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff ab40 	lhu	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df ab40 	lhu	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f ab40 	lhu	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f ab40 	lhu	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f ab40 	lhu	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f ab40 	lhu	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f ab40 	lhu	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> 8340      	lb	v0,0\(v1\)
[0-9a-f]+ <[^>]*> 8341      	lb	v0,1\(v1\)
[0-9a-f]+ <[^>]*> 8342      	lb	v0,2\(v1\)
[0-9a-f]+ <[^>]*> 8343      	lb	v0,3\(v1\)
[0-9a-f]+ <[^>]*> 8344      	lb	v0,4\(v1\)
[0-9a-f]+ <[^>]*> 8348      	lb	v0,8\(v1\)
[0-9a-f]+ <[^>]*> 8350      	lb	v0,16\(v1\)
[0-9a-f]+ <[^>]*> f020 8340 	lb	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 8340 	lb	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 8340 	lb	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 8340 	lb	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 8340 	lb	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 8340 	lb	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 8340 	lb	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff 835f 	lb	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff 835e 	lb	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff 835d 	lb	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff 835c 	lb	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8358 	lb	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8350 	lb	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff 8340 	lb	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df 8340 	lb	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f 8340 	lb	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f 8340 	lb	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f 8340 	lb	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f 8340 	lb	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f 8340 	lb	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> a340      	lbu	v0,0\(v1\)
[0-9a-f]+ <[^>]*> a341      	lbu	v0,1\(v1\)
[0-9a-f]+ <[^>]*> a342      	lbu	v0,2\(v1\)
[0-9a-f]+ <[^>]*> a343      	lbu	v0,3\(v1\)
[0-9a-f]+ <[^>]*> a344      	lbu	v0,4\(v1\)
[0-9a-f]+ <[^>]*> a348      	lbu	v0,8\(v1\)
[0-9a-f]+ <[^>]*> a350      	lbu	v0,16\(v1\)
[0-9a-f]+ <[^>]*> f020 a340 	lbu	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 a340 	lbu	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 a340 	lbu	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 a340 	lbu	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 a340 	lbu	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 a340 	lbu	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 a340 	lbu	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff a35f 	lbu	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff a35e 	lbu	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff a35d 	lbu	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff a35c 	lbu	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff a358 	lbu	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff a350 	lbu	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff a340 	lbu	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df a340 	lbu	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f a340 	lbu	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f a340 	lbu	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f a340 	lbu	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f a340 	lbu	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f a340 	lbu	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 7b41      	.short	0x7b41
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 7b42      	.short	0x7b42
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 7b43      	.short	0x7b43
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 7b44      	.short	0x7b44
[0-9a-f]+ <[^>]*> 7b41      	.short	0x7b41
[0-9a-f]+ <[^>]*> 7b42      	.short	0x7b42
[0-9a-f]+ <[^>]*> 7b44      	.short	0x7b44
[0-9a-f]+ <[^>]*> 7b48      	.short	0x7b48
[0-9a-f]+ <[^>]*> 7b50      	.short	0x7b50
[0-9a-f]+ <[^>]*> f100      	extend	0x100
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f200      	extend	0x200
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f400      	extend	0x400
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b5f      	.short	0x7b5f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b5e      	.short	0x7b5e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b5d      	.short	0x7b5d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b5c      	.short	0x7b5c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b58      	.short	0x7b58
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b50      	.short	0x7b50
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> 7b40      	.short	0x7b40
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f941      	.short	0xf941
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f942      	.short	0xf942
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f943      	.short	0xf943
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> f944      	.short	0xf944
[0-9a-f]+ <[^>]*> f941      	.short	0xf941
[0-9a-f]+ <[^>]*> f942      	.short	0xf942
[0-9a-f]+ <[^>]*> f944      	.short	0xf944
[0-9a-f]+ <[^>]*> f948      	.short	0xf948
[0-9a-f]+ <[^>]*> f950      	.short	0xf950
[0-9a-f]+ <[^>]*> f100      	extend	0x100
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f200      	extend	0x200
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f400      	extend	0x400
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f95f      	.short	0xf95f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f95e      	.short	0xf95e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f95d      	.short	0xf95d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f95c      	.short	0xf95c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f958      	.short	0xf958
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f950      	.short	0xf950
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> f940      	.short	0xf940
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> fa01      	.short	0xfa01
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> fa02      	.short	0xfa02
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> fa03      	.short	0xfa03
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> fa04      	.short	0xfa04
[0-9a-f]+ <[^>]*> fa01      	.short	0xfa01
[0-9a-f]+ <[^>]*> fa02      	.short	0xfa02
[0-9a-f]+ <[^>]*> fa04      	.short	0xfa04
[0-9a-f]+ <[^>]*> fa08      	.short	0xfa08
[0-9a-f]+ <[^>]*> fa10      	.short	0xfa10
[0-9a-f]+ <[^>]*> fa20      	.short	0xfa20
[0-9a-f]+ <[^>]*> fa40      	.short	0xfa40
[0-9a-f]+ <[^>]*> fa80      	.short	0xfa80
[0-9a-f]+ <[^>]*> f001      	extend	0x1
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa1f      	.short	0xfa1f
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa1e      	.short	0xfa1e
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa1d      	.short	0xfa1d
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa1c      	.short	0xfa1c
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa18      	.short	0xfa18
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa10      	.short	0xfa10
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f7df      	extend	0x7df
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f61f      	extend	0x61f
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f41f      	extend	0x41f
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> f01f      	extend	0x1f
[0-9a-f]+ <[^>]*> fa00      	.short	0xfa00
[0-9a-f]+ <[^>]*> db40      	sw	v0,0\(v1\)
[0-9a-f]+ <[^>]*> f000 db41 	sw	v0,1\(v1\)
[0-9a-f]+ <[^>]*> f000 db42 	sw	v0,2\(v1\)
[0-9a-f]+ <[^>]*> f000 db43 	sw	v0,3\(v1\)
[0-9a-f]+ <[^>]*> db41      	sw	v0,4\(v1\)
[0-9a-f]+ <[^>]*> db42      	sw	v0,8\(v1\)
[0-9a-f]+ <[^>]*> db44      	sw	v0,16\(v1\)
[0-9a-f]+ <[^>]*> db48      	sw	v0,32\(v1\)
[0-9a-f]+ <[^>]*> db50      	sw	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 db40 	sw	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 db40 	sw	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 db40 	sw	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 db40 	sw	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 db40 	sw	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff db5f 	sw	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff db5e 	sw	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff db5d 	sw	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff db5c 	sw	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff db58 	sw	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff db50 	sw	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff db40 	sw	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df db40 	sw	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f db40 	sw	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f db40 	sw	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f db40 	sw	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f db40 	sw	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f db40 	sw	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> d200      	sw	v0,0\(sp\)
[0-9a-f]+ <[^>]*> f000 d201 	sw	v0,1\(sp\)
[0-9a-f]+ <[^>]*> f000 d202 	sw	v0,2\(sp\)
[0-9a-f]+ <[^>]*> f000 d203 	sw	v0,3\(sp\)
[0-9a-f]+ <[^>]*> d201      	sw	v0,4\(sp\)
[0-9a-f]+ <[^>]*> d202      	sw	v0,8\(sp\)
[0-9a-f]+ <[^>]*> d204      	sw	v0,16\(sp\)
[0-9a-f]+ <[^>]*> d208      	sw	v0,32\(sp\)
[0-9a-f]+ <[^>]*> d210      	sw	v0,64\(sp\)
[0-9a-f]+ <[^>]*> d220      	sw	v0,128\(sp\)
[0-9a-f]+ <[^>]*> d240      	sw	v0,256\(sp\)
[0-9a-f]+ <[^>]*> d280      	sw	v0,512\(sp\)
[0-9a-f]+ <[^>]*> f400 d200 	sw	v0,1024\(sp\)
[0-9a-f]+ <[^>]*> f001 d200 	sw	v0,2048\(sp\)
[0-9a-f]+ <[^>]*> f7ff d21f 	sw	v0,-1\(sp\)
[0-9a-f]+ <[^>]*> f7ff d21e 	sw	v0,-2\(sp\)
[0-9a-f]+ <[^>]*> f7ff d21d 	sw	v0,-3\(sp\)
[0-9a-f]+ <[^>]*> f7ff d21c 	sw	v0,-4\(sp\)
[0-9a-f]+ <[^>]*> f7ff d218 	sw	v0,-8\(sp\)
[0-9a-f]+ <[^>]*> f7ff d210 	sw	v0,-16\(sp\)
[0-9a-f]+ <[^>]*> f7ff d200 	sw	v0,-32\(sp\)
[0-9a-f]+ <[^>]*> f7df d200 	sw	v0,-64\(sp\)
[0-9a-f]+ <[^>]*> f79f d200 	sw	v0,-128\(sp\)
[0-9a-f]+ <[^>]*> f71f d200 	sw	v0,-256\(sp\)
[0-9a-f]+ <[^>]*> f61f d200 	sw	v0,-512\(sp\)
[0-9a-f]+ <[^>]*> f41f d200 	sw	v0,-1024\(sp\)
[0-9a-f]+ <[^>]*> f01f d200 	sw	v0,-2048\(sp\)
[0-9a-f]+ <[^>]*> 6200      	sw	ra,0\(sp\)
[0-9a-f]+ <[^>]*> f000 6201 	sw	ra,1\(sp\)
[0-9a-f]+ <[^>]*> f000 6202 	sw	ra,2\(sp\)
[0-9a-f]+ <[^>]*> f000 6203 	sw	ra,3\(sp\)
[0-9a-f]+ <[^>]*> 6201      	sw	ra,4\(sp\)
[0-9a-f]+ <[^>]*> 6202      	sw	ra,8\(sp\)
[0-9a-f]+ <[^>]*> 6204      	sw	ra,16\(sp\)
[0-9a-f]+ <[^>]*> 6208      	sw	ra,32\(sp\)
[0-9a-f]+ <[^>]*> 6210      	sw	ra,64\(sp\)
[0-9a-f]+ <[^>]*> 6220      	sw	ra,128\(sp\)
[0-9a-f]+ <[^>]*> 6240      	sw	ra,256\(sp\)
[0-9a-f]+ <[^>]*> 6280      	sw	ra,512\(sp\)
[0-9a-f]+ <[^>]*> f400 6200 	sw	ra,1024\(sp\)
[0-9a-f]+ <[^>]*> f001 6200 	sw	ra,2048\(sp\)
[0-9a-f]+ <[^>]*> f7ff 621f 	sw	ra,-1\(sp\)
[0-9a-f]+ <[^>]*> f7ff 621e 	sw	ra,-2\(sp\)
[0-9a-f]+ <[^>]*> f7ff 621d 	sw	ra,-3\(sp\)
[0-9a-f]+ <[^>]*> f7ff 621c 	sw	ra,-4\(sp\)
[0-9a-f]+ <[^>]*> f7ff 6218 	sw	ra,-8\(sp\)
[0-9a-f]+ <[^>]*> f7ff 6210 	sw	ra,-16\(sp\)
[0-9a-f]+ <[^>]*> f7ff 6200 	sw	ra,-32\(sp\)
[0-9a-f]+ <[^>]*> f7df 6200 	sw	ra,-64\(sp\)
[0-9a-f]+ <[^>]*> f79f 6200 	sw	ra,-128\(sp\)
[0-9a-f]+ <[^>]*> f71f 6200 	sw	ra,-256\(sp\)
[0-9a-f]+ <[^>]*> f61f 6200 	sw	ra,-512\(sp\)
[0-9a-f]+ <[^>]*> f41f 6200 	sw	ra,-1024\(sp\)
[0-9a-f]+ <[^>]*> f01f 6200 	sw	ra,-2048\(sp\)
[0-9a-f]+ <[^>]*> cb40      	sh	v0,0\(v1\)
[0-9a-f]+ <[^>]*> f000 cb41 	sh	v0,1\(v1\)
[0-9a-f]+ <[^>]*> cb41      	sh	v0,2\(v1\)
[0-9a-f]+ <[^>]*> f000 cb43 	sh	v0,3\(v1\)
[0-9a-f]+ <[^>]*> cb42      	sh	v0,4\(v1\)
[0-9a-f]+ <[^>]*> cb44      	sh	v0,8\(v1\)
[0-9a-f]+ <[^>]*> cb48      	sh	v0,16\(v1\)
[0-9a-f]+ <[^>]*> cb50      	sh	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 cb40 	sh	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 cb40 	sh	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 cb40 	sh	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 cb40 	sh	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 cb40 	sh	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 cb40 	sh	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb5f 	sh	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb5e 	sh	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb5d 	sh	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb5c 	sh	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb58 	sh	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb50 	sh	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff cb40 	sh	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df cb40 	sh	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f cb40 	sh	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f cb40 	sh	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f cb40 	sh	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f cb40 	sh	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f cb40 	sh	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> c340      	sb	v0,0\(v1\)
[0-9a-f]+ <[^>]*> c341      	sb	v0,1\(v1\)
[0-9a-f]+ <[^>]*> c342      	sb	v0,2\(v1\)
[0-9a-f]+ <[^>]*> c343      	sb	v0,3\(v1\)
[0-9a-f]+ <[^>]*> c344      	sb	v0,4\(v1\)
[0-9a-f]+ <[^>]*> c348      	sb	v0,8\(v1\)
[0-9a-f]+ <[^>]*> c350      	sb	v0,16\(v1\)
[0-9a-f]+ <[^>]*> f020 c340 	sb	v0,32\(v1\)
[0-9a-f]+ <[^>]*> f040 c340 	sb	v0,64\(v1\)
[0-9a-f]+ <[^>]*> f080 c340 	sb	v0,128\(v1\)
[0-9a-f]+ <[^>]*> f100 c340 	sb	v0,256\(v1\)
[0-9a-f]+ <[^>]*> f200 c340 	sb	v0,512\(v1\)
[0-9a-f]+ <[^>]*> f400 c340 	sb	v0,1024\(v1\)
[0-9a-f]+ <[^>]*> f001 c340 	sb	v0,2048\(v1\)
[0-9a-f]+ <[^>]*> f7ff c35f 	sb	v0,-1\(v1\)
[0-9a-f]+ <[^>]*> f7ff c35e 	sb	v0,-2\(v1\)
[0-9a-f]+ <[^>]*> f7ff c35d 	sb	v0,-3\(v1\)
[0-9a-f]+ <[^>]*> f7ff c35c 	sb	v0,-4\(v1\)
[0-9a-f]+ <[^>]*> f7ff c358 	sb	v0,-8\(v1\)
[0-9a-f]+ <[^>]*> f7ff c350 	sb	v0,-16\(v1\)
[0-9a-f]+ <[^>]*> f7ff c340 	sb	v0,-32\(v1\)
[0-9a-f]+ <[^>]*> f7df c340 	sb	v0,-64\(v1\)
[0-9a-f]+ <[^>]*> f79f c340 	sb	v0,-128\(v1\)
[0-9a-f]+ <[^>]*> f71f c340 	sb	v0,-256\(v1\)
[0-9a-f]+ <[^>]*> f61f c340 	sb	v0,-512\(v1\)
[0-9a-f]+ <[^>]*> f41f c340 	sb	v0,-1024\(v1\)
[0-9a-f]+ <[^>]*> f01f c340 	sb	v0,-2048\(v1\)
[0-9a-f]+ <[^>]*> 6a00      	li	v0,0
[0-9a-f]+ <[^>]*> 6a01      	li	v0,1
[0-9a-f]+ <[^>]*> f100 6a00 	li	v0,256
[0-9a-f]+ <[^>]*> 675e      	move	v0,s8
[0-9a-f]+ <[^>]*> 6592      	move	s4,v0
[0-9a-f]+ <[^>]*> 4350      	.short	0x4350
[0-9a-f]+ <[^>]*> 4351      	.short	0x4351
[0-9a-f]+ <[^>]*> 435f      	.short	0x435f
[0-9a-f]+ <[^>]*> f010      	extend	0x10
[0-9a-f]+ <[^>]*> 4350      	.short	0x4350
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> 4350      	.short	0x4350
[0-9a-f]+ <[^>]*> e388      	.short	0xe388
[0-9a-f]+ <[^>]*> fd40      	.short	0xfd40
[0-9a-f]+ <[^>]*> fd41      	.short	0xfd41
[0-9a-f]+ <[^>]*> fd5f      	.short	0xfd5f
[0-9a-f]+ <[^>]*> f020      	extend	0x20
[0-9a-f]+ <[^>]*> fd40      	.short	0xfd40
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fd40      	.short	0xfd40
[0-9a-f]+ <[^>]*> f080      	extend	0x80
[0-9a-f]+ <[^>]*> fd40      	.short	0xfd40
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> fd40      	.short	0xfd40
[0-9a-f]+ <[^>]*> f17f      	extend	0x17f
[0-9a-f]+ <[^>]*> fe48      	.short	0xfe48
[0-9a-f]+ <[^>]*> f080      	extend	0x80
[0-9a-f]+ <[^>]*> fe40      	.short	0xfe40
[0-9a-f]+ <[^>]*> f1c0      	extend	0x1c0
[0-9a-f]+ <[^>]*> fe48      	.short	0xfe48
[0-9a-f]+ <[^>]*> f280      	extend	0x280
[0-9a-f]+ <[^>]*> fe4c      	.short	0xfe4c
[0-9a-f]+ <[^>]*> fb00      	.short	0xfb00
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> fb01      	.short	0xfb01
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> fb1f      	.short	0xfb1f
[0-9a-f]+ <[^>]*> fb20      	.short	0xfb20
[0-9a-f]+ <[^>]*> fbe0      	.short	0xfbe0
[0-9a-f]+ <[^>]*> ff40      	.short	0xff40
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> ff41      	.short	0xff41
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> ff5f      	.short	0xff5f
[0-9a-f]+ <[^>]*> ff48      	.short	0xff48
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> ff40      	.short	0xff40
[0-9a-f]+ <[^>]*> f080      	extend	0x80
[0-9a-f]+ <[^>]*> ff40      	.short	0xff40
[0-9a-f]+ <[^>]*> f79f      	extend	0x79f
[0-9a-f]+ <[^>]*> ff40      	.short	0xff40
[0-9a-f]+ <[^>]*> 4340      	addiu	v0,v1,0
[0-9a-f]+ <[^>]*> 4341      	addiu	v0,v1,1
[0-9a-f]+ <[^>]*> 434f      	addiu	v0,v1,-1
[0-9a-f]+ <[^>]*> f010 4340 	addiu	v0,v1,16
[0-9a-f]+ <[^>]*> f7ff 4340 	addiu	v0,v1,-16
[0-9a-f]+ <[^>]*> e389      	addu	v0,v1,a0
[0-9a-f]+ <[^>]*> 4a00      	addiu	v0,0
[0-9a-f]+ <[^>]*> 4a01      	addiu	v0,1
[0-9a-f]+ <[^>]*> 4aff      	addiu	v0,-1
[0-9a-f]+ <[^>]*> 4a20      	addiu	v0,32
[0-9a-f]+ <[^>]*> 4ae0      	addiu	v0,-32
[0-9a-f]+ <[^>]*> f080 4a00 	addiu	v0,128
[0-9a-f]+ <[^>]*> 4a80      	addiu	v0,-128
[0-9a-f]+ <[^>]*> f11f 0a14 	la	v0,00000000 <data1>
[0-9a-f]+ <[^>]*> 0a0b      	la	v0,0000071c <data2>
[0-9a-f]+ <[^>]*> 0a5d      	la	v0,00000868 <bar>
[0-9a-f]+ <[^>]*> 0a8f      	la	v0,00000930 <iuux>
[0-9a-f]+ <[^>]*> 6300      	addiu	sp,0
[0-9a-f]+ <[^>]*> f000 6301 	addiu	sp,1
[0-9a-f]+ <[^>]*> f7ff 631f 	addiu	sp,-1
[0-9a-f]+ <[^>]*> 6320      	addiu	sp,256
[0-9a-f]+ <[^>]*> 63e0      	addiu	sp,-256
[0-9a-f]+ <[^>]*> 0200      	addiu	v0,sp,0
[0-9a-f]+ <[^>]*> f000 0201 	addiu	v0,sp,1
[0-9a-f]+ <[^>]*> f7ff 021f 	addiu	v0,sp,-1
[0-9a-f]+ <[^>]*> 0208      	addiu	v0,sp,32
[0-9a-f]+ <[^>]*> f7ff 0200 	addiu	v0,sp,-32
[0-9a-f]+ <[^>]*> 0220      	addiu	v0,sp,128
[0-9a-f]+ <[^>]*> f79f 0200 	addiu	v0,sp,-128
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> e38a      	.short	0xe38a
[0-9a-f]+ <[^>]*> e38b      	subu	v0,v1,a0
[0-9a-f]+ <[^>]*> ea6b      	neg	v0,v1
[0-9a-f]+ <[^>]*> ea6c      	and	v0,v1
[0-9a-f]+ <[^>]*> ea6d      	or	v0,v1
[0-9a-f]+ <[^>]*> ea6e      	xor	v0,v1
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> 5200      	slti	v0,0
[0-9a-f]+ <[^>]*> 5201      	slti	v0,1
[0-9a-f]+ <[^>]*> f7ff 521f 	slti	v0,-1
[0-9a-f]+ <[^>]*> 52ff      	slti	v0,255
[0-9a-f]+ <[^>]*> f100 5200 	slti	v0,256
[0-9a-f]+ <[^>]*> ea62      	slt	v0,v1
[0-9a-f]+ <[^>]*> 5a00      	sltiu	v0,0
[0-9a-f]+ <[^>]*> 5a01      	sltiu	v0,1
[0-9a-f]+ <[^>]*> f7ff 5a1f 	sltiu	v0,-1
[0-9a-f]+ <[^>]*> 5aff      	sltiu	v0,255
[0-9a-f]+ <[^>]*> f100 5a00 	sltiu	v0,256
[0-9a-f]+ <[^>]*> ea63      	sltu	v0,v1
[0-9a-f]+ <[^>]*> 7200      	cmpi	v0,0
[0-9a-f]+ <[^>]*> 7201      	cmpi	v0,1
[0-9a-f]+ <[^>]*> 72ff      	cmpi	v0,255
[0-9a-f]+ <[^>]*> f100 7200 	cmpi	v0,256
[0-9a-f]+ <[^>]*> ea6a      	cmp	v0,v1
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> 3261      	.short	0x3261
[0-9a-f]+ <[^>]*> 3265      	.short	0x3265
[0-9a-f]+ <[^>]*> 3261      	.short	0x3261
[0-9a-f]+ <[^>]*> f240      	extend	0x240
[0-9a-f]+ <[^>]*> 3261      	.short	0x3261
[0-9a-f]+ <[^>]*> f7e0      	extend	0x7e0
[0-9a-f]+ <[^>]*> 3261      	.short	0x3261
[0-9a-f]+ <[^>]*> eb54      	.short	0xeb54
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> e848      	.short	0xe848
[0-9a-f]+ <[^>]*> e948      	.short	0xe948
[0-9a-f]+ <[^>]*> e848      	.short	0xe848
[0-9a-f]+ <[^>]*> f240      	extend	0x240
[0-9a-f]+ <[^>]*> e848      	.short	0xe848
[0-9a-f]+ <[^>]*> f7e0      	extend	0x7e0
[0-9a-f]+ <[^>]*> e848      	.short	0xe848
[0-9a-f]+ <[^>]*> eb56      	.short	0xeb56
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> e853      	.short	0xe853
[0-9a-f]+ <[^>]*> e953      	.short	0xe953
[0-9a-f]+ <[^>]*> e853      	.short	0xe853
[0-9a-f]+ <[^>]*> f240      	extend	0x240
[0-9a-f]+ <[^>]*> e853      	.short	0xe853
[0-9a-f]+ <[^>]*> f7e0      	extend	0x7e0
[0-9a-f]+ <[^>]*> e853      	.short	0xe853
[0-9a-f]+ <[^>]*> eb57      	.short	0xeb57
[0-9a-f]+ <[^>]*> ea12      	mflo	v0
[0-9a-f]+ <[^>]*> eb10      	mfhi	v1
[0-9a-f]+ <[^>]*> f000 3260 	sll	v0,v1,0
[0-9a-f]+ <[^>]*> 3264      	sll	v0,v1,1
[0-9a-f]+ <[^>]*> 3260      	sll	v0,v1,8
[0-9a-f]+ <[^>]*> f240 3260 	sll	v0,v1,9
[0-9a-f]+ <[^>]*> f7c0 3260 	sll	v0,v1,31
[0-9a-f]+ <[^>]*> eb44      	sllv	v0,v1
[0-9a-f]+ <[^>]*> f000 3262 	srl	v0,v1,0
[0-9a-f]+ <[^>]*> 3266      	srl	v0,v1,1
[0-9a-f]+ <[^>]*> 3262      	srl	v0,v1,8
[0-9a-f]+ <[^>]*> f240 3262 	srl	v0,v1,9
[0-9a-f]+ <[^>]*> f7c0 3262 	srl	v0,v1,31
[0-9a-f]+ <[^>]*> eb46      	srlv	v0,v1
[0-9a-f]+ <[^>]*> f000 3263 	sra	v0,v1,0
[0-9a-f]+ <[^>]*> 3267      	sra	v0,v1,1
[0-9a-f]+ <[^>]*> 3263      	sra	v0,v1,8
[0-9a-f]+ <[^>]*> f240 3263 	sra	v0,v1,9
[0-9a-f]+ <[^>]*> f7c0 3263 	sra	v0,v1,31
[0-9a-f]+ <[^>]*> eb47      	srav	v0,v1
[0-9a-f]+ <[^>]*> ea7c      	.short	0xea7c
[0-9a-f]+ <[^>]*> ea7d      	.short	0xea7d
[0-9a-f]+ <[^>]*> ea7e      	.short	0xea7e
[0-9a-f]+ <[^>]*> 2b01      	bnez	v1,000007d4 <insns2\+0xb4>
[0-9a-f]+ <[^>]*> e8e5      	break	0x7
[0-9a-f]+ <[^>]*> ea12      	mflo	v0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ea7f      	.short	0xea7f
[0-9a-f]+ <[^>]*> 2b01      	bnez	v1,000007e0 <insns2\+0xc0>
[0-9a-f]+ <[^>]*> e8e5      	break	0x7
[0-9a-f]+ <[^>]*> ea12      	mflo	v0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ea78      	mult	v0,v1
[0-9a-f]+ <[^>]*> ea79      	multu	v0,v1
[0-9a-f]+ <[^>]*> ea7a      	div	zero,v0,v1
[0-9a-f]+ <[^>]*> 2b01      	bnez	v1,000007f0 <insns2\+0xd0>
[0-9a-f]+ <[^>]*> e8e5      	break	0x7
[0-9a-f]+ <[^>]*> ea12      	mflo	v0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ea7b      	divu	zero,v0,v1
[0-9a-f]+ <[^>]*> 2b01      	bnez	v1,000007fc <insns2\+0xdc>
[0-9a-f]+ <[^>]*> e8e5      	break	0x7
[0-9a-f]+ <[^>]*> ea12      	mflo	v0
[0-9a-f]+ <[^>]*> ea00      	jr	v0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> ea40      	jalr	v0
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> f3ff 221b 	beqz	v0,00000004 <insns1>
[0-9a-f]+ <[^>]*> 2288      	beqz	v0,00000720 <insns2>
[0-9a-f]+ <[^>]*> 222b      	beqz	v0,00000868 <bar>
[0-9a-f]+ <[^>]*> f080 220d 	beqz	v0,00000930 <iuux>
[0-9a-f]+ <[^>]*> f3ff 2a15 	bnez	v0,00000004 <insns1>
[0-9a-f]+ <[^>]*> 2a82      	bnez	v0,00000720 <insns2>
[0-9a-f]+ <[^>]*> 2a25      	bnez	v0,00000868 <bar>
[0-9a-f]+ <[^>]*> f080 2a07 	bnez	v0,00000930 <iuux>
[0-9a-f]+ <[^>]*> f3ff 600f 	bteqz	00000004 <insns1>
[0-9a-f]+ <[^>]*> f77f 601b 	bteqz	00000720 <insns2>
[0-9a-f]+ <[^>]*> 601e      	bteqz	00000868 <bar>
[0-9a-f]+ <[^>]*> f080 6000 	bteqz	00000930 <iuux>
[0-9a-f]+ <[^>]*> f3ff 6108 	btnez	00000004 <insns1>
[0-9a-f]+ <[^>]*> f77f 6114 	btnez	00000720 <insns2>
[0-9a-f]+ <[^>]*> 6117      	btnez	00000868 <bar>
[0-9a-f]+ <[^>]*> 617a      	btnez	00000930 <iuux>
[0-9a-f]+ <[^>]*> f3ff 1002 	b	00000004 <insns1>
[0-9a-f]+ <[^>]*> 176f      	b	00000720 <insns2>
[0-9a-f]+ <[^>]*> 1012      	b	00000868 <bar>
[0-9a-f]+ <[^>]*> 1075      	b	00000930 <iuux>
[0-9a-f]+ <[^>]*> e805      	break
[0-9a-f]+ <[^>]*> e825      	break	0x1
[0-9a-f]+ <[^>]*> efe5      	break	0x3f
[0-9a-f]+ <[^>]*> 1800 0000 	jal	00000000 <data1>
[ 	]*[0-9a-f]+: R_MIPS16_26	extern
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> e809      	entry
[0-9a-f]+ <[^>]*> e909      	entry	a0
[0-9a-f]+ <[^>]*> eb49      	entry	a0-a2,s0
[0-9a-f]+ <[^>]*> e8a9      	entry	s0-s1,ra
[0-9a-f]+ <[^>]*> e829      	entry	ra
[0-9a-f]+ <[^>]*> ef09      	exit
[0-9a-f]+ <[^>]*> ef49      	exit	s0
[0-9a-f]+ <[^>]*> efa9      	exit	s0-s1,ra
[0-9a-f]+ <[^>]*> ef29      	exit	ra
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
