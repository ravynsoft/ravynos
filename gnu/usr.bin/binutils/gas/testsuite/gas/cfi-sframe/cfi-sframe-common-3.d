#as:
#objdump: --sframe=.sframe
#name: SFrame can co-exist with EH Frame
#...
Contents of the SFrame section .sframe:

  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
    Num FDEs: 1
    Num FREs: 1

  Function Index :
    func idx \[0\]: pc = 0x0, size = 0 bytes
    STARTPC + CFA + FP + RA +
#pass
