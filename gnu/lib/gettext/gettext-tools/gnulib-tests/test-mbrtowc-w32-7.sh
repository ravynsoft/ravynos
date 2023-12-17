#!/bin/sh

# Test a CP936 locale.
${CHECKER} ./test-mbrtowc-w32${EXEEXT} Chinese_China 936
