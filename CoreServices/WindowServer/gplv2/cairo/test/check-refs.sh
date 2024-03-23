#!/bin/bash

cd $(dirname $0)/reference || exit

pdiff=$1
[ -n "$pdiff" ] || pdiff=../pdiff/perceptualdiff
if [ ! -e "${pdiff}" ]; then
    echo "Error:  requires pdiff executable"
    exit 128
fi

exit_code=0

for file in *.ref.png; do
    test=$(echo $file | cut -d'.' -f1)
    target=$(echo $file | cut -d'.' -f2)
    format=$(echo $file | cut -d'.' -f3)
    notes=""
    ref=""
    result=""

    if [ $target = 'base' ]; then
        # Ignore the base images for this script's purposes
        continue
    elif [ $target = 'ref' ]; then
        # This is actually the baseline reference image
        continue
    elif [ $format = 'ref' ]; then
        # This is either a format-specific reference, or a target-specific/format-generic image
        # In either case, compare it against the generic reference image
        ref="$test.ref.png"
    else
        # Prefer the target-specific/format-generic reference image, if available
	ref="$test.$target.ref.png"
	if [ ! -e $ref ]; then
            ref="$test.$format.ref.png"
	fi
    fi

    # Special cases
    if [ $test = "create-from-png" ]; then
	# The create-from-png test utilizes multiple reference images directly
	continue
    elif [ $test = "fallback-resolution" ]; then
	# The fallback-resolution test generates a set of reference images;
	# These won't be redundant with one another, but just ignore them all.
	continue
    fi

    if [ -e $ref ]; then
	if cmp --silent "$ref" "$file" ; then
	    printf "redundant: %s and %s are byte-by-byte identical files\n" $file $ref
	    exit_code=1
	else
            # Run perceptualdiff with minimum threshold
            pdiff_output=$($pdiff $ref $file -threshold 1)
            result=${pdiff_output%:*}
            notes=$(echo "${pdiff_output#*: }" | tail -n 1)
            if [ "$result" = "PASS" ] && [ "$notes" = "Images are binary identical" ]; then
		printf "redundant: %s and %s are pixel equivalent images\n" $file $ref
		exit_code=1
		notes=""
            fi
	fi
    fi

done

exit $exit_code
