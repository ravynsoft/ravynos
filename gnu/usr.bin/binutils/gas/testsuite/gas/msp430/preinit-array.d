#objdump: -t
#name: Check symbols to run .preinit_array functions have been defined
#...
.*__crt0_run_preinit_array.*
#...
.*__crt0_run_array.*
#pass
