#as: -mcpu=arc700
#target: arceb-*
#objdump: -sj .text
#source: inline-data-1.s

.*: +file format .*arc.*

Contents of section .text:
 [0-9a-f]+ aabbccdd eeff .*
