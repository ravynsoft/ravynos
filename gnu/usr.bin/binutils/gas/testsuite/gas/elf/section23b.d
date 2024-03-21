#name: SHF_GNU_RETAIN set with numeric flag value in .section for non-GNU OSABI target
#source: section23.s
#error_output: section23b.err
#target: msp430-*-elf visium-*-elf

# This test only runs for targets which set ELFOSABI_STANDALONE.
