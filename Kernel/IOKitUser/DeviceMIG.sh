# Copyright (c) 2006, 2009 Apple Inc. All rights reserved.
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

# This script is run from the shell script build phases in the DeviceMIG target

MIGCC=`xcodebuild -sdk "${SDKROOT}" -find cc`
MIG=`xcodebuild -sdk "${SDKROOT}" -find mig`

if [ install == "$1" ]
then
    if [ $SCRIPT_INPUT_FILE_0 -nt $SCRIPT_OUTPUT_FILE_0 ]
    then
		echo Creating 32 bit Mig header for backward compatibility
		cat > $SCRIPT_OUTPUT_FILE_0 <<- EOI_iokitmig
			#if !defined(__LP64__)

			`cat $SCRIPT_INPUT_FILE_0`

			#endif /* !__LP64__ */
		EOI_iokitmig
    fi
    exit
fi

# This script generates the device.defs mig interface for the IOKit.framework to the kernel
runMig()
{
    local input=$1 head=$2 user=$3; shift 3
	migargs=$@
	set -- $ARCHS
	MIGARCH=$1; shift
	cmd="$MIG -cc $MIGCC -arch $MIGARCH ${migargs} -novouchers -server /dev/null -header $head -user $user $input";
    echo $cmd
    eval $cmd
}

# which input files is newest.
if [ $SCRIPT_INPUT_FILE_0 -nt $SCRIPT_INPUT_FILE_1 ]
then
    testFile=$SCRIPT_INPUT_FILE_0
else
    testFile=$SCRIPT_INPUT_FILE_1
fi

ARCHS32=""
ARCHS64=""
for a in $ARCHS; do
    case $a in
	*64)
	    ARCHS64="$ARCHS64 $a"
	    ;;
	*64?)
	    ARCHS64="$ARCHS64 $a"
	    ;;
	*)
	    ARCHS32="$ARCHS32 $a"
	    ;;
    esac
done

echo "Inferred 32-bit architectures: $ARCHS32"
echo "Inferred 64-bit architectures: $ARCHS64"

if [ $testFile -nt $SCRIPT_OUTPUT_FILE_0 -o $testFile -nt $SCRIPT_OUTPUT_FILE_1 \
  -o $testFile -nt $SCRIPT_OUTPUT_FILE_2 -o $testFile -nt $SCRIPT_OUTPUT_FILE_3 ]
then
    if [ -n "${ARCHS32}" ]
    then
        ARCHS=${ARCHS32}
        runMig $SCRIPT_INPUT_FILE_0 $SCRIPT_OUTPUT_FILE_0 $SCRIPT_OUTPUT_FILE_1 $OTHER_CFLAGS
    fi

	if [ -n "${ARCHS64}" ]
	then
		ARCHS=${ARCHS64}
		OTHER_CFLAGS="$OTHER_CFLAGS -D__LP64__"
		runMig $SCRIPT_INPUT_FILE_0 $SCRIPT_OUTPUT_FILE_2 $SCRIPT_OUTPUT_FILE_3 $OTHER_CFLAGS
	fi
fi
