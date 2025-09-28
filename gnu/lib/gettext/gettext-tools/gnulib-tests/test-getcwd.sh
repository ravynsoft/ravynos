#!/bin/sh

. "${srcdir=.}/init.sh"; path_prepend_ .

${CHECKER} test-getcwd

Exit $?
