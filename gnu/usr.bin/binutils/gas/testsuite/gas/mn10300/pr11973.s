      .org 0x00000000
      _baseAddress:
      .equ _base40,_baseAddress
      .equ _base7B,_baseAddress+0x3B000000
      .equ sub_7BC01234,_base7B+0xC01234

      .org 0x00000100
      SomeProc_40000100:
      mov 0x123, D0
      call sub_7BC01234, [D2], 0x04


      .org 0x00000200
      SomeProc_40000200:
      mov 0x123, D0
      call sub_7BC01234, [D2], 0x04
      call SomeProc_40000100, [D2], 0x04
