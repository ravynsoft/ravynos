#objdump: -dr --prefix-addresses
#as: -32 -EL --defsym tl_d=1
#name: MIPS l.d singlefloat
#source: ld.s

# Test the l.d macro on system without ldc1 and sdc1:

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(zero\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(zero\)
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(zero\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(zero\)
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(zero\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(zero\)
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x2
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(a1\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(a1\)
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(a1\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(a1\)
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(a1\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(a1\)
[0-9a-f]+ <[^>]*> lui	at,0x1
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x2
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(gp\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,gp
[0-9a-f]+ <[^>]*> lwc1	\$f4,1\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,5\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_GPREL16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x0
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-32768\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-32764\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x1
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,0\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,4\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.data
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.data
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_data_label
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_data_label
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	big_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	big_external_common
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	small_external_common
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	small_external_common
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.bss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> lui	at,0x2
			[0-9a-f]+: R_(MICRO)?MIPS_HI16	\.sbss
[0-9a-f]+ <[^>]*> addu	at,a1,at
[0-9a-f]+ <[^>]*> lwc1	\$f4,-23131\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
[0-9a-f]+ <[^>]*> lwc1	\$f5,-23127\(at\)
			[0-9a-f]+: R_(MICRO)?MIPS_LO16	\.sbss
	\.\.\.
