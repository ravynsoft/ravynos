MK_WERROR = no
FRAMEWORK = HID
FMWK_VERSION = A
SRCS = 	HIDEventService.m \
	HIDEvent+HIDEventFields.m \
	HIDVirtualEventService.m \
	HIDEvent.m \
	HIDManager.m \
	HIDServiceClient.m \
	HIDDevice.m \
	HIDEventSystemClient.m \
	HIDTransaction.m \
	HIDConnection.m \
	HIDSession.m \
	HIDUserDevice.m \
	HIDElement.m \
	HIDEventAccessors.m \
	NSError+IOReturn.m

SLF = /System/Library/Frameworks
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-fblocks -fobjc-arc -framework Foundation -framework CoreFoundation \
	-I${ROOT_SOURCE_DIR}/Frameworks -I${.CURDIR:H} \
	-I${.CURDIR:H}/IOHIDFamily \
	-I${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers \
	-Wno-implicit-function-declaration -Wno-cast-qual \
	-Wno-missing-method-return-type -Wno-nullability-completeness
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}

IOKIT_HDRS = ${RAVYN_SDKROOT}${SLF}/IOKit.framework/Versions/A/Headers
IOKIT_PRIVHDRS = ${RAVYN_SDKROOT}${SLF}/IOKit.framework/Versions/A/PrivateHeaders
KERNEL_HDRS = ${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/Headers
KERNEL_PRIVHDRS = ${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/PrivateHeaders

HID_DK = HIDDriverKit/IOHIDDeviceTypes.h
HID_DK_DST = ${KERNEL_HDRS}/HIDDriverKit

HID_KERNPRIV = IOHIDFamily/IOHIDEventServiceKeys_Private.h \
	IOHIDFamily/IOHIDEventFieldDefs.h \
	IOHIDFamily/IOHIDEventMacroDefs.h \
	IOHIDFamily/IOHIDEventStructDefs.h \
	IOHIDFamily/AppleEmbeddedHIDKeys.h \
	IOHIDFamily/AppleEmbeddedHIDEventService.h \
	IOHIDFamily/AppleEmbeddedKeyboard.h \
	IOHIDFamily/AppleHIDUsageTables.h \
	IOHIDFamily/IOHIDPrivateKeys.h \
	IOHIDFamily/IOHIDEventTypes.h \
	IOHIDFamily/IOHIDEventData.h \
	IOHIDFamily/IOHIDEvent.h
HID_KERNPRIV_DST = ${KERNEL_PRIVHDRS}/IOKit/hid

HIDSYS = IOHIDSystem/IOKit/hidsystem/ev_private.h \
	IOHIDSystem/IOHIDPrivate.h \
        IOHIDFamily/IOHIDUsageTables.h \
	IOHIDFamily/AppleHIDUsageTables.h
HIDSYS_DST = ${KERNEL_PRIVHDRS}/IOKit/hidsystem

HID_KERN = IOHIDFamily/IOHIDEventServiceTypes.h \
	IOHIDFamily/IOHIDEventServiceKeys.h \
	IOHIDFamily/IOHIDDeviceKeys.h \
	IOHIDFamily/IOHIDInterface.h \
	IOHIDFamily/IOHIDProperties.h \
	IOHIDFamily/IOHIDDevice.h \
	IOHIDFamily/IOHIDKeys.h \
	IOHIDFamily/IOHIDUsageTables.h \
	IOHIDFamily/IOHIDElement.h \
	IOHIDFamily/IOHIDDeviceTypes.h 
HID_KERN_DST = ${KERNEL_HDRS}/IOKit/hid

HID_IOKIT = IOHIDFamily/IOHIDEventServiceTypes.h \
	IOHIDFamily/IOHIDEventServiceKeys.h \
	IOHIDFamily/IOHIDDeviceTypes.h \
	IOHIDFamily/IOHIDDeviceKeys.h \
	IOHIDFamily/IOHIDProperties.h \
	IOHIDLib/IOHIDLibObsolete.h \
	IOHIDFamily/IOHIDKeys.h \
	IOHIDFamily/IOHIDUsageTables.h
HID_IOKIT2 = IOHIDServicePrivate.h IOHIDEvent.h \
	IOHIDEventSystemClientPrivate.h IOHIDService.h IOHIDServiceClient.h \
	IOHIDServiceClientPrivate.h IOHIDServiceConnectionCache.h \
	IOHIDServiceFilter.h IOHIDServiceFilterPlugIn.h IOHIDServiceKeys.h \
	IOHIDServicePlugIn.h IOHIDEventSystemKeysPrivate.h

HID_IOKIT_DST = ${IOKIT_HDRS}/hid

HID_IOKITPRIV = IOHIDFamily/IOHIDEventServiceKeys_Private.h \
	IOHIDFamily/IOHIDEventFieldDefs.h \
	IOHIDFamily/IOHIDEventMacroDefs.h \
	IOHIDFamily/IOHIDEventStructDefs.h \
	IOHIDFamily/AppleEmbeddedHIDKeys.h \
	IOHIDFamily/IOHIDResourceUserClient.h \
	IOHIDFamily/IOHIDEventTypes.h \
	IOHIDFamily/IOHIDEventData.h \
	IOHIDFamily/IOHIDLibUserClient.h \
	IOHIDFamily/IOHIDPrivateKeys.h \
	IOHIDFamily/AppleHIDUsageTables.h
HID_IOKITPRIV_DST = ${IOKIT_PRIVHDRS}/hid

HID_IOKITSYS = IOHIDSystem/IOKit/hidsystem/IOHIDParameter.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDShared.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDTypes.h \
	IOHIDSystem/IOKit/hidsystem/IOLLEvent.h \
	IOHIDSystem/IOKit/hidsystem/ev_keymap.h
HID_IOKITSYS_DST = ${IOKIT_HDRS}/hidsystem

HID_IOKITPRIVSYS = IOHIDSystem/IOKit/hidsystem/ev_private.h \
	IOHIDSystem/IOHIDPrivate.h
HID_IOKITPRIVSYS_DST = ${IOKIT_PRIVHDRS}/hidsystem

HID_KERNPRIVSYS = IOHIDSystem/IOKit/hidsystem/ev_keymap.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDDescriptorParser.h \
	IOHIDFamily/IOHIDDevice.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDParameter.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDShared.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDSystem.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDTypes.h \
	IOHIDFamily/IOHIDUsageTables.h \
	IOHIDSystem/IOKit/hidsystem/IOHIKeyboard.h \
	IOHIDSystem/IOKit/hidsystem/IOHIPointing.h \
	IOHIDSystem/IOKit/hidsystem/IOHIKeyboardMapper.h \
	IOHIDSystem/IOKit/hidsystem/IOLLEvent.h
HID_KERNPRIVSYS_DST = ${KERNEL_PRIVHDRS}/IOKit/hidsystem

HIDEVENT_KERN = IOHIDFamily/IOHIDEventDriver.h \
	IOHIDFamily/IOHIDEventService.h \
	IOHIDFamily/IOHIDUtility.h
HIDEVENT_KERN_DST = ${KERNEL_HDRS}/IOKit/hidevent

HID_KERNSYS = IOHIDSystem/IOHIDWorkLoop.h \
	IOHIDSystem/IOKit/hidsystem/ev_keymap.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDDescriptorParser.h \
	IOHIDFamily/IOHIDDevice.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDParameter.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDShared.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDSystem.h \
	IOHIDSystem/IOKit/hidsystem/IOHIDTypes.h \
	IOHIDFamily/IOHIDUsageTables.h \
	IOHIDSystem/IOKit/hidsystem/IOHIKeyboard.h \
	IOHIDSystem/IOKit/hidsystem/IOHIPointing.h \
	IOHIDSystem/IOKit/hidsystem/IOHIKeyboardMapper.h \
	IOHIDSystem/IOKit/hidsystem/IOLLEvent.h
HID_KERNSYS_DST = ${KERNEL_HDRS}/IOKit/hidsystem

HID_OBJC = hidobjcbase.h HIDConnectionBase.h HIDConnectionIvar.h \
	HIDDeviceBase.h HIDDeviceIvar.h HIDElementBase.h HIDElementIvar.h \
	HIDEventBase.h HIDEventBasePrivate.h HIDEventIvar.h HIDServiceBase.h \
	HIDServiceClientBase.h HIDServiceClientIvar.h HIDServiceIvar.h \
	HIDSessionBase.h HIDSessionIvar.h
HID_OBJC_DST = ${IOKIT_HDRS}/hidobjc

${FRAMEWORK}: copy_headers

copy_headers: .PHONY
	mkdir -p ${RAVYN_SDKROOT}/usr/local/share/misc
	cp -f ${.CURDIR:H}/tools/hid.codes \
	  ${RAVYN_SDKROOT}/usr/local/share/misc/
.for dir in ${HID_OBJC_DST} ${HID_KERNSYS_DST} ${HIDEVENT_KERN_DST} \
  ${HID_KERNPRIVSYS_DST} ${HID_IOKITPRIVSYS_DST} ${HID_IOKITSYS_DST} \
  ${HID_IOKITPRIV_DST} ${HID_IOKIT_DST} ${HID_KERN_DST} ${HID_KERNPRIV_DST} \
  ${HID_SYS_DST} ${HID_DK_DST}
	mkdir -p ${dir}
.endfor
	touch ${HID_IOKIT_DST}/IOHIDEventSystemClient.h
.for file in ${HID_KERNSYS}
	cp -f ${.CURDIR:H}/${file} ${HID_KERNSYS_DST}
.endfor
.for file in ${HIDEVENT_KERN}
	cp -f ${.CURDIR:H}/${file} ${HIDEVENT_KERN_DST}
.endfor
.for file in ${HID_KERNPRIVSYS}
	cp -f ${.CURDIR:H}/${file} ${HID_KERNPRIVSYS_DST}
.endfor
.for file in ${HID_IOKITPRIVSYS}
	cp -f ${.CURDIR:H}/${file} ${HID_IOKITPRIVSYS_DST}
.endfor
.for file in ${HID_IOKITSYS}
	cp -f ${.CURDIR:H}/${file} ${HID_IOKITSYS_DST}
.endfor
.for file in ${HID_IOKITPRIV}
	cp -f ${.CURDIR:H}/${file} ${HID_IOKITPRIV_DST}
.endfor
.for file in ${HID_IOKIT}
	cp -f ${.CURDIR:H}/${file} ${HID_IOKIT_DST}
.endfor
.for file in ${HID_IOKIT2}
	cp -f ${ROOT_SOURCE_DIR}/Kernel/IOKitUser/hid.subproj/${file} \
	  ${HID_IOKIT_DST}
.endfor
.for file in ${HID_KERN}
	cp -f ${.CURDIR:H}/${file} ${HID_KERN_DST}
.endfor
.for file in ${HID_KERNPRIV}
	cp -f ${.CURDIR:H}/${file} ${HID_KERNPRIV_DST}
.endfor
.for file in ${HID_SYS}
	cp -f ${.CURDIR:H}/${file} ${HID_SYS_DST}
.endfor
.for file in ${HID_DK}
	cp -f ${.CURDIR:H}/${file} ${HID_DK_DST}
.endfor
.for file in ${HID_OBJC}
	cp -f ${ROOT_SOURCE_DIR}/Kernel/IOKitUser/hid.subproj/${file} \
	  ${HID_OBJC_DST}
.endfor

.include <rvn.framework.mk>

# D868E2CD1BD5D9C600CC07F9 /* IOHIDNXEventRouter.plugin in CopyFiles */,
# 848E575C0CC55C7800D5BE22 /* IOHIDSystem.kext in CopyFiles */,
# 848E575D0CC55C7800D5BE22 /* IOHIDUserClient.kext in CopyFiles */,
# 848E575A0CC55C7800D5BE22 /* IOHIDEventDriver.kext in CopyFiles */,
# 848E57590CC55C7800D5BE22 /* IOHIDEventDriverSafeBoot.kext in CopyFiles */,
