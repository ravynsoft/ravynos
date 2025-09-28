        LOOP .bug LC0 = P0>>1;
        LOOP_BEGIN .bug;             
                R0 = [P3+_foo@GOT17M4];
        LOOP_END .bug;
        call  _bar;
