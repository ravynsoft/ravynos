    .text
footext:
    .text
    .global lbur
lbur:
    lbur r0,r0,r0
    .text
    .global lhur
lhur:
    lhur r0,r0,r0
    .text
    .global lwr
lwr:
    lwr r0,r0,r0
    .text
    .global sbr
sbr:
    sbr r0,r0,r0
    .text
    .global shr
shr:
    shr r0,r0,r0
    .text
    .global swr
swr:
    swr r0,r0,r0
    .text
    .global clz
clz:
    clz r0,r0
    .text
    .global mbar
mbar:
    mbar 2
    .text
    .global sleep
sleep:
    sleep
    .text
    .global regslr
regslr:
    la r11,r0,r0
    mts rslr,r11
    .text
    .global regshr
regshr:
    la r11,r0,r0
    mts rshr,r11
    .text
    .global swapb
swapb:
    swapb r0,r0
    .text
    .global swaph
swaph:
    swaph r0,r0

