#!/bin/sh

# Test a CP950 locale.
${CHECKER} ./test-mbrtoc32-w32${EXEEXT} Chinese_Taiwan 950
