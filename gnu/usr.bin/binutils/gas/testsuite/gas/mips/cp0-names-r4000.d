#objdump: -dr --prefix-addresses --show-raw-insn -M gpr-names=numeric
#name: MIPS CP0 register disassembly
#as: -32 -march=r4000
#source: cp0-names.s

# Check objdump's handling of -M cp0-names=foo options.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40800000 	mtc0	\$0,c0_index
[0-9a-f]+ <[^>]*> 40800800 	mtc0	\$0,c0_random
[0-9a-f]+ <[^>]*> 40801000 	mtc0	\$0,c0_entrylo0
[0-9a-f]+ <[^>]*> 40801800 	mtc0	\$0,c0_entrylo1
[0-9a-f]+ <[^>]*> 40802000 	mtc0	\$0,c0_context
[0-9a-f]+ <[^>]*> 40802800 	mtc0	\$0,c0_pagemask
[0-9a-f]+ <[^>]*> 40803000 	mtc0	\$0,c0_wired
[0-9a-f]+ <[^>]*> 40803800 	mtc0	\$0,\$7
[0-9a-f]+ <[^>]*> 40804000 	mtc0	\$0,c0_badvaddr
[0-9a-f]+ <[^>]*> 40804800 	mtc0	\$0,c0_count
[0-9a-f]+ <[^>]*> 40805000 	mtc0	\$0,c0_entryhi
[0-9a-f]+ <[^>]*> 40805800 	mtc0	\$0,c0_compare
[0-9a-f]+ <[^>]*> 40806000 	mtc0	\$0,c0_sr
[0-9a-f]+ <[^>]*> 40806800 	mtc0	\$0,c0_cause
[0-9a-f]+ <[^>]*> 40807000 	mtc0	\$0,c0_epc
[0-9a-f]+ <[^>]*> 40807800 	mtc0	\$0,c0_prid
[0-9a-f]+ <[^>]*> 40808000 	mtc0	\$0,c0_config
[0-9a-f]+ <[^>]*> 40808800 	mtc0	\$0,c0_lladdr
[0-9a-f]+ <[^>]*> 40809000 	mtc0	\$0,c0_watchlo
[0-9a-f]+ <[^>]*> 40809800 	mtc0	\$0,c0_watchhi
[0-9a-f]+ <[^>]*> 4080a000 	mtc0	\$0,c0_xcontext
[0-9a-f]+ <[^>]*> 4080a800 	mtc0	\$0,\$21
[0-9a-f]+ <[^>]*> 4080b000 	mtc0	\$0,\$22
[0-9a-f]+ <[^>]*> 4080b800 	mtc0	\$0,\$23
[0-9a-f]+ <[^>]*> 4080c000 	mtc0	\$0,\$24
[0-9a-f]+ <[^>]*> 4080c800 	mtc0	\$0,\$25
[0-9a-f]+ <[^>]*> 4080d000 	mtc0	\$0,c0_ecc
[0-9a-f]+ <[^>]*> 4080d800 	mtc0	\$0,c0_cacheerr
[0-9a-f]+ <[^>]*> 4080e000 	mtc0	\$0,c0_taglo
[0-9a-f]+ <[^>]*> 4080e800 	mtc0	\$0,c0_taghi
[0-9a-f]+ <[^>]*> 4080f000 	mtc0	\$0,c0_errorepc
[0-9a-f]+ <[^>]*> 4080f800 	mtc0	\$0,\$31
	\.\.\.
