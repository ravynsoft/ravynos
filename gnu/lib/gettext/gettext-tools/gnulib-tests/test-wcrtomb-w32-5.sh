#!/bin/sh

# Test a CP932 locale.
${CHECKER} ./test-wcrtomb-w32${EXEEXT} Japanese_Japan 932
