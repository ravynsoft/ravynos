#source: ../x86-64-fma.s
#objdump: -drwMintel
#name: x86-64 (ILP32) FMA (Intel mode)

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 cd 98 d4       	vfmadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 98 11       	vfmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 98 d4       	vfmadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 98 11       	vfmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a8 d4       	vfmadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a8 11       	vfmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a8 d4       	vfmadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a8 11       	vfmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b8 d4       	vfmadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b8 11       	vfmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b8 d4       	vfmadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b8 11       	vfmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 96 d4       	vfmaddsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 96 11       	vfmaddsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 96 d4       	vfmaddsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 96 11       	vfmaddsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a6 d4       	vfmaddsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a6 11       	vfmaddsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a6 d4       	vfmaddsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a6 11       	vfmaddsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b6 d4       	vfmaddsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b6 11       	vfmaddsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b6 d4       	vfmaddsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b6 11       	vfmaddsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 97 d4       	vfmsubadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 97 11       	vfmsubadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 97 d4       	vfmsubadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 97 11       	vfmsubadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a7 d4       	vfmsubadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a7 11       	vfmsubadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a7 d4       	vfmsubadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a7 11       	vfmsubadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b7 d4       	vfmsubadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b7 11       	vfmsubadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b7 d4       	vfmsubadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b7 11       	vfmsubadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9a d4       	vfmsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9a 11       	vfmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9a d4       	vfmsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9a 11       	vfmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd aa d4       	vfmsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd aa 11       	vfmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d aa d4       	vfmsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d aa 11       	vfmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ba d4       	vfmsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ba 11       	vfmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ba d4       	vfmsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ba 11       	vfmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9c d4       	vfnmadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9c 11       	vfnmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9c d4       	vfnmadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9c 11       	vfnmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ac d4       	vfnmadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ac 11       	vfnmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ac d4       	vfnmadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ac 11       	vfnmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bc d4       	vfnmadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd bc 11       	vfnmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bc d4       	vfnmadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d bc 11       	vfnmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9e d4       	vfnmsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9e 11       	vfnmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9e d4       	vfnmsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9e 11       	vfnmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ae d4       	vfnmsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ae 11       	vfnmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ae d4       	vfnmsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ae 11       	vfnmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd be d4       	vfnmsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd be 11       	vfnmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d be d4       	vfnmsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d be 11       	vfnmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 98 d4       	vfmadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 98 39       	vfmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 98 d4       	vfmadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 98 39       	vfmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a8 d4       	vfmadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a8 39       	vfmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a8 d4       	vfmadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a8 39       	vfmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b8 d4       	vfmadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b8 39       	vfmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b8 d4       	vfmadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b8 39       	vfmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 96 d4       	vfmaddsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 96 39       	vfmaddsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 96 d4       	vfmaddsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 96 39       	vfmaddsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a6 d4       	vfmaddsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a6 39       	vfmaddsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a6 d4       	vfmaddsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a6 39       	vfmaddsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b6 d4       	vfmaddsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b6 39       	vfmaddsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b6 d4       	vfmaddsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b6 39       	vfmaddsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 97 d4       	vfmsubadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 97 39       	vfmsubadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 97 d4       	vfmsubadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 97 39       	vfmsubadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a7 d4       	vfmsubadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a7 39       	vfmsubadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a7 d4       	vfmsubadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a7 39       	vfmsubadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b7 d4       	vfmsubadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b7 39       	vfmsubadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b7 d4       	vfmsubadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b7 39       	vfmsubadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9a d4       	vfmsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9a 39       	vfmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9a d4       	vfmsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9a 39       	vfmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 aa d4       	vfmsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 aa 39       	vfmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 aa d4       	vfmsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 aa 39       	vfmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ba d4       	vfmsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ba 39       	vfmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ba d4       	vfmsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ba 39       	vfmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9c d4       	vfnmadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9c 39       	vfnmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9c d4       	vfnmadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9c 39       	vfnmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ac d4       	vfnmadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ac 39       	vfnmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ac d4       	vfnmadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ac 39       	vfnmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bc d4       	vfnmadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bc 39       	vfnmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bc d4       	vfnmadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bc 39       	vfnmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9e d4       	vfnmsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9e 39       	vfnmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9e d4       	vfnmsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9e 39       	vfnmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ae d4       	vfnmsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ae 39       	vfnmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ae d4       	vfnmsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ae 39       	vfnmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 be d4       	vfnmsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 be 39       	vfnmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 be d4       	vfnmsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 be 39       	vfnmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 99 d4       	vfmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a9 d4       	vfmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b9 d4       	vfmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9b d4       	vfmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ab d4       	vfmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bb d4       	vfmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9d d4       	vfnmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ad d4       	vfnmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bd d4       	vfnmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9f d4       	vfnmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 af d4       	vfnmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bf d4       	vfnmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 99 d4       	vfmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a9 d4       	vfmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b9 d4       	vfmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9b d4       	vfmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ab d4       	vfmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bb d4       	vfmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9d d4       	vfnmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ad d4       	vfnmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bd d4       	vfnmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9f d4       	vfnmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 af d4       	vfnmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bf d4       	vfnmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 98 d4       	vfmadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 98 11       	vfmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 98 11       	vfmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 98 d4       	vfmadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 98 11       	vfmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 98 11       	vfmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a8 d4       	vfmadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a8 11       	vfmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a8 11       	vfmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a8 d4       	vfmadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a8 11       	vfmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a8 11       	vfmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b8 d4       	vfmadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b8 11       	vfmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b8 11       	vfmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b8 d4       	vfmadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b8 11       	vfmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b8 11       	vfmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 96 d4       	vfmaddsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 96 11       	vfmaddsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 96 11       	vfmaddsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 96 d4       	vfmaddsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 96 11       	vfmaddsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 96 11       	vfmaddsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a6 d4       	vfmaddsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a6 11       	vfmaddsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a6 11       	vfmaddsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a6 d4       	vfmaddsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a6 11       	vfmaddsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a6 11       	vfmaddsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b6 d4       	vfmaddsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b6 11       	vfmaddsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b6 11       	vfmaddsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b6 d4       	vfmaddsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b6 11       	vfmaddsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b6 11       	vfmaddsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 97 d4       	vfmsubadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 97 11       	vfmsubadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 97 11       	vfmsubadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 97 d4       	vfmsubadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 97 11       	vfmsubadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 97 11       	vfmsubadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a7 d4       	vfmsubadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd a7 11       	vfmsubadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a7 11       	vfmsubadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a7 d4       	vfmsubadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d a7 11       	vfmsubadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a7 11       	vfmsubadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b7 d4       	vfmsubadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd b7 11       	vfmsubadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b7 11       	vfmsubadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b7 d4       	vfmsubadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d b7 11       	vfmsubadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b7 11       	vfmsubadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9a d4       	vfmsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9a 11       	vfmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9a 11       	vfmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9a d4       	vfmsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9a 11       	vfmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9a 11       	vfmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd aa d4       	vfmsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd aa 11       	vfmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd aa 11       	vfmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d aa d4       	vfmsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d aa 11       	vfmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d aa 11       	vfmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ba d4       	vfmsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ba 11       	vfmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ba 11       	vfmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ba d4       	vfmsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ba 11       	vfmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ba 11       	vfmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9c d4       	vfnmadd132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9c 11       	vfnmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9c 11       	vfnmadd132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9c d4       	vfnmadd132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9c 11       	vfnmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9c 11       	vfnmadd132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ac d4       	vfnmadd213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ac 11       	vfnmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ac 11       	vfnmadd213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ac d4       	vfnmadd213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ac 11       	vfnmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ac 11       	vfnmadd213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bc d4       	vfnmadd231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd bc 11       	vfnmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bc 11       	vfnmadd231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bc d4       	vfnmadd231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d bc 11       	vfnmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bc 11       	vfnmadd231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9e d4       	vfnmsub132pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9e 11       	vfnmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9e 11       	vfnmsub132pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9e d4       	vfnmsub132ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9e 11       	vfnmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9e 11       	vfnmsub132ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ae d4       	vfnmsub213pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd ae 11       	vfnmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ae 11       	vfnmsub213pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ae d4       	vfnmsub213ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d ae 11       	vfnmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ae 11       	vfnmsub213ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd be d4       	vfnmsub231pd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 cd be 11       	vfnmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 cd be 11       	vfnmsub231pd ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d be d4       	vfnmsub231ps ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d be 11       	vfnmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 4d be 11       	vfnmsub231ps ymm2,ymm6,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 98 d4       	vfmadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 98 39       	vfmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 98 39       	vfmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 98 d4       	vfmadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 98 39       	vfmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 98 39       	vfmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a8 d4       	vfmadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a8 39       	vfmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a8 39       	vfmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a8 d4       	vfmadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a8 39       	vfmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a8 39       	vfmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b8 d4       	vfmadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b8 39       	vfmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b8 39       	vfmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b8 d4       	vfmadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b8 39       	vfmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b8 39       	vfmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 96 d4       	vfmaddsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 96 39       	vfmaddsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 96 39       	vfmaddsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 96 d4       	vfmaddsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 96 39       	vfmaddsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 96 39       	vfmaddsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a6 d4       	vfmaddsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a6 39       	vfmaddsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a6 39       	vfmaddsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a6 d4       	vfmaddsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a6 39       	vfmaddsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a6 39       	vfmaddsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b6 d4       	vfmaddsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b6 39       	vfmaddsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b6 39       	vfmaddsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b6 d4       	vfmaddsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b6 39       	vfmaddsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b6 39       	vfmaddsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 97 d4       	vfmsubadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 97 39       	vfmsubadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 97 39       	vfmsubadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 97 d4       	vfmsubadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 97 39       	vfmsubadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 97 39       	vfmsubadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a7 d4       	vfmsubadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a7 39       	vfmsubadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a7 39       	vfmsubadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a7 d4       	vfmsubadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a7 39       	vfmsubadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a7 39       	vfmsubadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b7 d4       	vfmsubadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b7 39       	vfmsubadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b7 39       	vfmsubadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b7 d4       	vfmsubadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b7 39       	vfmsubadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b7 39       	vfmsubadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9a d4       	vfmsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9a 39       	vfmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9a 39       	vfmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9a d4       	vfmsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9a 39       	vfmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9a 39       	vfmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 aa d4       	vfmsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 aa 39       	vfmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 aa 39       	vfmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 aa d4       	vfmsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 aa 39       	vfmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 aa 39       	vfmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ba d4       	vfmsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ba 39       	vfmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ba 39       	vfmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ba d4       	vfmsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ba 39       	vfmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ba 39       	vfmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9c d4       	vfnmadd132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9c 39       	vfnmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9c 39       	vfnmadd132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9c d4       	vfnmadd132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9c 39       	vfnmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9c 39       	vfnmadd132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ac d4       	vfnmadd213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ac 39       	vfnmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ac 39       	vfnmadd213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ac d4       	vfnmadd213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ac 39       	vfnmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ac 39       	vfnmadd213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bc d4       	vfnmadd231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bc 39       	vfnmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bc 39       	vfnmadd231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bc d4       	vfnmadd231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bc 39       	vfnmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bc 39       	vfnmadd231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9e d4       	vfnmsub132pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9e 39       	vfnmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9e 39       	vfnmsub132pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9e d4       	vfnmsub132ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9e 39       	vfnmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9e 39       	vfnmsub132ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ae d4       	vfnmsub213pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ae 39       	vfnmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ae 39       	vfnmsub213pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ae d4       	vfnmsub213ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ae 39       	vfnmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ae 39       	vfnmsub213ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 be d4       	vfnmsub231pd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 be 39       	vfnmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 be 39       	vfnmsub231pd xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 be d4       	vfnmsub231ps xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 be 39       	vfnmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 be 39       	vfnmsub231ps xmm7,xmm6,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 99 d4       	vfmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a9 d4       	vfmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b9 d4       	vfmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9b d4       	vfmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ab d4       	vfmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bb d4       	vfmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9d d4       	vfnmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ad d4       	vfnmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bd d4       	vfnmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9f d4       	vfnmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 af d4       	vfnmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bf d4       	vfnmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 c9 bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 c9 bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 99 d4       	vfmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a9 d4       	vfmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b9 d4       	vfmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9b d4       	vfmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ab d4       	vfmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bb d4       	vfmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9d d4       	vfnmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ad d4       	vfnmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bd d4       	vfnmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9f d4       	vfnmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 af d4       	vfnmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bf d4       	vfnmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 49 bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	c4 e2 49 bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[rcx\]
#pass
