begin:

offsetAtStart:    equ   32

            LD    A, offsetAtStart
            LD    A, (IX + offsetAtStart)

            LD    A, offsetAtEnd
            LD    A, (IX + offsetAtEnd)

            RET

offsetAtEnd:      equ   64
            END

