#!/bin/sh

# Test a CP1256 locale.
${CHECKER} ./test-c32rtomb-w32${EXEEXT} "Arabic_Saudi Arabia" 1256
