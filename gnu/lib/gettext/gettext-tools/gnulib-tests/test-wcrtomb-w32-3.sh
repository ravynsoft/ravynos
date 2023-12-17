#!/bin/sh

# Test a CP1256 locale.
${CHECKER} ./test-wcrtomb-w32${EXEEXT} "Arabic_Saudi Arabia" 1256
