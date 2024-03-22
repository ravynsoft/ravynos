(+f0.0) or(8)   g9<1>.wUD       g9<4>.wUD       0x00000040UD    { align16 };
or(8)           g13<1>.xUD      g13<4>.xUD      g14<4>.xUD      { align16 };
or(8)           g3<1>UD         g3<8,8,1>UD     g5<8,8,1>UD     { align1 };
or(16)          g12<1>UD        g14<8,8,1>UD    g20<8,8,1>UD    { align1 compr };
(+f0.0) or(16)  g12<1>UD        g12<8,8,1>UD    0x3f800000UD    { align1 compr };
or(8)           g8<1>.wUD       g11<4>.xUD      g12<4>.xUD      { align16 NoDDChk };
