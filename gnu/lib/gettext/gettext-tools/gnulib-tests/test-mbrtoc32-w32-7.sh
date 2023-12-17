#!/bin/sh

# Test a CP936 locale.
${CHECKER} ./test-mbrtoc32-w32${EXEEXT} Chinese_China 936
