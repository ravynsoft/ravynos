# Check illegal 64bit suffer usage in LKGS instructions

    .text
_start:
    lkgsb    %r12     #LKGS
    lkgss    %r12     #LKGS
    lkgsb    (%r9)    #LKGS
    lkgss    (%r9)    #LKGS

    .intel_syntax noprefix
    lkgsb    %r12     #LKGS
    lkgsb    BYTE PTR [r9]    #LKGS
    lkgsd    DWORD PTR [r9]    #LKGS
    lkgsq    QWORD PTR [r9]    #LKGS
