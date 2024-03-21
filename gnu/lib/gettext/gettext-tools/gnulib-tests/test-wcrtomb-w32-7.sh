#!/bin/sh

# Test a CP936 locale.
${CHECKER} ./test-wcrtomb-w32${EXEEXT} Chinese_China 936
