#!/bin/sh
#
# Usage: extract-group-names-from-symbols.sh ../symbols
#
# Extract the Group1 names from all symbol files in the given directory
#
# Example output:
# us:"Atsina"
# us:"Cherokee"
# us:"Coeur d'Alene Salish"
# us:"Czech, Slovak and German (US)"
# us:"English (3l)"
# us:"English (3l, Chromebook)"
# us:"English (3l, emacs)"
# us:"English (Carpalx)"

pushd $1 > /dev/null
grep 'name\[Group1\]' * | sed 's/[[:space:]]*name\[Group1\].*=[[:space:]]*//;s/;[[:space:]]*$//' | sort
popd > /dev/null
