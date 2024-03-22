break(8)        JIP: LABEL1    UIP: LABEL2                    { align16 1Q };
LABEL1:
break(8)        JIP: LABEL2    UIP: LABEL2                    { align1 1Q };
break(16)       JIP: LABEL2    UIP: LABEL2                    { align1 1H };
LABEL2:
(+f0.0) break(8) JIP: LABEL3   UIP: LABEL3                    { align1 1Q };
(+f0.0) break(16) JIP: LABEL3  UIP: LABEL3                    { align1 1H };
(+f0.0.x) break(8) JIP: LABEL3 UIP: LABEL3                    { align16 1Q };
LABEL3:
