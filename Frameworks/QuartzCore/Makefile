.include "../framework.common.mk"

FRAMEWORK=QuartzCore
SRCS= \
	CAAnimation.m \
	CAAnimationGroup.m \
	CABase.m \
	CABasicAnimation.m \
	CALayer.m \
	CALayerContext.m \
	CAMediaTimingFunction.m \
	CAPropertyAnimation.m \
	CARenderer.m \
	CATransaction.m \
	CATransactionGroup.m \
	CATransform3D.m \
	CATransition.m \
	CAValueFunction.m \
	CAWindowOpenGLContext.m \
	CIAffineTransform.m \
	CIBloom.m \
	CIBoxBlur.m \
	CIColor.m \
	CIContext.m \
	CIFilter.m \
	CIImage.m \
	CILinearGradient.m \
	CIVector.m

INCS= \
	CAAction.h \
	CAAnimation.h \
	CAAnimationGroup.h \
	CABase.h \
	CABasicAnimation.h \
	CALayer.h \
	CALayerContext.h \
	CAMediaTiming.h \
	CAMediaTimingFunction.h \
	CAPropertyAnimation.h \
	CIBloom.h \
	CARenderer.h \
	CATransaction.h \
	CATransactionGroup.h \
	CATransform3D.h \
	CATransition.h \
	CAValueFunction.h \
	CAWindowOpenGLContext.h \
	CIAffineTransform.h \
	CIBoxBlur.h \
	CIColor.h \
	CIContext.h \
	CIFilter.h \
	CIImage.h \
	CILinearGradient.h \
	CIVector.h \
	CoreAnimation.h \
	CoreImage.h \
	CoreVideo.h \
	QuartzCore.h 

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I${TOPDIR}/Frameworks -I${.CURDIR}/../OpenGL/include -I${.CURDIR}/../Foundation/Headers -I${.CURDIR}/../AppKit/Headers
LDFLAGS+= ${FMWK_LDFLAGS} -L${BUILDROOT}/System/Library/Frameworks/OpenGL.framework \
	-lOpenGL -lGL -Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

installresources: _install-hook
