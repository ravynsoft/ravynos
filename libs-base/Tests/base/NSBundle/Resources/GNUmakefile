
include $(GNUSTEP_MAKEFILES)/common.make

NEEDS_GUI=no
BUNDLE_NAME = TestBundle
FRAMEWORK_NAME = TestFramework

TestBundle_OBJC_FILES = TestBundle.m
TestBundle_RESOURCE_FILES = NonLocalRes.txt
TestBundle_LANGUAGES = English French de
TestBundle_LOCALIZED_RESOURCE_FILES = TextRes.txt
TestBundle_NEEDS_GUI = NO


TestFramework_OBJC_FILES = TestFramework.m
TestFramework_RESOURCE_FILES = NonLocalRes.txt
TestFramework_LANGUAGES = English French de
TestFramework_LOCALIZED_RESOURCE_FILES = TextRes.txt
TestFramework_CURRENT_VERSION_NAME = 2

include $(GNUSTEP_MAKEFILES)/bundle.make
include $(GNUSTEP_MAKEFILES)/framework.make
check::	all
