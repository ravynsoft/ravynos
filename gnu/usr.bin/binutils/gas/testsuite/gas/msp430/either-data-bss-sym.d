#objdump: -t
#name: Check symbols to initialise data and bss have been defined for .either sections
#...
.*__crt0_movedata.*
.*__crt0_move_highdata.*
#...
.*__crt0_init_bss.*
.*__crt0_init_highbss.*
#pass
