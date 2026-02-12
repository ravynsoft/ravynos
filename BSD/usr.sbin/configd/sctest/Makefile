#
# Makefile for generating the SCTestOptions.[mh] files
#

all: SCTestOptions.h SCTestOptions.m

/tmp/genSCTestOptions: genSCTestOptions.c Makefile
	cc -g -o /tmp/genSCTestOptions genSCTestOptions.c

SCTestOptions.h: /tmp/genSCTestOptions
	/tmp/genSCTestOptions header > SCTestOptions.h

SCTestOptions.m: /tmp/genSCTestOptions
	/tmp/genSCTestOptions mfile > SCTestOptions.m