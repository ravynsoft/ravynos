#!/bin/sh

# Test locale names with likely unsupported encoding in Unix syntax.
for name in ar_SA.ISO-8859-1 fr_FR.CP1251 zh_TW.GB18030 zh_CN.BIG5; do
  env LC_ALL=$name ${CHECKER} ./test-setlocale2${EXEEXT} 1 || exit 1
done

# Test locale names with likely unsupported encoding in native Windows syntax.
for name in "Arabic_Saudi Arabia.1252" "Arabic_Saudi Arabia.65001" \
            French_France.65001 Japanese_Japan.65001 Turkish_Turkey.65001 \
            Chinese_Taiwan.65001 Chinese_China.54936 Chinese_China.65001; do
  # Here we use 'env' to set the LC_ALL environment variable, because on
  # Solaris 11.0, the /bin/sh refuses to do it for Turkish_Turkey.65001.
  env LC_ALL="$name" ${CHECKER} ./test-setlocale2${EXEEXT} 1 || exit 1
done

exit 0
