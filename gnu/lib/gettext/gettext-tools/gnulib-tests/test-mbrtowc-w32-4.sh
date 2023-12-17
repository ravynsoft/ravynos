#!/bin/sh

# Test some UTF-8 locales.
${CHECKER} ./test-mbrtowc-w32${EXEEXT} French_France Japanese_Japan Chinese_Taiwan Chinese_China 65001
