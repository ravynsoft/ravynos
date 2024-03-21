    .arch armv8.1-m.main
    .arch_extension pacbti
    .eabi_attribute Tag_PAC_extension, 2
    .eabi_attribute Tag_BTI_extension, 2
    .eabi_attribute Tag_BTI_use, 1
    .eabi_attribute Tag_PACRET_use, 1
    .syntax unified
    .text
    .thumb
.Lstart:
    .cfi_startproc
    pacbti ip, lr, sp
    .cfi_register ra_auth_code, ip
    push {ip, lr}
    .cfi_def_cfa_offset 8
    .cfi_offset lr, -8
    .cfi_offset ip, -4
    pop {ip, lr}
    .cfi_restore 143
    .cfi_restore 14
    .cfi_def_cfa_offset 0
    .cfi_endproc
