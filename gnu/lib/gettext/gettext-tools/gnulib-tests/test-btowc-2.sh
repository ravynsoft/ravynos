#!/bin/sh

# Test in an ISO-8859-1 or ISO-8859-15 locale.
: "${LOCALE_FR=fr_FR}"
if test $LOCALE_FR = none; then
  if test -f /usr/bin/localedef; then
    echo "Skipping test: no traditional french locale is installed"
  else
    echo "Skipping test: no traditional french locale is supported"
  fi
  exit 77
fi

LC_ALL=$LOCALE_FR \
${CHECKER} ./test-btowc${EXEEXT} 2
