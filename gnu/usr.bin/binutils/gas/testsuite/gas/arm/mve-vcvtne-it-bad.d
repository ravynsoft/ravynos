# name: MVE vcvtne instruction outside of IT block
# as: -march=armv8.1-m.main+mve.fp+fp.dp
# error_output: mve-vcvtne-it-bad.l

.*: +file format .*arm.*
