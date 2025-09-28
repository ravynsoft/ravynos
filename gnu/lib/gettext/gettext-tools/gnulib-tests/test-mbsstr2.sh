#!/bin/sh

# Test whether a specific UTF-8 locale is installed.
: "${LOCALE_FR_UTF8=fr_FR.UTF-8}"
if test $LOCALE_FR_UTF8 = none; then
  if test -f /usr/bin/localedef; then
    echo "Skipping test: no french Unicode locale is installed"
  else
    echo "Skipping test: no french Unicode locale is supported"
  fi
  exit 77
fi

LC_ALL=$LOCALE_FR_UTF8 \
${CHECKER} ./test-mbsstr2${EXEEXT}
