#as:
#source: enums.c
#objdump: --ctf
#ld: -shared
#name: Enumerated types

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Compilation unit name: .*enums.c
#...
    Type section:	.*\(0x134 bytes\)
#...
  Types:
    0x1: \(kind 8\) enum nine_els \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
         NINE_ONE: 0
         NINE_TWO: 1
         NINE_THREE: 256
         NINE_FOUR: 257
         NINE_FIVE: 258
         NINE_SIX: 259
         NINE_SEVEN: 260
         NINE_EIGHT: 261
         NINE_NINE: 262
    0x2: \(kind 1\) .*int \(format 0x[01]\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]\)
    0x3: \(kind 8\) enum ten_els \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
         TEN_ONE: 10
         TEN_TWO: 11
         TEN_THREE: -256
         TEN_FOUR: -255
         TEN_FIVE: -254
         TEN_SIX: -253
         TEN_SEVEN: -252
         TEN_EIGHT: -251
         TEN_NINE: -250
         TEN_TEN: -249
    0x4: \(kind 1\) .*int \(format 0x[01]\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x5: \(kind 8\) enum eleven_els \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
         ELEVEN_ONE: 10
         ELEVEN_TWO: 11
         ELEVEN_THREE: -256
         ELEVEN_FOUR: -255
         ELEVEN_FIVE: -254
         \.\.\.
         ELEVEN_SEVEN: -252
         ELEVEN_EIGHT: -251
         ELEVEN_NINE: -250
         ELEVEN_TEN: -249
         ELEVEN_ELEVEN: -248

#...
