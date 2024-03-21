#as: -Os
#ld: -static --relax -T $srcdir/$subdir/diff.ld
#objdump: -D --prefix-addresses -j .data --show-raw-insn

.*:     file format .*nds32.*


Disassembly of section .data:
00008000 <WORD> (7e 00 00 00|00 00 00 7e).*
00008004 <HALF> (7e 00|00 7e).*
00008006 <BYTE> 7e.*
00008007 <ULEB128> fe.*
	...
00008009 <ULEB128_2> fe 00.*
.*
.*
