unset(CMAKE_CXX_STANDARD)
unset(CMAKE_CXX_STANDARD_REQUIRED)
unset(CMAKE_CXX_EXTENSIONS)
set(CMAKE_OBJC_COMPILER ${CC})
set(CMAKE_OBJCXX_COMPILER ${CXX})

add_compile_definitions(
	APPLE PRIVATE _ANSI_C_SOURCE volatile=__volatile
	#HAVE_CFPLUGIN HAVE_CFURLACCESS
	NO_SECURITY_FRAMEWORK
	"AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER="
	"AVAILABLE_MAC_OS_X_VERSION_10_15_AND_LATER="
	IOKIT DISPATCH_MACH_SPI
	OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE
	OS_OBJECT_USE_OBJC
	"CF_ASSUME_NONNULL_BEGIN="
	"CF_ASSUME_NONNULL_END=")

add_compile_options(
	-fblocks --sysroot=${RAVYN_SDKROOT}
	-mmacos-version-min=${MACOS_VERSION_MIN}
	-Wno-nullability-completeness
	-Wno-implicit-function-declaration
	-Wno-elaborated-enum-base # CF_ENUM, rdar://121475724
)

set(MIG_FLAGS -DIOKIT -DPRIVATE -DNO_SECURITY_FRAMEWORK)

add_framework_bundle(IOKit
	VERSION A
	INFO_PLIST Info.plist
	INSTALL_NAME_DIR /System/Library/Frameworks/IOKit.framework/Versions/A)

set(iokitPrivateHdrs ${RAVYN_SDKROOT}/System/Library/Frameworks/IOKit.framework/Versions/A/PrivateHeaders)
set(iokitHdrs ${RAVYN_SDKROOT}/System/Library/Frameworks/IOKit.framework/Versions/A/Headers)

file(COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/bootfiles.h
		DESTINATION ${RAVYN_SDKROOT}/usr/local/include/)
file(COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDDeviceBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDDeviceIvar.h 
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDServiceClientIvar.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDServiceClientBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDElementBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDElementIvar.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDConnectionBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDConnectionIvar.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDSessionIvar.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDSessionBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDEventBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDServiceBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/HIDServiceIvar.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/hidobjcbase.h
		DESTINATION ${iokitPrivateHdrs}/hidobjc/)
file(COPY
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/OSKext.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/OSKextPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/misc_util.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/fat_util.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/macho_util.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/KextManager.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/KextManagerPriv.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/kextmanager_types.h
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/kextmanager_mig.defs
		DESTINATION ${RAVYN_SDKROOT}/usr/local/include/IOKit/kext/)
file(COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/audio.subproj/IOAudioLib.h 
		DESTINATION ${iokitHdrs}/audio/)
file(COPY
		${CMAKE_CURRENT_SOURCE_DIR}/hidsystem.subproj/IOHIDUserDevice.h
		${CMAKE_CURRENT_SOURCE_DIR}/hidsystem.subproj/IOHIDServiceClient.h
		${CMAKE_CURRENT_SOURCE_DIR}/hidsystem.subproj/IOHIDEventSystemClient.h
		${CMAKE_CURRENT_SOURCE_DIR}/hidsystem.subproj/IOHIDLib.h
		${CMAKE_CURRENT_SOURCE_DIR}/hidsystem.subproj/event_status_driver.h
		DESTINATION ${iokitHdrs}/hidsystem/)
file(COPY
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDAnalytics.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDConnectionFilterPlugin.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServiceConnectionCache.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemClientPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemConnection.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemKeysPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServiceClient.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServiceKeys.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServicePlugIn.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventQueue.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystem.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemClient.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDEventSystemKeys.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDNotification.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServicePrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDService.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDTypes.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDServiceFilterPlugIn.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDSessionFilterPlugIn.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDUserDevice.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDBase.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDDevice.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDDevicePlugIn.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDElement.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDManager.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDQueue.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDTransaction.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDValue.h
		${CMAKE_CURRENT_SOURCE_DIR}/hid.subproj/IOHIDLibPrivate.h
		DESTINATION ${iokitHdrs}/hid/)
file(COPY
		${CMAKE_CURRENT_SOURCE_DIR}/network.subproj/IOUserEthernetController.h
		${CMAKE_CURRENT_SOURCE_DIR}/network.subproj/IONetworkLib.h
		DESTINATION ${iokitHdrs}/network/)
file(COPY 
		${CMAKE_CURRENT_SOURCE_DIR}/ps.subproj/IOPowerSourcesPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/ps.subproj/IOPSKeysPrivate.h
		DESTINATION ${iokitHdrs}/ps/)
file(COPY
		${CMAKE_CURRENT_SOURCE_DIR}/pwr_mgt.subproj/IOPMUPSPrivate.h
		${CMAKE_CURRENT_SOURCE_DIR}/pwr_mgt.subproj/powermanagement_mig.h
		${CMAKE_CURRENT_SOURCE_DIR}/pwr_mgt.subproj/powermanagement.defs
		DESTINATION ${iokitHdrs}/pwr_mgt/)

set(MIGFLAGS 
	-cc ${CC} -novouchers -server /dev/null 
	-DIOKIT -I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk)

add_custom_command(OUTPUT kextmanager_mig.h
	COMMAND ${MIG} ${MIGFLAGS} -header kextmanager_mig.h 
		${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/kextmanager_mig.defs
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kext.subproj/kextmanager_mig.defs)

add_custom_command(OUTPUT powermanagement.h
	COMMAND ${MIG} ${MIGFLAGS} -header powermanagement.h
		${CMAKE_CURRENT_SOURCE_DIR}/pwr_mgt.subproj/powermanagement.defs
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pwr_mgt.subproj/powermanagement.defs)

add_custom_command(OUTPUT iokitmig_c.h iokitmig64.c
	COMMAND ${MIG} ${MIGFLAGS} -header iokitmig_c.h -user iokitmig64.c
		${CMAKE_CURRENT_SOURCE_DIR}/iokitmig.defs
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/iokitmig.defs)

target_sources(IOKit PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/iokitmig64.c
	./hid.subproj/IOHIDEventObjc.m
	./hid.subproj/IOHIDServiceObjc.m
	./hid.subproj/IOHIDSessionObjc.m
	./hid.subproj/IOHIDConnectionObjc.m
	./hid.subproj/IOHIDElementObjc.m
	./hid.subproj/IOHIDServiceClientObjc.m
	./hid.subproj/IOHIDDeviceObjc.m
	./hid.subproj/HIDEventBase.m
	./hid.subproj/HIDServiceBase.m
	./hid.subproj/HIDSessionBase.m
	./hid.subproj/HIDConnectionBase.m
	./hid.subproj/HIDElementBase.m
	./hid.subproj/HIDServiceClientBase.m
	./hid.subproj/HIDDeviceBase.m
	./kext.subproj/cross_link.c
	./IOCFSerialize.c
	./IOCFUnserialize.tab.c
	./IOKitLib.c
	./IOCFURLAccess.c
	./hid.subproj/IOHIDAnalytics.c
	./IODataQueueClient.c
	./IOCFPlugIn.c
	./IOTrap.s
	./pwr_mgt.subproj/IOPMLib.c
	./pwr_mgt.subproj/IOPMEnergyPrefs.c
	./pwr_mgt.subproj/IOPMAutoWake.c
	./pwr_mgt.subproj/IOPMRepeatingPower.c
	./pwr_mgt.subproj/IOPMUPSPrefs.c
	./usb.subproj/IOUSBLib.c
	./network.subproj/IONetworkLib.c
	./graphics.subproj/IOAccelSurfaceControl.c
	./graphics.subproj/IOGraphicsLib.c
	./graphics.subproj/IODisplayLib.c
	./hidsystem.subproj/IOEventStatusAPI.c
	./kext.subproj/AdjustForNewSegmentLocation.cpp
	./hidsystem.subproj/IOHIDLib.c
	./kext.subproj/KextManager.c
	./ps.subproj/IOPowerSources.c
	./hid.subproj/IOHIDServiceConnectionCache.c
	./ps.subproj/IOPowerSourcesPrivate.c
	./pwr_mgt.subproj/IOPMLibPrivate.c
	./IOSystemConfiguration.c
	./hid.subproj/IOHIDValue.c
	./hid.subproj/IOHIDSessionFilter.c
	./hid.subproj/IOHIDElement.c
	./kext.subproj/fat_util.c
	./kext.subproj/macho_util.c
	./hid.subproj/IOHIDDevice.c
	./hid.subproj/IOHIDServiceKeys.c
	./hid.subproj/IOHIDLibPrivate.c
	./hid.subproj/IOHIDManager.c
	./hid.subproj/IOHIDQueue.c
	./hid.subproj/IOHIDTransaction.c
	./hid.subproj/IOHIDEvent.c
	./hid.subproj/IOHIDEventSystem.c
	./hid.subproj/IOHIDConnectionFilter.c
	./hid.subproj/IOHIDEventSystemClient.c
	./hid.subproj/IOHIDNotification.c
	./hid.subproj/IOHIDService.c
	./hid.subproj/IOHIDSession.c
	./hid.subproj/IOHIDEventQueue.c
	./pwr_mgt.subproj/IOPMPowerNotifications.c
	./kext.subproj/OSKextVersion.c
	./kext.subproj/misc_util.c
	./kext.subproj/printPList_new.c
	./IOMIGMachPort.c
	./kext.subproj/OSKext.c
	./hid.subproj/IOHIDEventSystemPrivate.c
	./kext.subproj/kextsplit.h
	./hid.subproj/IOHIDServiceFilter.c
	./hid.subproj/IOHIDServiceClient.c
	./hid.subproj/IOHIDEventSystemConnection.c
	./hid.subproj/IOHIDEventServer.c
	./pwr_mgt.subproj/IOPMAssertions.c
	./network.subproj/IOUserEthernetController.c
	./hid.subproj/IOHIDUserDevice.c
	./hid.subproj/IOHIDPlugins.c
	./platform.subproj/IOPlatformSupport.c)

target_sources(IOKit INTERFACE 
	${CMAKE_CURRENT_BINARY_DIR}/powermanagement.h
	${CMAKE_CURRENT_BINARY_DIR}/kextmanager_mig.h
	./kext.subproj/FileAbstraction.hpp
	./kext.subproj/Architectures.hpp
	./kext.subproj/MachOFileAbstraction.hpp
	./kext.subproj/Trie.hpp)

target_compile_options(IOKit PRIVATE
	-nostdinc --target=${CpuArch}-apple-darwin
	-isystem${RAVYN_SDKROOT}/usr/include/c++/v1
	-isystem${TOOLS}/../lib/clang-17)

target_include_directories(IOKit PRIVATE
	../xnu/bsd 
	../xnu/osfmk 
	../xnu/libkern
	../xnu/iokit
	${CMAKE_CURRENT_BINARY_DIR}
	${RAVYN_SDKROOT}/usr/local/include/kernel
	${RAVYN_SDKROOT}/usr/local/include
	${ROOT_SOURCE_DIR}/Frameworks
	${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers
	${RAVYN_SDKROOT}/usr/include
	${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders
	../../Libraries/Libsystem/private)

set_source_files_properties(
	hid.subproj/IOHIDEventBase.m
	hid.subproj/IOHIDEventObjc.m
	hid.subproj/IOHIDConnectionObjc.m
	hid.subproj/IOHIDDeviceObjc.m
	hid.subproj/IOHIDElementObjc.m
	hid.subproj/IOHIDServiceObjc.m
	hid.subproj/IOHIDServiceClientObjc.m
	hid.subproj/IOHIDSessionObjc.m
	hid.subproj/HIDEventBase.m
	hid.subproj/HIDConnectionBase.m
	hid.subproj/HIDDeviceBase.m
	hid.subproj/HIDElementBase.m
	hid.subproj/HIDServiceBase.m
	hid.subproj/HIDServiceClientBase.m
	hid.subproj/HIDSessionBase.m
	${CMAKE_CURRENT_BINARY_DIR}/iokitmig64.c
	PROPERTIES LANGUAGE OBJCXX)

target_compile_options(IOKit PRIVATE
	-include${RAVYN_SDKROOT}/usr/local/include/kernel/os/object.h
	-include${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders/libkern/c++/OSObject.h
)


target_link_options(IOKit PRIVATE
	-Wl,-compatibility_version,1.0.0
	-Wl,-current_version,275.0.0)

add_subdirectory(kext.subproj)
