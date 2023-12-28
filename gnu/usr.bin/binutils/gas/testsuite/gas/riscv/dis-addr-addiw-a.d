#as: -march=rv64ic
#source: dis-addr-addiw.s
#objdump: -d --adjust-vma=0xffffffe0

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0+ffffffe0 <_start>:
[ 	]+ffffffe0:[ 	]+00000297[ 	]+auipc[ 	]+t0,0x0
[ 	]+ffffffe4:[ 	]+0182831b[ 	]+addw[ 	]+t1,t0,24 # fffffffffffffff8 <addr_rv64_addiw_0a>
[ 	]+ffffffe8:[ 	]+00000397[ 	]+auipc[ 	]+t2,0x0
[ 	]+ffffffec:[ 	]+01c38e1b[ 	]+addw[ 	]+t3,t2,28 # 4 <addr_rv64_addiw_0b>
[ 	]+fffffff0:[ 	]+00000e97[ 	]+auipc[ 	]+t4,0x0
[ 	]+fffffff4:[ 	]+2eb1[ 	]+addw[ 	]+t4,t4,12 # fffffffffffffffc <addr_rv64_c_addiw_0a>
[ 	]+fffffff6:[ 	]+00000f17[ 	]+auipc[ 	]+t5,0x0
[ 	]+fffffffa:[ 	]+2f49[ 	]+addw[ 	]+t5,t5,18 # 8 <addr_rv64_c_addiw_0b>
