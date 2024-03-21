#!/bin/sh

# Test whether a specific GB18030 locale is installed.
: "${LOCALE_ZH_CN=zh_CN.GB18030}"
if test $LOCALE_ZH_CN = none; then
  if test -f /usr/bin/localedef; then
    echo "Skipping test: no chinese GB18030 locale is installed"
  else
    echo "Skipping test: no chinese GB18030 locale is supported"
  fi
  exit 77
fi

LC_ALL=$LOCALE_ZH_CN \
${CHECKER} ./test-mbsstr3${EXEEXT}
