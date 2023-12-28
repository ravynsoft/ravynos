# Tests for the JUMP pseudo-C instructions
        .text
        goto 2f
        r1 += r1
1:      if r3 == 3 goto 2f
        if r3 == r4 goto 2f
2:      if r3 >= 3 goto 1b
        if r3 >= r4 goto 1b
1:      if r3 < 3 goto 1f
        if r3 < r4 goto  1f
1:      if r3 <= 3 goto 1f
        if r3 <= r4 goto 1f
1:      if r3 & 3 goto 1f
        if r3 & r4 goto 1f
1:      if r3 != 3 goto 1f
        if r3 != r4 goto 1f
1:      if r3 s> 3 goto 1f
        if r3 s> r4 goto 1f
1:      if r3 s>= 3 goto 1f
        if r3 s>= r4 goto 1f
1:      if r3 s< 3 goto 1f
        if r3 s< r4 goto 1f
1:      if r3 s<= 3 goto 1f
        if r3 s<= r4 goto 1f
1:
