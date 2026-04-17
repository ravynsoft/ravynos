#!/bin/bash
#
# Run code coverage on fsck_msdos
#
# To use this script, you must have already built fsck_msdos using the
# CodeCoverage configuration.  You can do that by:
#	xcodebuild -target fsck_msdos -configuration CodeCoverage
#

OBJECTS=build/msdosfs.build/CodeCoverage/fsck_msdos.build/Objects-normal/`arch`

# Remove any pre-existing samples
rm -f $OBJECTS/*.gcda *.gcov

# Run the tests
python test_fsck.py build/CodeCoverage/fsck_msdos || exit

# Run gcov on each source file
for FILE in fsck_msdos.tproj/*.c; do
	gcov -o $OBJECTS $FILE
done

# Produce summaries for each file
for FILE in *.gcov; do
	cat $FILE | awk 'BEGIN {total=0; covered=0}
		{if($1 != "-:") { total++; if($1 != "#####:") covered++}}
		END {printf("%f%% of %d lines covered", 100 * covered/total, total)}'
	echo " for ${FILE%.gcov}"
done

# Produce an overall percentage of coverage
find . -name "*.gcov" | xargs cat | awk 'BEGIN {total=0; covered=0}
	{if($1 != "-:") { total++; if($1 != "#####:") covered++}}
	END {printf("Total: %f%% of %d lines covered\n", 100 * covered/total, total)}'
