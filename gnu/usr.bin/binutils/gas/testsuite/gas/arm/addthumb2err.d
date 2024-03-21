#name: bad Thumb2 Add{S} and Sub{S} instructions
#as: -march=armv7-a
#error_output: addthumb2err.l

# Test some Thumb2 instructions:

.*: +file format .*arm.*
