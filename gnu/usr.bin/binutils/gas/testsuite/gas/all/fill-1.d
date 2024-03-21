#objdump: -s -j .data -j "\$DATA\$"
#name: fill test with forward labels
# The following targets do not define DIFF_EXPR_OK and so the
# .fill expression cannot be calculated at assembly time:
#notarget: tic4x-*-* tic54x-*-* mep-*-*

.*: +file format .*

Contents of section (\.data|\$DATA\$):
 [^ ]* 0a0a0d0d 0b0b0c0c .*
