#objdump: -d --prefix-addresses
#as: -mips64r2
#name: MIPS li.d
#source: li-d.s

# Test the li.d macro.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> li	v(0|1),0
[0-9a-f]+ <[^>]*> move	v(1|0),zero
[0-9a-f]+ <[^>]*> mtc1	zero,\$f1
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f1
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> ldc1	\$f0,0\(gp\)
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> dmtc1	zero,\$f0
[0-9a-f]+ <[^>]*> lui	v[01],0x3ff0
[0-9a-f]+ <[^>]*> move	v[01],zero
[0-9a-f]+ <[^>]*> lui	at,0x3ff0
[0-9a-f]+ <[^>]*> mtc1	at,\$f1
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> lui	at,0x3ff0
[0-9a-f]+ <[^>]*> mtc1	at,\$f1
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> ldc1	\$f0,(0|8)\(gp\)
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> lui	at,0x3ff0
[0-9a-f]+ <[^>]*> mthc1	at,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> lui	at,0x3ff0
[0-9a-f]+ <[^>]*> mthc1	at,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f0
[0-9a-f]+ <[^>]*> lui	at,0x3ff0
[0-9a-f]+ <[^>]*> mthc1	at,\$f0
[0-9a-f]+ <[^>]*> li	at,0xffc0
[0-9a-f]+ <[^>]*> dsll32	at,at,0xe
[0-9a-f]+ <[^>]*> dmtc1	at,\$f0
[0-9a-f]+ <[^>]*> li	v[01],0
[0-9a-f]+ <[^>]*> li	v[01],4250
[0-9a-f]+ <[^>]*> mtc1	zero,\$f1
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> mtc1	at,\$f0
[0-9a-f]+ <[^>]*> mtc1	zero,\$f1
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> mtc1	at,\$f0
[0-9a-f]+ <[^>]*> ldc1	\$f0,(0|16)\(gp\)
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> mtc1	at,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> mtc1	at,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> mtc1	at,\$f0
[0-9a-f]+ <[^>]*> mthc1	zero,\$f0
[0-9a-f]+ <[^>]*> li	at,4250
[0-9a-f]+ <[^>]*> dmtc1	at,\$f0
	\.\.\.
