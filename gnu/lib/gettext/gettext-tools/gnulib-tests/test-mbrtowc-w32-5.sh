#!/bin/sh

# Test a CP932 locale.
${CHECKER} ./test-mbrtowc-w32${EXEEXT} Japanese_Japan 932
