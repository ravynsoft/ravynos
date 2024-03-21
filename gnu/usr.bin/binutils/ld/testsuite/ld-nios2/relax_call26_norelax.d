#name: NIOS2 relax_call26_norelax
#ld: --no-relax -Trelax_call26_multi.ld
#source: relax_call26.s
#error: .*relocation truncated to fit: R_NIOS2_CALL26.*
# Test relaxation of call26 relocations via linker stubs
