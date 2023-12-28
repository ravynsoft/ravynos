#objdump: -t
#name: Check symbols to initialise data and bss have been defined for .lower sections
#...
.*__crt0_movedata.*
#...
.*__crt0_init_bss.*
#pass
