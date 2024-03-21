.arm
.syntax unified
  # VFPv3xD has no VMOV instruction for double precision registers...
  # 15 * 2^-7 =0.1171875 Not convertible to VMOV
  .align 3
  vldr d0,=0x3FBE000000000000
  vldr s0,=0x3df00000
  .pool

  # -16 * 2^-7 =0.125 Convertible to VMOV
  .align 3
  vldr d0,=0xbfc0000000000000
  vldr s0,=0xbe000000
  .pool

  # 16 * 2^-7 =0.125 Convertible to VMOV
  .align 3
  vldr d0,=0x3fc0000000000000
  vldr s0,=0x3e000000
  .pool

  # 16.5 * 2^-7 =0.125 Not convertible to VMOV
  .align 3
  vldr d0,=0x3fe0800000000000
  vldr s0,=0x3f040000
  .pool

  # 31 * 2^-5 = 0.96875 Convertible to VMOV
  .align 3
  vldr d0,=0x3fef000000000000
  vldr s0,=0x3f780000
  .pool

  # 31 * 2^ 0 = 31 Convertible to VMOV
  .align 3
  vldr d0,=0x403F000000000000 
  vldr s0,=0x41f80000
  .pool

  # 16 * 2^ 1 = 32 Not convertible to VMOV
  .align 3
  vldr d0,=0x4040000000000000
  vldr s0,=0x42000000
  .pool

  nop
	
