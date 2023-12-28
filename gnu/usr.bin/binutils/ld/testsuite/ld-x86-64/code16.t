OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")
SECTIONS
{
.text.default_process_op.isra.0 0x737c : { *(.text.default_process_op.isra.0) }
.text.mpt_scsi_process_op 0xf869 : { *(.text.mpt_scsi_process_op) }
}
