# Bifrost compiler

## Register file

Defined partially in software, partially in hardware.

## Blend shaders

R0 - R3: input (color #0)
R4 - R7: input (color #1)
R8 - R15: general purpose
R48: return address

## Fragment

Anything live during BLEND must respect blend shader registers.

R0 - R3: preloaded (message #0)
R4 - R7: preloaded (message #1)
R57 - R63: preloaded (various)

R0 - R15: general purpose (full threads)
R48 - R63: general purpose (full threads)

R32 - R47: general purpose (half threads, or v6)
