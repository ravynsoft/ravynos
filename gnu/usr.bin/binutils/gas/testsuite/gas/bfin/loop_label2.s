        r0 <<= 1;
        p0 = 15;
        divs (r0, r1);
        loop .Lfoo lc0=p0;
loop_begin .Lfoo;
        divq (r0, r1);
loop_end .Lfoo;
        r0 = r0.l (x);
