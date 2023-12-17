#!/bin/sh

: "${LOCALE_FR=fr_FR}"
: "${LOCALE_FR_UTF8=fr_FR.UTF-8}"
: "${LOCALE_JA=ja_JP}"
: "${LOCALE_ZH_CN=zh_CN.GB18030}"

if test $LOCALE_FR = none && test $LOCALE_FR_UTF8 = none \
   && test $LOCALE_JA = none && test $LOCALE_ZH_CN = none; then
  if test -f /usr/bin/localedef; then
    echo "Skipping test: no locale for testing is installed"
  else
    echo "Skipping test: no locale for testing is supported"
  fi
  exit 77
fi

if test $LOCALE_FR != none; then
  LC_ALL=$LOCALE_FR      ${CHECKER} ./test-setlocale1${EXEEXT} || exit 1
fi

if test $LOCALE_FR_UTF8 != none; then
  LC_ALL=$LOCALE_FR_UTF8 ${CHECKER} ./test-setlocale1${EXEEXT} || exit 1
fi

if test $LOCALE_JA != none; then
  LC_ALL=$LOCALE_JA      ${CHECKER} ./test-setlocale1${EXEEXT} || exit 1
fi

if test $LOCALE_ZH_CN != none; then
  LC_ALL=$LOCALE_ZH_CN   ${CHECKER} ./test-setlocale1${EXEEXT} || exit 1
fi

exit 0
