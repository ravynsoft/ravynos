#!/bin/sh

# Test whether a specific EUC-JP locale is installed.
: "${LOCALE_JA=ja_JP}"
if test $LOCALE_JA = none; then
  if test -f /usr/bin/localedef; then
    echo "Skipping test: no traditional japanese locale is installed"
  else
    echo "Skipping test: no traditional japanese locale is supported"
  fi
  exit 77
fi

LC_ALL=$LOCALE_JA \
${CHECKER} ./test-fnmatch${EXEEXT} 4
