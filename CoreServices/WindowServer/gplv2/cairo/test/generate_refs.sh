#!/bin/bash

# A very simple script. But sufficient, hopefully, for our current purposes.

cat <<EOF
# Note REFERENCE_IMAGES must be in lexicographical order.
# Use generate_refs.sh on a git checkout with updated images.
REFERENCE_IMAGES = \\
EOF

git ls-files 'reference/*.ref.png' '*.xfail.png' | sed 's/\(.*\)/	\1 \\/'
echo '	$(NULL)'
