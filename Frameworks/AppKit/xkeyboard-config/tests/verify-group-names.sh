#!/bin/sh

# This script compares the group names that are mentioned in base*.xml with the ones
# that actually exist in the symbol files.  Some differences are okay -- like extra
# quotes or an extra escaping character -- but apart from that they should match.

set -e

pwd="$PWD"
tmpdir=$(mktemp -d xkeyboard-config.XXXX)
scriptdir=$(dirname "$0")
ROOT=$(realpath "$scriptdir/..")

cd "$tmpdir" || exit 1

# temporary files
registry_names=registry_names.lst
group_names=group_names.lst

# Convert base.xml and base.extras.xml to a list of `layout(variant):"blah blah"` lines
xsltproc "$ROOT"/tests/reg2ll.xsl "$ROOT"/rules/base.xml "$ROOT"/rules/base.extras.xml | grep -v sun_type > $registry_names

# Filter out empty lines and the custom layout
grep -v -e '^$' \
        -e '^custom:' $registry_names | \
  sort | \
  uniq > $registry_names.tmp
mv $registry_names.tmp $registry_names

# Now search each symbols file for xkb_symbols "variant" and the description of
# name[Group1]="blah blah" and print out a line `filename(variant):"blah blah"`.
# Ideally that file should then match the base{.extras}.xml extracted names, i.e.
# the two files are in sync.
for sym in "$ROOT"/symbols/*; do
  if [ -f "$sym" ]; then
    id="$(basename "$sym")"
    export id
    gawk 'BEGIN{
  FS = "\"";
  id = ENVIRON["id"];
  isDefault = 0;
  isHwSpecificDefault = 0;
  isUnregistered = 0;
}
/#HW-SPECIFIC/{
  isHwSpecificDefault = 1;
}
/#UNREGISTERED/{
  isUnregistered = 1;
}
/^[[:space:]]*\/\//{
  next
}
/.*default.*/{
  isDefault = 1;
}
/xkb_symbols/{
  variant = $2;
}/^[[:space:]]*name\[[Gg]roup1\][[:space:]]*=/{
  if (isUnregistered == 1) {
    isUnregistered = 0;
  } else if (isDefault == 1)
  {
    printf "%s:\"%s\"\n",id,$2;
    isDefault=0;
  } else
  {
    name=$2;
    if (isHwSpecificDefault == 1) {
      isHwSpecificDefault = 0;
      printf "%s:\"%s\"\n", id, name;
    } else {
      printf "%s(%s):\"%s\"\n", id, variant, name;
    }
  }
}' "$sym"
  fi
done | sort | uniq > $group_names

diff -u $registry_names $group_names
rc=$?

if [ $rc != 0 ] ; then
  echo "Legend: '-' is for rules/base*.xml, '+' is for symbols/*"
fi

cd "$pwd" || exit 1

exit $rc
