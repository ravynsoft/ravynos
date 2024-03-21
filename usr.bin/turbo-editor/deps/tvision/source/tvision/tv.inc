;/*------------------------------------------------------------*/
;/* filename -       tv.inc                                    */
;/*                                                            */
;/*                  Header for Turbo Vision .asm files        */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

ifdef __FLAT__ ;;;;;;;;;;;;;;;;;;;;;;;; 32-bit ;;;;;;;;;;;;;;;;;;;;;;;

    P386
    MODEL  FLAT, C

        ARGINT   equ DWORD
    Dnear    equ DD
    LocalRet equ db 0C3h
    _FAR   equ


    INCLUDE TVWRIT32.INC


else    ;;;;;;;;;;;;;;;;;;;;;;;; 16-bit ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    MODEL  LARGE, PROLOG

    ARGINT equ WORD
    Dnear  equ DW
    _FAR   equ FAR

    INCLUDE TVWRITE.INC

endif
    LOCALS @@

