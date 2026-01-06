TESTROOT = ../..
include ${TESTROOT}/include/common.makefile

#
# Verify that ld -r -S preserves the __objc_imageinfo section
#

IMAGE_INFO = "__image_info"

ifeq ($(ARCH),x86_64)
	IMAGE_INFO = "__objc_imageinfo"
endif
ifeq ($(FILEARCH),arm)
	IMAGE_INFO = "__objc_imageinfo"
endif



run: all

all:
	${CC} ${CCFLAGS} -gdwarf-2 hello.m -c -o hello.o
	${LD} -r -S hello.o -o hello-r.o
	size -m -l hello-r.o | grep ${IMAGE_INFO} | ${PASS_IFF_STDIN}
	
	

clean:
	rm -rf hello.o hello-r.o 
