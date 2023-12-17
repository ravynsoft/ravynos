#!/bin/sh

# Test a CP1252 locale.
${CHECKER} ./test-mbrtowc-w32${EXEEXT} French_France 1252
