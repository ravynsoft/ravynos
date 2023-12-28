#name: C6X PC-relative relocations, local symbols, -r, mixed link of REL/RELA
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: pcrel-reloc-local-1.s -mgenerate-rel
#source: pcrel-reloc-local-2.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

0+ <[^>]*>:
[ \t]*0:[ \t]+00000000[ \t]+nop 1
[ \t]*4:[ \t]+00800162[ \t]+addkpc \.S2 0 <[^>]*>,b1,0
[ \t]*4: R_C6000_PCR_S7[ \t]+\.text\.1
[ \t]*8:[ \t]+00810162[ \t]+addkpc \.S2 4 <[^>]*>,b1,0
[ \t]*8: R_C6000_PCR_S7[ \t]+\.text\.1
[ \t]*c:[ \t]+00000012[ \t]+b \.S2 0 <[^>]*>
[ \t]*c: R_C6000_PCR_S21[ \t]+\.text\.1
[ \t]*10:[ \t]+00000092[ \t]+b \.S2 4 <[^>]*>
[ \t]*10: R_C6000_PCR_S21[ \t]+\.text\.1
[ \t]*14:[ \t]+00801022[ \t]+bdec \.S2 0 <[^>]*>,b1
[ \t]*14: R_C6000_PCR_S10[ \t]+\.text\.1
[ \t]*18:[ \t]+00803022[ \t]+bdec \.S2 4 <[^>]*>,b1
[ \t]*18: R_C6000_PCR_S10[ \t]+\.text\.1
[ \t]*1c:[ \t]+00000122[ \t]+bnop \.S2 0 <[^>]*>,0
[ \t]*1c: R_C6000_PCR_S12[ \t]+\.text\.1
[ \t]*20:[ \t]+00010122[ \t]+bnop \.S2 24 <[^>]*>,0
[ \t]*20: R_C6000_PCR_S12[ \t]+\.text\.1
[ \t]*\.\.\.
[ \t]*44:[ \t]+00000122[ \t]+bnop \.S2 40 <[^>]*>,0
[ \t]*44: R_C6000_PCR_S12[ \t]+\.text\.1\+0x20
[ \t]*48:[ \t]+00000122[ \t]+bnop \.S2 40 <[^>]*>,0
[ \t]*48: R_C6000_PCR_S12[ \t]+\.text\.1\+0x24
[ \t]*4c:[ \t]+00801022[ \t]+bdec \.S2 40 <[^>]*>,b1
[ \t]*4c: R_C6000_PCR_S10[ \t]+\.text\.1\+0x20
[ \t]*50:[ \t]+00801022[ \t]+bdec \.S2 40 <[^>]*>,b1
[ \t]*50: R_C6000_PCR_S10[ \t]+\.text\.1\+0x24
[ \t]*54:[ \t]+00000012[ \t]+b \.S2 40 <[^>]*>
[ \t]*54: R_C6000_PCR_S21[ \t]+\.text\.1\+0x20
[ \t]*58:[ \t]+00000012[ \t]+b \.S2 40 <[^>]*>
[ \t]*58: R_C6000_PCR_S21[ \t]+\.text\.1\+0x24
[ \t]*5c:[ \t]+00800162[ \t]+addkpc \.S2 40 <[^>]*>,b1,0
[ \t]*5c: R_C6000_PCR_S7[ \t]+\.text\.1\+0x20
[ \t]*60:[ \t]+00800162[ \t]+addkpc \.S2 60 <[^>]*>,b1,0
[ \t]*60: R_C6000_PCR_S7[ \t]+\.text\.1\+0x24
[ \t]*\.\.\.

Disassembly of section \.text\.1:

0+ <[^>]*>:
[ \t]*0:[ \t]+00000000[ \t]+nop 1

0+4 <[^>]*>:
[ \t]*\.\.\.

0+20 <[^>]*>:
[ \t]*20:[ \t]+00000000[ \t]+nop 1

0+24 <[^>]*>:
[ \t]*\.\.\.
