include $(GNUSTEP_MAKEFILES)/common.make

BUNDLE_NAME = TestConnection

NEEDS_GUI=NO

TestConnection_OBJC_FILES = TestCase.m SimpleWebServer.m TestWebServer.m NSURLConnectionTest.m RequestHandler.m
#TestConnection_OBJC_LIBS += -lWebServer -lPerformance
TestConnection_RESOURCE_FILES += testKey.pem testCert.pem
TestConnection_PRINCIPAL_CLASS = NSURLConnectionTest

TOOL_NAME = testTestWebServer
testTestWebServer_OBJC_FILES += testTestWebServer.m

-include GNUmakefile.preamble
include $(GNUSTEP_MAKEFILES)/bundle.make
include $(GNUSTEP_MAKEFILES)/tool.make
-include GNUmakefile.postamble

