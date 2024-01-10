# CMake build for libtiff
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
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

# MSVC specific resource preparation
if (WIN32 AND MSVC)
  message(STATUS "Include resource version information for TOOLS")
  set(MSVC_RESSOURCE_FILE tif_tools_versioninfo.rc)
  source_group("Resource Files" FILES tif_tools_versioninfo.rc)
endif ()


add_executable(tiffcp ../placeholder.h)
target_sources(tiffcp PRIVATE tiffcp.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiffcp PRIVATE tiff tiff_port)

add_executable(tiffdump ../placeholder.h)
target_sources(tiffdump PRIVATE tiffdump.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiffdump PRIVATE tiff tiff_port)

add_executable(tiffinfo ../placeholder.h)
target_sources(tiffinfo PRIVATE tiffinfo.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiffinfo PRIVATE tiff tiff_port)

add_executable(tiffset ../placeholder.h)
target_sources(tiffset PRIVATE tiffset.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiffset PRIVATE tiff tiff_port)

add_executable(tiffsplit ../placeholder.h)
target_sources(tiffsplit PRIVATE tiffsplit.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiffsplit PRIVATE tiff tiff_port)

# rgb2ycbcr and thumbnail are intended to *NOT* be installed. They are for
# testing purposes only.
install(TARGETS tiffcp
                tiffdump
                tiffinfo
                tiffset
                tiffsplit
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(WEBP_SUPPORT AND EMSCRIPTEN)
  # Emscripten is pretty finnicky about linker flags.
  # It needs --shared-memory if and only if atomics or bulk-memory is used.
  foreach(target tiffcp
                 tiffdump
                 tiffinfo
                 tiffset
                 tiffsplit)
    target_link_options(${target} PUBLIC "-Wl,--shared-memory")
  endforeach()
endif()
