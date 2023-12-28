#as: -linker-allocated-gregs
#source: start.s
#source: data-1.s
#source: tm-orph1.s
#source: tm-d.s
#source: tm-awpe.s
#ld: -m mmo -u __etext -u __Sdata -u __Edata -u __Sbss -u __Ebss -u __Eall
#error: overlaps section .text

# Like orph-d-awp.d but with contents in that section.  Also, mismatching
# section flags for the contents will cause a linker error, but we'll
# call this a doctor-it-hurts situation; either list the section in
# the linker script or have consistent section flags.
