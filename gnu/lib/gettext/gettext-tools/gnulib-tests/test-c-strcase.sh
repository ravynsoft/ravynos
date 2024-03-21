#!/bin/sh

# Test in the C locale.
${CHECKER} ./test-c-strcasecmp${EXEEXT} || exit 1
${CHECKER} ./test-c-strncasecmp${EXEEXT} || exit 1

# Test in an ISO-8859-1 or ISO-8859-15 locale.
: "${LOCALE_FR=fr_FR}"
if test $LOCALE_FR != none; then
  LC_ALL=$LOCALE_FR ${CHECKER} ./test-c-strcasecmp${EXEEXT} locale || exit 1
  LC_ALL=$LOCALE_FR ${CHECKER} ./test-c-strncasecmp${EXEEXT} locale || exit 1
fi

# Test in a Turkish UTF-8 locale.
: "${LOCALE_TR_UTF8=tr_TR.UTF-8}"
if test $LOCALE_TR_UTF8 != none; then
  LC_ALL=$LOCALE_TR_UTF8 ${CHECKER} ./test-c-strcasecmp${EXEEXT} locale || exit 1
  LC_ALL=$LOCALE_TR_UTF8 ${CHECKER} ./test-c-strncasecmp${EXEEXT} locale || exit 1
fi

exit 0
