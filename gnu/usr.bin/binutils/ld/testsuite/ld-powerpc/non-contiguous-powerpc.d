#name: non-contiguous-powerpc
#source: non-contiguous-powerpc.s
#ld: --enable-non-contiguous-regions -T non-contiguous-powerpc.ld
#error: .*Relaxation not supported with --enable-non-contiguous-regions.*
#skip: powerpc64*-*
