#as: -mcpu=arc700
#target: arc-*
#objdump: -sj .text

.*: +file format .*arc.*

Contents of section .text:
 [0-9a-f]+ ddccbbaa ffee .*
