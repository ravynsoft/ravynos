#objdump: -r
#name: i386 quoted symbols (data)
# Mach-O relocations appear in inverse order
#notarget: *-*-darwin

.*: +file format .*

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
0+00 (R_386_|dir)?32 +%ebx
0+04 (R_386_|dir)?32 +%rdx
0+08 (R_386_|dir)?32 +eax
0+0c (R_386_|dir)?32 +cr0
0+10 (R_386_|dir)?32 +k0
#pass
