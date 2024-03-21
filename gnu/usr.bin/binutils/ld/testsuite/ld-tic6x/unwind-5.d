#ld: -T discard-unwind.ld
#objdump: -s

.*:     file format.*

# Check we don't crash when discarding unwind info.
#...
