#name: attributes for 'any' CPU with ARMv8-M Mainline Security Extensions instructions
#source: archv8m-cmse-main.s
#as:
#readelf: -A
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_CPU_arch: v8-M.mainline
  Tag_CPU_arch_profile: Microcontroller
  Tag_THUMB_ISA_use: Yes
