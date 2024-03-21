#objdump: -d
#name: rotate and shift

.*: .*

Disassembly of section .text:

0+ <.text>:

[ 	]+[0-9a-f]+:[ 	]+cb 07[ 	]+rlc a
[ 	]+[0-9a-f]+:[ 	]+cb 00[ 	]+rlc b
[ 	]+[0-9a-f]+:[ 	]+cb 01[ 	]+rlc c
[ 	]+[0-9a-f]+:[ 	]+cb 02[ 	]+rlc d
[ 	]+[0-9a-f]+:[ 	]+cb 03[ 	]+rlc e
[ 	]+[0-9a-f]+:[ 	]+cb 04[ 	]+rlc h
[ 	]+[0-9a-f]+:[ 	]+cb 05[ 	]+rlc l
[ 	]+[0-9a-f]+:[ 	]+cb 06[ 	]+rlc \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 06[ 	]+rlc \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 06[ 	]+rlc \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 0f[ 	]+rrc a
[ 	]+[0-9a-f]+:[ 	]+cb 08[ 	]+rrc b
[ 	]+[0-9a-f]+:[ 	]+cb 09[ 	]+rrc c
[ 	]+[0-9a-f]+:[ 	]+cb 0a[ 	]+rrc d
[ 	]+[0-9a-f]+:[ 	]+cb 0b[ 	]+rrc e
[ 	]+[0-9a-f]+:[ 	]+cb 0c[ 	]+rrc h
[ 	]+[0-9a-f]+:[ 	]+cb 0d[ 	]+rrc l
[ 	]+[0-9a-f]+:[ 	]+cb 0e[ 	]+rrc \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 0e[ 	]+rrc \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 0e[ 	]+rrc \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 17[ 	]+rl a
[ 	]+[0-9a-f]+:[ 	]+cb 10[ 	]+rl b
[ 	]+[0-9a-f]+:[ 	]+cb 11[ 	]+rl c
[ 	]+[0-9a-f]+:[ 	]+cb 12[ 	]+rl d
[ 	]+[0-9a-f]+:[ 	]+cb 13[ 	]+rl e
[ 	]+[0-9a-f]+:[ 	]+cb 14[ 	]+rl h
[ 	]+[0-9a-f]+:[ 	]+cb 15[ 	]+rl l
[ 	]+[0-9a-f]+:[ 	]+cb 16[ 	]+rl \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 16[ 	]+rl \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 16[ 	]+rl \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 1f[ 	]+rr a
[ 	]+[0-9a-f]+:[ 	]+cb 18[ 	]+rr b
[ 	]+[0-9a-f]+:[ 	]+cb 19[ 	]+rr c
[ 	]+[0-9a-f]+:[ 	]+cb 1a[ 	]+rr d
[ 	]+[0-9a-f]+:[ 	]+cb 1b[ 	]+rr e
[ 	]+[0-9a-f]+:[ 	]+cb 1c[ 	]+rr h
[ 	]+[0-9a-f]+:[ 	]+cb 1d[ 	]+rr l
[ 	]+[0-9a-f]+:[ 	]+cb 1e[ 	]+rr \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 1e[ 	]+rr \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 1e[ 	]+rr \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 27[ 	]+sla a
[ 	]+[0-9a-f]+:[ 	]+cb 20[ 	]+sla b
[ 	]+[0-9a-f]+:[ 	]+cb 21[ 	]+sla c
[ 	]+[0-9a-f]+:[ 	]+cb 22[ 	]+sla d
[ 	]+[0-9a-f]+:[ 	]+cb 23[ 	]+sla e
[ 	]+[0-9a-f]+:[ 	]+cb 24[ 	]+sla h
[ 	]+[0-9a-f]+:[ 	]+cb 25[ 	]+sla l
[ 	]+[0-9a-f]+:[ 	]+cb 26[ 	]+sla \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 26[ 	]+sla \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 26[ 	]+sla \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 2f[ 	]+sra a
[ 	]+[0-9a-f]+:[ 	]+cb 28[ 	]+sra b
[ 	]+[0-9a-f]+:[ 	]+cb 29[ 	]+sra c
[ 	]+[0-9a-f]+:[ 	]+cb 2a[ 	]+sra d
[ 	]+[0-9a-f]+:[ 	]+cb 2b[ 	]+sra e
[ 	]+[0-9a-f]+:[ 	]+cb 2c[ 	]+sra h
[ 	]+[0-9a-f]+:[ 	]+cb 2d[ 	]+sra l
[ 	]+[0-9a-f]+:[ 	]+cb 2e[ 	]+sra \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 2e[ 	]+sra \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 2e[ 	]+sra \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+cb 3f[ 	]+srl a
[ 	]+[0-9a-f]+:[ 	]+cb 38[ 	]+srl b
[ 	]+[0-9a-f]+:[ 	]+cb 39[ 	]+srl c
[ 	]+[0-9a-f]+:[ 	]+cb 3a[ 	]+srl d
[ 	]+[0-9a-f]+:[ 	]+cb 3b[ 	]+srl e
[ 	]+[0-9a-f]+:[ 	]+cb 3c[ 	]+srl h
[ 	]+[0-9a-f]+:[ 	]+cb 3d[ 	]+srl l
[ 	]+[0-9a-f]+:[ 	]+cb 3e[ 	]+srl \(hl\)
[ 	]+[0-9a-f]+:[ 	]+dd cb 05 3e[ 	]+srl \(ix\+5\)
[ 	]+[0-9a-f]+:[ 	]+fd cb 05 3e[ 	]+srl \(iy\+5\)
[ 	]+[0-9a-f]+:[ 	]+07[ 	]+rlca
[ 	]+[0-9a-f]+:[ 	]+0f[ 	]+rrca
[ 	]+[0-9a-f]+:[ 	]+17[ 	]+rla
[ 	]+[0-9a-f]+:[ 	]+1f[ 	]+rra
[ 	]+[0-9a-f]+:[ 	]+ed 6f[ 	]+rld
[ 	]+[0-9a-f]+:[ 	]+ed 67[ 	]+rrd
#pass