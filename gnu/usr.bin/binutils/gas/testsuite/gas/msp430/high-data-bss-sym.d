#objdump: -t
#name: Check symbols to initialise high data and high bss have been defined
#...
.*__crt0_move_highdata.*
.*__crt0_init_highbss.*
#pass
