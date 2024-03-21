#source: eh-frame-hdr.s
#ld: -e _start --eh-frame-hdr
#objdump: -hw
#alltargets: [check_as_cfi] [check_shared_lib_support]
#...
  [0-9] .eh_frame_hdr +0*[12][048c] .*
#pass
