#source: ../ld-x86-64/property-x86-6.s
#as: --32 -mx86-used-note=no
#ld: -shared -m elf_i386
#readelf: -n

Displaying notes found in: .note.gnu.property
[ 	]+Owner[ 	]+Data size[ 	]+Description
  GNU                  0x00000018	NT_GNU_PROPERTY_TYPE_0
      Properties: x86 ISA needed: CMOV, SSE, SSSE3, SSE4_1
	x86 ISA used: SSE, SSE3, SSE4_1, AVX
