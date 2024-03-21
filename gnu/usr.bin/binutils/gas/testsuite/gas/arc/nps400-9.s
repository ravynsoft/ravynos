        .text

        ;; dcmac
        dcmac r0,[cm:r0],[cm:r0],r0
        dcmac r2,[cm:r4],[cm:r4],r7
        dcmac r31,[cm:r31],[cm:r31],r31

        dcmac r0,[cm:r0],[cm:0x0],r0
        dcmac r2,[cm:r4],[cm:0x1234],r7
        dcmac r31,[cm:r31],[cm:0xffff],r31

        dcmac r0,[cm:0x0],[cm:r0],r0
        dcmac r2,[cm:0x4321],[cm:r4],r7
        dcmac r31,[cm:0xffff],[cm:r31],r31

        dcmac 0,[cm:r0],[cm:r0],r0
        dcmac 0,[cm:r4],[cm:r4],r7
        dcmac 0,[cm:r31],[cm:r31],r31

        dcmac 0,[cm:r0],[cm:0x0],r0
        dcmac 0,[cm:r4],[cm:0x1234],r7
        dcmac 0,[cm:r31],[cm:0xffff],r31

        dcmac 0,[cm:0x0],[cm:r0],r0
        dcmac 0,[cm:0x4321],[cm:r4],r7
        dcmac 0,[cm:0xffff],[cm:r31],r31

        dcmac r0,[cm:r0],[cm:r0],1
        dcmac r2,[cm:r4],[cm:r4],0xf
        dcmac r31,[cm:r31],[cm:r31],0x3f

        dcmac r0,[cm:r0],[cm:0x0],1
        dcmac r2,[cm:r4],[cm:0x1234],0xf
        dcmac r31,[cm:r31],[cm:0xffff],0x3f

        dcmac r0,[cm:0x0],[cm:r0],1
        dcmac r2,[cm:0x4321],[cm:r4],0xf
        dcmac r31,[cm:0xffff],[cm:r31],0x3f

        dcmac 0,[cm:r0],[cm:r0],1
        dcmac 0,[cm:r4],[cm:r4],0xf
        dcmac 0,[cm:r31],[cm:r31],64

        dcmac 0,[cm:r0],[cm:0x0],1
        dcmac 0,[cm:r4],[cm:0x1234],0xf
        dcmac 0,[cm:r31],[cm:0xffff],64

        dcmac 0,[cm:0x0],[cm:r0],1
        dcmac 0,[cm:0x4321],[cm:r4],0xf
        dcmac 0,[cm:0xffff],[cm:r31],64

