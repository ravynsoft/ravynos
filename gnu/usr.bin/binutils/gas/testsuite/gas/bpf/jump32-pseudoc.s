# Tests for the eBPF JUMP32 pseudo-C instructions
        .text
        goto 2f
        r1 += r1
1:      if w3 == 3 goto 2f
        if w3 == w4 goto 2f
2:      if w3 >= 3 goto 1b
        if w3 >= w4 goto 1b
1:      if w3 < 3 goto 1f
        if w3 < w4 goto 1f
1:      if w3 <= 3 goto 1f
        if w3 <= w4 goto 1f
1:      if w3 & 3 goto 1f
        if w3 & w4 goto 1f
1:      if w3 != 3 goto 1f
        if w3 != w4 goto 1f
1:      if w3 s> 3 goto 1f
        if w3 s> w4 goto 1f
1:      if w3 s>= 3 goto 1f
        if w3 s>= w4 goto 1f
1:      if w3 s< 3 goto 1f
        if w3 s< w4 goto 1f
1:      if w3 s<= 3 goto 1f
        if w3 s<= w4 goto 1f
1:
