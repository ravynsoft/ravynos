#!/bin/sh

# Test a CP1256 locale.
${CHECKER} ./test-mbrtoc32-w32${EXEEXT} "Arabic_Saudi Arabia" 1256
