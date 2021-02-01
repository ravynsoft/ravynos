#!/bin/sh
# Create a Framework directory structure with the given name

NAME="$1"
if [ "$NAME" = "" ]; then
    NAME=Framework
fi
NAME="${NAME}.framework"

mkdir -p "${NAME}/Versions/A/Headers" \
    "${NAME}/Versions/A/Modules" \
    "${NAME}/Versions/A/Resources"
(cd "${NAME}"; \
    ln -sf Versions/A/Headers Headers; \
    ln -sf Versions/A/Modules Modules; \
    ln -sf Versions/A/Resources Resources)
(cd "${NAME}/Versions"; ln -sf A Current)
touch "${NAME}/Versions/A/Resources/Info.plist"
