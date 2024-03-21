#source: pcrel-lo-addend-3a.s
#as: -march=rv64i -mabi=lp64 -mno-relax
#ld: -m[riscv_choose_lp64_emul] -Tpcrel-lo-addend-3.ld
#objdump: -d

#...
Disassembly of section .text:

0+900000000 <_start>:
.*:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+a5,0x2
.*:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+a0,0\(a5\) # 2000 <ll>
.*:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+a0,4\(a5\)
.*:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+a5,0x2
.*:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+a0,4\(a5\) # 2004 <ll\+0x4>
.*:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+a0,8\(a5\)
.*:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+a5,0x1
.*:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+a0,8\(a5\) # 1008 <_GLOBAL_OFFSET_TABLE_\+0x8>
#pass
