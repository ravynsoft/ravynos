#objdump: -t
#source: zerofill-1.s
.*: +file format mach-o.*
#...
SYMBOL TABLE:
(00000000)?00000004 l( )+0e SECT( )+03 0000 \[__DATA.__zf_2\] zfs
(00000000)?00000008 l( )+0e SECT( )+04 0000 \[__DATA.__zf_3\] withalign
(00000000)?00000010 l( )+0e SECT( )+04 0000 \[__DATA.__zf_3\] withalign1
(00000000)?00000000 g( )+0f SECT( )+01 0000 \[.text\] a
(00000000)?00000001 g( )+0f SECT( )+01 0000 \[.text\] b
(00000000)?00000002 g( )+0f SECT( )+01 0000 \[.text\] c
(00000000)?00000003 g( )+0f SECT( )+01 0000 \[.text\] d


