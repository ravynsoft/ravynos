#!/bin/bash -e
# install the pthread lldbmacros into the module

mkdir -p $DWARF_DSYM_FOLDER_PATH/$DWARF_DSYM_FILE_NAME/Contents/Resources/Python || true
rsync -aq $SRCROOT/lldbmacros/* $DWARF_DSYM_FOLDER_PATH/$DWARF_DSYM_FILE_NAME/Contents/Resources/Python

for variant in $BUILD_VARIANTS; do
	case $variant in
	normal)
		SUFFIX=""
		;;
	*)
		SUFFIX="_$variant"
		;;
	esac

	ln -sf init.py $DWARF_DSYM_FOLDER_PATH/$DWARF_DSYM_FILE_NAME/Contents/Resources/Python/$EXECUTABLE_NAME$SUFFIX.py
done

if test "$PLATFORM_NAME" != macosx; then
    mkdir -p $DSTROOT/AppleInternal/KextObjects/Python/$MODULE_NAME || true
    rsync -aq $SRCROOT/lldbmacros/* $DSTROOT/AppleInternal/KextObjects/Python/$MODULE_NAME
    ln -sf init.py $DSTROOT/AppleInternal/KextObjects/Python/$MODULE_NAME/$EXECUTABLE_NAME.py
fi
