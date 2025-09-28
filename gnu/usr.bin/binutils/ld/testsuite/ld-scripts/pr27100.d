#ld: -r -T pr27100.t
#objdump: -h
#notarget: [is_aout_format]
#xfail: alpha*-*-*vms* mmix-*-* *c54x-*-* [is_xcoff_format]

#...
.* \.data +0+60 .*
#...
.* \.data +0+10 .*
#pass
