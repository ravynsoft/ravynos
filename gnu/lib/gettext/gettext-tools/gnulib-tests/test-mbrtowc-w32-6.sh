#!/bin/sh

# Test a CP950 locale.
${CHECKER} ./test-mbrtowc-w32${EXEEXT} Chinese_Taiwan 950
