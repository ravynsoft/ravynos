include $(GNUSTEP_MAKEFILES)/common.make
include ../../config.make

VERSION = 0.1

# This is an app not a tool because, eventually, it will present the user
# interface for the GUI part of the speech engine.
APP_NAME = GSSpeechRecognitionServer

GSSpeechRecognitionServer_LANGUAGES = English

GSSpeechRecognitionServer_OBJC_FILES = \
	GSSpeechRecognitionEngine.m \
	GSSpeechRecognitionServer.m \
	main.m

# Add includes and link dirs.... 
GSSpeechRecognitionServer_OBJC_FILES += $(RECOGNIZER_ENGINE_CLASS)
GSSpeechRecognitionServer_INCLUDE_DIRS += $(RECOGNIZER_BASE_CFLAGS) \
	-I../../Headers \
	-I../../Headers/Additions
GSSpeechRecognitionServer_LIB_DIRS += -L../../Source/$(GNUSTEP_OBJ_DIR) \
	-L/usr/local/lib -lgnustep-gui \
	$(RECOGNIZER_BASE_LIBS)

include $(GNUSTEP_MAKEFILES)/application.make
