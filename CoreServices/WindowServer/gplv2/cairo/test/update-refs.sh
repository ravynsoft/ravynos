#!/bin/bash

# This script can be used to update the reference images using certain
# test results as a baseline.
#
# Our test suite expects nearly pixel-perfection, but in some cases we
# give the renderer some flexibility and so these cases will show up as
# test failures.  So, this script can be used to do a visual check and
# if they "look" ok, to go ahead and update the reference image by
# copying the test output.
#
# NOTE: When adding to this file, make sure to thoroughly document the
# rationale when and why the existing reference images can be updated
# from regular test output, such that people other than you can
# intelligently keep the test reference images updated.

if [ ! -d output ] || [ ! -d reference ]; then
    echo "This script must be run in cairo's test directory after the full testsuite has been run."
    exit
fi

PDIFF="./pdiff/perceptualdiff"

# Returns 1 if images are different, 0 if they're essentially identical
images_differ() {
    # Check if bytewise identical
    if cmp --silent "${1}" "${2}"; then
	# Images are identical
	return 0
    fi

    # Run perceptualdiff with minimum threshold
    pdiff_output=$($PDIFF "${1}" "${2}" -threshold 1)
    result=${pdiff_output%:*}
    notes=$(echo "${pdiff_output#*: }" | tail -n 1)
    if [ "$result" = "PASS" ] && [ "$notes" = "Images are binary identical" ]; then
	return 0
    fi

    return 1
}

# ----------------------------------------------------------------------
# pixman-downscale images
#
# The *-95 tests check rendering at a difficult to downsize dimension.
# The border pixels between different colored areas can be blurred in
# different ways resulting in some color variation that is acceptable
# but throws off the testsuite.  So a visual check is sufficient to
# verify the results aren't crazily off.

# Use the ARGB32 format of the image file as the main reference
for file in $(ls ./output/pixman-downscale-*-95.image.argb32.out.png); do
    dest=$(basename ${file/.image.argb32.out./.ref.})
    echo "$file -> ./reference/${dest}"
    cp $file ./reference/${dest}
done
echo

# If the ARGB32 format of a given backend's file differs from the main reference,
# then use it as the backend reference
for file in $(ls ./output/pixman-downscale-*-95.*.argb32.out.png); do
    ref=$(basename ${file/-95.*.argb32.out.png/-95.ref.png})
    if ! images_differ "./reference/${ref}" "${file}"; then
	dest=$(basename ${file/.argb32.out.png/.ref.png})
	echo "${file} -> ./reference/${dest}"
	cp ${file} ./reference/${dest}
    fi
done
echo

# If the RGB24 format differs from existing ref image, then use it as a ref.
for file in $(ls ./output/pixman-downscale-*-95.*.rgb24.out.png); do
    ref=$(basename ${file/.rgb24.out.png/.ref.png})
    if [ ! -e "./reference/${ref}" ]; then
	ref=$(basename ${file/-95.*.rgb24.out.png/-95.ref.png})
    fi

    if ! images_differ "./reference/${ref}" "${file}"; then
	dest=$(basename ${file/.rgb24.out.png/.rgb24.ref.png})
	echo "${file} -> ./reference/${dest}"
	cp ${file} ./reference/${dest}
    fi
done
