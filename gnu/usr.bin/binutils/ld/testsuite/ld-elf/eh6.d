#source: eh6.s
#ld: --gc-sections -shared -z notext
#readelf: -wf -rW
#target: x86_64-*-linux-gnu* i?86-*-linux-gnu i?86-*-gnu*

#...
[0-9a-f]+ +[0-9a-f]+ +R_.*_NONE *0?
#...
Contents of the .eh_frame section:

0+0000 0+001[48] 0+0000 CIE
  Version:               1
  Augmentation:          "zPR"
  Code alignment factor: 1
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     9[bc] .* 1b

  DW_CFA_nop
#pass
