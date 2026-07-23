SUBDIR = AppleAPIC IOACPIFamily IOPCIFamily IOStorageFamily IOATAFamily \
	 AppleIntelPIIXATA msdosfs hfs AppleFileSystemDriver RavynAHCIPort \
	 IOGraphics corecrypto pthread IOHIDFamily ApplePS2Controller \
         #RavynXHCIPort IOUSBFamily

all: kernelcache
.for target_dir in ${RAVYN_SDKROOT}/System/Library/Extensions \
  ${SYSROOT_DIR}/System/Library/Extensions
	mkdir -p ${target_dir}
.for kext in ${SUBDIR}
	cp -fR ${.OBJDIR}/${kext}/${kext:S/IOGraphics/IOGOPFramebuffer/}.kext ${target_dir}/
.endfor
.endfor

EXTENSIONS = ${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/Extensions
XNU_OBJDIR = ${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu

kernelcache: ${SUBDIR}
	@echo Generating prelinked kernel cache
	${TOOLS}/plktool ${.OBJDIR}/${.TARGET}.tmp \
	${XNU_OBJDIR}/sym/kernel \
	${XNU_OBJDIR}/sym/System.kext \
	${XNU_OBJDIR}/sym/System.kext/PlugIns/* \
	${EXTENSIONS}/IOACPIFamily/IOACPIFamily.kext \
	${EXTENSIONS}/AppleAPIC/AppleAPIC.kext \
	${EXTENSIONS}/ApplePS2Controller/ApplePS2Controller.kext \
	${EXTENSIONS}/IOPCIFamily/IOPCIFamily.kext \
	${EXTENSIONS}/AppleI386PCI/AppleI386PCI.kext \
	${EXTENSIONS}/corecrypto/corecrypto.kext \
	${EXTENSIONS}/pthread/pthread.kext \
	${EXTENSIONS}/RavynAHCIPort/RavynAHCIPort.kext \
	${EXTENSIONS}/IOATAFamily/IOATAFamily.kext \
	${EXTENSIONS}/IOStorageFamily/IOStorageFamily.kext \
	${EXTENSIONS}/AppleFileSystemDriver/AppleFileSystemDriver.kext \
	${EXTENSIONS}/hfs/hfs_encodings/hfs_encodings.kext \
	${EXTENSIONS}/hfs/hfs.kext \
	${EXTENSIONS}/IOGraphics/IOGOPFramebuffer.kext \
	${EXTENSIONS}/IOHIDFamily/IOHIDFamily.kext
	chmod 0644 ${.TARGET}.tmp
	${LIPO} -create -output ${.OBJDIR}/${.TARGET} \
	  -arch x86_64 ${.OBJDIR}/${.TARGET}.tmp
	cp -f ${.OBJDIR}/${.TARGET} ${ROOT_BINARY_DIR}

#	${EXTENSIONS}/IOUSBFamily/IOUSBFamily.kext \
	${EXTENSIONS}/RavynXHCIPort/RavynXHCIPort.kext \

.include <bsd.subdir.mk>
