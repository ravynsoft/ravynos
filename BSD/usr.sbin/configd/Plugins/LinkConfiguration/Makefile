all : linkconfig

linkconfig.o: linkconfig.c Makefile
	cc -I../../SystemConfiguration.fproj -Wall -g -DMAIN -O0 -c linkconfig.c

linkconfig: linkconfig.o Makefile
#	cc -o linkconfig linkconfig.c -framework CoreFoundation -framework SystemConfiguration -framework IOKit
	cc -o linkconfig linkconfig.o -framework CoreFoundation -framework SystemConfiguration

clean:
	rm -rf *.o linkconfig linkconfig.dSYM

