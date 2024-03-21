#name: NIOS2 relax_call26_boundary_dc
#ld: --relax -Trelax_call26_boundary.ld --section-start=text0=0x0fffffdc
#source: relax_call26_boundary.s
#objdump: -dr --prefix-addresses 
# Test relaxation of call26 relocations via linker stubs.  We don't need to
# check the exact layout of stubs for this test, only verify that it
# links without "relocation truncated to fit" errors.

#pass
