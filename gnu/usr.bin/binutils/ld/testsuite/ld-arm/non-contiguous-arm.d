#name: non-contiguous-arm
#source: non-contiguous-arm.s
#ld: --enable-non-contiguous-regions -T non-contiguous-arm.ld
# error: .*Could not assign .?\.code\.4.? to an output section. Retry without --enable-non-contiguous-regions\.
