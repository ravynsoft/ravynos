#objdump: -t
#name: Check symbols to run .fini_array functions have been defined
#...
.*__crt0_run_fini_array.*
#...
.*__crt0_run_array.*
#pass
