#!/bin/sh

# Test a CP950 locale.
${CHECKER} ./test-wcrtomb-w32${EXEEXT} Chinese_Taiwan 950
