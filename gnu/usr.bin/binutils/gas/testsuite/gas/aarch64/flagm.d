#name: FLAGM (Condition flag manipulation) feature
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d500401f 	cfinv
.*:	ba0407cf 	rmif	x30, #8, #15
.*:	3a00080d 	setf8	w0
.*:	3a00480d 	setf16	w0
.*:	d500401f 	cfinv
.*:	ba0407cf 	rmif	x30, #8, #15
.*:	3a00080d 	setf8	w0
.*:	3a00480d 	setf16	w0
