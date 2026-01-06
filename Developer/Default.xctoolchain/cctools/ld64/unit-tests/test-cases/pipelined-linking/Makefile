##
# Copyright (c) 2012 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
##
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Validate these pipelined linking cases:
# - missing pipe
# - bad file passed via pipe
# - eof on pipe
# - success case
# - differing file orders produce same binary
#
# When testing the success case take care to verify that pipelined linking is actually
# enabled (environment variable is set). Because there is no difference in the command
# line the test will still succeed with pipelined linking turned off, without testing
# pipelined linking.
#

all:
	# Make some object files
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	${CC} ${CCFLAGS} cat.c -c -o cat.o
	ls *.o > filelist

	# missing fifo
	(LD_PIPELINE_FIFO=bad_fifo ; export LD_PIPELINE_FIFO ; ${CC} ${CCFLAGS} -filelist filelist -o partial ; exit 0) 2>&1 | ${PASS_IFF_STDIN}

	# partial file list passed via pipe
	head -1 filelist > pipelineFile
	(LD_PIPELINE_FIFO=pipelineFile ; export LD_PIPELINE_FIFO ; ${CC} ${CCFLAGS} -filelist filelist -o partial ; exit 0) 2>&1 | ${PASS_IFF_STDIN}
	${FAIL_IFF} ${MACHOCHECK} partial 2>&1 > /dev/null

	# bogus file passed via pipe
	echo "bogus_file.o" > pipelineFile
	(LD_PIPELINE_FIFO=pipelineFile ; export LD_PIPELINE_FIFO ; ${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -filelist filelist -o bogus ; exit 0) 2>&1 | ${PASS_IFF_STDIN}
	${FAIL_IFF} ${MACHOCHECK} bogus 2>&1 > /dev/null

	# success cases - check different orderings of files
	sort < filelist > pipelineFile
	(LD_PIPELINE_FIFO=pipelineFile ; export LD_PIPELINE_FIFO ; ${CC} ${CCFLAGS} -filelist filelist -o normal)
	${FAIL_IF_BAD_MACHO} normal
	sort -r < filelist > pipelineFile
	(LD_PIPELINE_FIFO=pipelineFile ; export LD_PIPELINE_FIFO ; ${CC} ${CCFLAGS} -filelist filelist -o reverse)
	${FAIL_IF_BAD_MACHO} reverse

	# verify that the same binary was generated using both forward and reverse ordering
	${PASS_IFF} cmp reverse normal


clean:
	rm -f *.o filelist partial bogus reverse normal pipelineFile

