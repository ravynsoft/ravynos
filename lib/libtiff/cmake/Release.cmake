# Release support
#
# Copyright © 2021 Roger Leigh <rleigh@codelibre.net>
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

# Add target tiff_release to update the version information in the root files
# VERSION and RELEASE-DATE.
add_custom_target(tiff_release
    COMMAND ${CMAKE_COMMAND}
    "-DSOURCE_DIR:PATH=${PROJECT_SOURCE_DIR}"
    "-DLIBTIFF_VERSION=${PROJECT_VERSION}"
    "-DLIBTIFF_BASIC_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    "-DLIBTIFF_BASIC_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/ReleaseScript.cmake"
    COMMENT "Releasing ${PROJECT_NAME} ${PROJECT_VERSION} ...")

# Version information is taken from configure.ac
# Note: Single command "cmake --build . --target tiff_release"
#       does not work correctly, because version information is taken from CMakeCache.txt,
#       which is not updated by cmake --build.
#       Therefore, force CMake re-configure if configure.ac has changed.
#       By the way, release-date and VERSION will not change but only when --targt tiff_release is called.
set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/configure.ac
)

