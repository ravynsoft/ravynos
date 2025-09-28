# If you change this file, please also look at files which source this one:
# elf32_sparc_sol2.sh, elf64_sparc_sol2.sh, elf_i386_sol2.sh,
# elf_x86_64_sol2.sh.

# The Solaris 2 ABI requires that two local symbols are present in every
# executable and shared object.
# Cf. Linker and Libraries Guide, Ch. 2, Link-Editor, Generating the Output
# File, p.63.
TEXT_START_SYMBOLS='_START_ = .;'
OTHER_END_SYMBOLS='_END_ = .;'
# Beginning with Solaris 11.x and Solaris 12, there's PIE support.
GENERATE_PIE_SCRIPT=yes
