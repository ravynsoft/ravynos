        r0 <<= 1;
        p0 = 15;
        divs (r0, r1);
        loop 1f lc0=p0;
loop_begin 1;
        divq (r0, r1);
loop_end 1;
        r0 = r0.l (x);
