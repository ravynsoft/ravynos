#name: readelf SHF_GNU_RETAIN
#source: retain1.s
#target: [supports_gnu_osabi]
#readelf: -S --wide

#...
  \[[ 0-9]+\] .bss.retain0.*WAR.*
  \[[ 0-9]+\] .bss.retain1.*WAR.*
  \[[ 0-9]+\] .data.retain2.*WAR.*
  \[[ 0-9]+\] .bss.sretain0.*WAR.*
  \[[ 0-9]+\] .bss.sretain1.*WAR.*
  \[[ 0-9]+\] .data.sretain2.*WAR.*
  \[[ 0-9]+\] .text.fnretain1.*AXR.*
#...
  \[[ 0-9]+\] .bss.lsretain0.*WAR.*
  \[[ 0-9]+\] .bss.lsretain1.*WAR.*
  \[[ 0-9]+\] .data.lsretain2.*WAR.*
#...
  R \(retain\), D \(mbind\), .*
#pass
