#name: NIOS2 relax_call26_cache
#ld: --relax -Trelax_call26_cache.ld
#source: relax_call26_cache.s
#objdump: -dr --prefix-addresses 
# Test relaxation of call26 relocations via linker stubs.  We don't need to
# check the exact layout of stubs for this test, only verify that it
# links without "relocation truncated to fit" errors.

#pass
