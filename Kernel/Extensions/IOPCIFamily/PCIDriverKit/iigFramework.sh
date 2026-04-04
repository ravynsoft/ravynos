SOURCE=$1
SDK_NAME=$2
#SDK_NAME=macosx.internal
SDKROOT=$(xcrun --sdk ${SDK_NAME} --show-sdk-path)
IIG=$(xcrun -sdk ${SDK_NAME} -find iig)

FRAMEWORK_NAME=$(basename ${SOURCE})

CFLAGS_sdk_include="-isysroot ${SDKROOT} -iframework ${SDKROOT}/${DRIVERKITROOT}/System/Library/Frameworks -idirafter ${SDKROOT}/${DRIVERKITROOT}/usr/local/include -idirafter ${SDKROOT}/${DRIVERKITROOT}/usr/include"

OTHER_IIG_FLAGS=${OTHER_IIG_FLAGS:-}
OTHER_IIG_CFLAGS=${OTHER_IIG_CFLAGS:-${CFLAGS_sdk_include}}" -x c++ -std=gnu++1z -fno-exceptions -fno-rtti -D__IIG=1 -DPLATFORM_DriverKit=1 ${DEPLOYMENT_TARGET_DEFINES}"

FRAMEWORK=${SOURCE}

shopt -s nullglob
FILES="${FRAMEWORK}/Headers/*.iig ${FRAMEWORK}/PrivateHeaders/*.iig"

for DEF in ${FILES}; do
    ${IIG} --def ${DEF} --header ${DEF%.iig}.h --impl /dev/null ${OTHER_IIG_FLAGS} -- ${OTHER_IIG_CFLAGS};
done

