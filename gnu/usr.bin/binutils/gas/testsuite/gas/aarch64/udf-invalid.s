// Instructions in this file are invalid.
// See udf.s for valid instructions.
.text
udf #65536
udf 0xeffff
udf -1
