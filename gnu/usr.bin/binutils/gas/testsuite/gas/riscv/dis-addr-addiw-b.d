#as: -march=rv64ic
#source: dis-addr-addiw.s
#objdump: -d --adjust-vma=0x7fffffe0

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0+7fffffe0 <_start>:
[ 	]+7fffffe0:[ 	]+00000297[ 	]+auipc[ 	]+t0,0x0
[ 	]+7fffffe4:[ 	]+0182831b[ 	]+addw[ 	]+t1,t0,24 # 7ffffff8 <addr_rv64_addiw_1a>
[ 	]+7fffffe8:[ 	]+00000397[ 	]+auipc[ 	]+t2,0x0
[ 	]+7fffffec:[ 	]+01c38e1b[ 	]+addw[ 	]+t3,t2,28 # ffffffff80000004 <addr_rv64_addiw_1b>
[ 	]+7ffffff0:[ 	]+00000e97[ 	]+auipc[ 	]+t4,0x0
[ 	]+7ffffff4:[ 	]+2eb1[ 	]+addw[ 	]+t4,t4,12 # 7ffffffc <addr_rv64_c_addiw_1a>
[ 	]+7ffffff6:[ 	]+00000f17[ 	]+auipc[ 	]+t5,0x0
[ 	]+7ffffffa:[ 	]+2f49[ 	]+addw[ 	]+t5,t5,18 # ffffffff80000008 <addr_rv64_c_addiw_1b>
