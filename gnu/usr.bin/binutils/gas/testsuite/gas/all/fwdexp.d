#objdump: -rs -j .text
#name: forward expression

.*: .*

RELOCATION RECORDS FOR .*
OFFSET +TYPE +VALUE
0+ .*(\.data|label_i)(|\+0xf+e|\+0xf+c|\+0xf+8|-0x0*2|-0x0*4|-0x0*8)
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*

Contents of section .*
 0+ (0+|feff|fffe|fcffffff|fffffffc|f8ffffff|f8ffffff ffffffff|ffffffff fffffff8|0+4) .*
