.include "../framework.common.mk"

FRAMEWORK=CoreGraphics
SRCS= \
	CGAffineTransform.m \
	CGBitmapContext.m \
	CGColor.m \
	CGColorSpace.m \
	CGContext.m \
	CGDataConsumer.m \
	CGDataProvider.m \
	CGDirectDisplay.m \
	CGEvent.m \
	CGFont.m \
	CGFunction.m \
	CGGeometry.m \
	CGGradient.m \
	CGImage.m \
	CGImageDestination.m \
	CGImageProperties.m \
	CGImageSource.m \
	CGLPixelSurface.m \
	CGLayer.m \
	CGPDFContext.m \
	CGPDFDocument.m \
	CGPDFPage.m \
	CGPath.m \
	CGPattern.m \
	CGShading.m \
	CGWindow.m 

INCS= \
	CGAffineTransform.h \
	CGBitmapContext.h \
	CGColor.h \
	CGColorSpace.h \
	CGContext.h \
	CGConversions.h \
	CGDataConsumer.h \
	CGDataProvider.h \
	CGDirectDisplay.h \
	CGError.h \
	CGEvent.h \
	CGFont.h \
	CGFunction.h \
	CGGeometry.h \
	CGGradient.h \
	CGImage.h \
	CGImageDestination.h \
	CGImageProperties.h \
	CGImageSource.h \
	CGLPixelSurface.h \
	CGLayer.h \
	CGPDFContext.h \
	CGPDFDocument.h \
	CGPDFPage.h \
	CGPath.h \
	CGPattern.h \
	CGShading.h \
	CGWindow.h \
	CGWindowLevel.h \
	CoreGraphics.h \
	CoreGraphicsExport.h \
	CoreText.h 

RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -F${.CURDIR}/../OpenGL -framework OpenGL -I.. -I../Foundation/Headers
LDFLAGS+= ${FMWK_LDFLAGS} -L${BUILDROOT}/System/Library/Frameworks/Onyx2D.framework \
	-L${BUILDROOT}/System/Library/Frameworks/OpenGL.framework -lOnyx2D -lOpenGL -lGL -lm \
	-Wl,-R/System/Library/Frameworks/Onyx2D.framework/Versions/A \
	-Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <rvn.framework.mk>

installresources: _install-hook
