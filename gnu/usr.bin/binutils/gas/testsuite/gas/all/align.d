#objdump: -s -j .text
#name: align
# The RX port will always replace zeros in any aligned area with NOPs,
# even if the user requested that they filled with zeros.
# RISC-V handles alignment via relaxation and therefor won't have object files
# LoongArch handles alignment via relaxation and therefor won't have object
# files with the expected alignment.
#notarget: loongarch*-* riscv*-* rx-*

# Test the alignment pseudo-op.

.*: .*

Contents of section .text:
 0000 ff00ff01 ff020202 ffff0303 04040404  ................
 0010 ffffffff 05050505 ff090a0a 0a0a0a0a  ................
 0020 ff00ff01 ff020202 ffff0303 04040404  ................
 0030 ffffffff 05050505 ff090a0a 0a0a0a0a  ................
