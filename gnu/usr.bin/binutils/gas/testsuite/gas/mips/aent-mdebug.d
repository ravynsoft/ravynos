#objdump: -dr --prefix-addresses
#name: MIPS .aent directive with ECOFF debug
#as: -32 -mdebug
#source: aent.s
#dump: aent.d

# Test the .aent directive retains function symbol type annotation.
