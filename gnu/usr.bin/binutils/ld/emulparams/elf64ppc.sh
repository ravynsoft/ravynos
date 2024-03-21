source_sh ${srcdir}/emulparams/elf32ppccommon.sh
source_sh ${srcdir}/emulparams/plt_unwind.sh
source_sh ${srcdir}/emulparams/dt-relr.sh
EXTRA_EM_FILE=ppc64elf
ELFSIZE=64
OUTPUT_FORMAT="elf64-powerpc"
TEXT_START_ADDR=0x10000000
#SEGMENT_SIZE=0x10000000
ARCH=powerpc:common64
unset EXECUTABLE_SYMBOLS
unset SDATA_START_SYMBOLS
unset SDATA2_START_SYMBOLS
unset SBSS_START_SYMBOLS
unset SBSS_END_SYMBOLS
unset OTHER_END_SYMBOLS
unset OTHER_RELRO_SECTIONS
OTHER_TEXT_SECTIONS="*(.sfpr .glink)"
OTHER_SDATA_SECTIONS="
  .tocbss	${RELOCATING-0} :${RELOCATING+ ALIGN(8)} { *(.tocbss)}"

if test x${RELOCATING+set} = xset; then
  GOT="
  .got		: ALIGN(256) { *(.got .toc) }"
else
  GOT="
  .got		0 : { *(.got) }
  .toc		0 : { *(.toc) }"
fi
# Put .opd relocs first so ld.so will process them before any ifunc relocs.
INITIAL_RELOC_SECTIONS="
  .rela.opd	${RELOCATING-0} : { *(.rela.opd) }"
OTHER_GOT_RELOC_SECTIONS="
  .rela.toc	${RELOCATING-0} : { *(.rela.toc) }
  .rela.toc1	${RELOCATING-0} : { *(.rela.toc1) }
  .rela.tocbss	${RELOCATING-0} : { *(.rela.tocbss) }
  .rela.branch_lt	${RELOCATING-0} : { *(.rela.branch_lt) }"
# The idea behind setting .branch_lt address as we do below is to put
# it up against .got which is 256 byte aligned, so that the offset
# from .TOC. to an entry in .branch_lt remains fixed after stub
# sizing.  (.eh_frame is edited late.)  When -z relro -z now, we have
# .branch_lt, .plt, .iplt, then .got, so in that case we move
# .branch_lt so that the end of .iplt is against .got.  All of these
# sections are linker generated, with alignment eight and size a
# multiple of eight, but a user playing games with their own
# .branch_lt, .plt or .iplt sections can result in unexpected
# alignment or size.  Cope with that anyway.  Note that if user
# alignment of .branch_lt is 256 or more then nothing special need be
# done.
#
# To understand what is going on here consider that the end address
# of .iplt should be 0 mod 256, so the start of .iplt should be
# -sizeof(.iplt) mod 256.  But the start is constrained by alignment,
# so goes down to (-alignof(.iplt) & -sizeof(.iplt)) mod 256.  Repeat
# that calculation for .plt and .branch_lt to find the start of
# .branch_lt then subtract . mod 256 to find the padding.  Of course
# just one mod 256 suffices, which is done by anding with 255.
OTHER_RELRO_SECTIONS_2="
  .opd		${RELOCATING-0} :${RELOCATING+ ALIGN(8)} { KEEP (*(.opd)) }
  .toc1		${RELOCATING-0} :${RELOCATING+ ALIGN(8)} { *(.toc1) }
  .branch_lt	${RELOCATING-0}${RELOCATING+ALIGNOF(.branch_lt) < 256 && SIZEOF(.got) != 0 ? . + (((-MAX(ALIGNOF(.branch_lt),8) & (-SIZEOF(.branch_lt)${RELRO_NOW+ + (-MAX(ALIGNOF(.plt),8) & (-SIZEOF(.plt) + (-MAX(ALIGNOF(.iplt),8) & -SIZEOF(.iplt))))})) - .) & 255) : ALIGN(MAX(ALIGNOF(.branch_lt), SIZEOF(.got) != 0 ? 256 : 8))} : { *(.branch_lt) }"
INITIAL_READWRITE_SECTIONS="
  .toc		${RELOCATING-0} :${RELOCATING+ ALIGN(8)} { *(.toc) }"
# Put .got before .data
DATA_GOT=" "
# Always make .got read-only after relocation
SEPARATE_GOTPLT=0
# Also put .sdata before .data
DATA_SDATA=" "
# and .plt/.iplt before .data
DATA_PLT=
PLT_BEFORE_GOT=" "
