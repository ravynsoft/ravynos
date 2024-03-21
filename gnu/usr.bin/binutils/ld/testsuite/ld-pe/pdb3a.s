.equ CV_SIGNATURE_C13, 4
.equ DEBUG_S_LINES, 0xf2
.equ DEBUG_S_STRINGTABLE, 0xf3
.equ DEBUG_S_FILECHKSMS, 0xf4
.equ CHKSUM_TYPE_MD5, 1

.equ NUM_MD5_BYTES, 16

.section ".debug$S", "rn"
.long CV_SIGNATURE_C13
.long DEBUG_S_STRINGTABLE
.long .strings_end - .strings_start

.strings_start:

.asciz ""

.src1:
.asciz "foo"

.src2:
.asciz "bar"

.strings_end:

.balign 4

.long DEBUG_S_FILECHKSMS
.long .chksms_end - .chksms_start

.chksms_start:

.long .src1 - .strings_start
.byte NUM_MD5_BYTES
.byte CHKSUM_TYPE_MD5
.long 0x01234567
.long 0x89abcdef
.long 0xfedcba98
.long 0x67452310
.short 0 /* padding */

.long .src2 - .strings_start
.byte NUM_MD5_BYTES
.byte CHKSUM_TYPE_MD5
.long 0xfedcba98
.long 0x67452310
.long 0x01234567
.long 0x89abcdef
.short 0 /* padding */

.chksms_end:

.balign 4

.long DEBUG_S_LINES
.long .lines_end - .lines_start

.lines_start:

.secrel32 main
.secidx main
.short 0 /* flags */
.long .main_end - main /* length of region */

.lines_block1:

.long 0 /* file ID 0 (foo) */
.long 2 /* no. lines */
.long .lines_block2 - .lines_block1 /* length */

.long .line1 - main
.long 0x80000001 /* line 1 */
.long .line2 - main
.long 0x80000002 /* line 2 */

.lines_block2:

.long 0x18 /* file ID 18 (bar) */
.long 2 /* no. lines */
.long .lines_block3 - .lines_block2 /* length */

.long .line3 - main
.long 0x80000003 /* line 3 */
.long .line4 - main
.long 0x80000004 /* line 4 */

.lines_block3:

.long 0 /* file ID 0 (foo) */
.long 1 /* no. lines */
.long .lines_end - .lines_block3 /* length */

.long .line5 - main
.long 0x80000005 /* line 5 */

.lines_end:

.long DEBUG_S_LINES
.long .lines_end2 - .lines_start2

.lines_start2:

.secrel32 gcfunc
.secidx gcfunc
.short 0 /* flags */
.long .gcfunc_end - gcfunc /* length of region */

.lines_block4:

.long 0 /* file ID 0 (foo) */
.long 1 /* no. lines */
.long .lines_end2 - .lines_block4 /* length */

.long .line6 - gcfunc
.long 0x80000006 /* line 6 */

.lines_end2:

.text

.global main
main:
.line1:
	.long 0x12345678
.line2:
	.long 0x12345678
.line3:
	.long 0x12345678
.line4:
	.long 0x12345678
.line5:
	.long 0x12345678
.main_end:

.section "gcsect"

gcfunc:
.line6:
	.long 0x12345678
.gcfunc_end:
