#
# Makefile for generating the SCSchemaDefinitions.[ch] files
#

all: SCSchemaDefinitions.h SCSchemaDefinitionsPrivate.h SCSchemaDefinitions.c

/tmp/genSCPreferences:	genSCPreferences.c Makefile
	xcrun --sdk macosx.internal cc -g -o /tmp/genSCPreferences genSCPreferences.c

SCSchemaDefinitions.h:	/tmp/genSCPreferences
	/tmp/genSCPreferences header	> SCSchemaDefinitions.h

SCSchemaDefinitionsPrivate.h:	/tmp/genSCPreferences
	/tmp/genSCPreferences private	> SCSchemaDefinitionsPrivate.h

SCSchemaDefinitions.c:	/tmp/genSCPreferences
	/tmp/genSCPreferences cfile	> SCSchemaDefinitions.c
