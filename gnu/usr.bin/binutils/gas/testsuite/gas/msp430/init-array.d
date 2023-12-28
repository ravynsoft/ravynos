#objdump: -t
#name: Check symbols to run .init_array functions have been defined
#...
.*__crt0_run_init_array.*
#...
.*__crt0_run_array.*
#pass
