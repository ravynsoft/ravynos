PACKAGE_NAME = gnustep-gui
include $(GNUSTEP_MAKEFILES)/common.make

BUNDLE_NAME = Sndfile AudioOutput
BUNDLE_EXTENSION = .nssound

# These are here in case GNUstep is not installed,
# otherwise compilation will fail.
Sndfile_INCLUDE_DIRS += -I../../Headers \
        -I../../Headers/Additions

Sndfile_LIB_DIRS += \
	-L../../Source/$(GNUSTEP_OBJ_DIR)

AudioOutput_INCLUDE_DIRS += -I../../Headers \
        -I../../Headers/Additions

AudioOutput_LIB_DIRS += \
	-L../../Source/$(GNUSTEP_OBJ_DIR)

# Build the bundles.
Sndfile_OBJC_FILES = SndfileSource.m
AudioOutput_OBJC_FILES = AudioOutputSink.m

Sndfile_PRINCIPAL_CLASS = SndfileSource
AudioOutput_PRINCIPAL_CLASS = AudioOutputSink

Sndfile_BUNDLE_LIBS = -lsndfile
AudioOutput_BUNDLE_LIBS = -lao

include $(GNUSTEP_MAKEFILES)/bundle.make
