#!/bin/sh

# Test a CP932 locale.
${CHECKER} ./test-mbrtoc32-w32${EXEEXT} Japanese_Japan 932
