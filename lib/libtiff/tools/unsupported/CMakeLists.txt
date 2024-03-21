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

message(STATUS "====  Tools - Unsupported =========")

# MSVC specific resource preparation
if (WIN32 AND MSVC)
  message(STATUS "Include resource version information for TOOLS-UNSUPPORTED")
  set(MSVC_RESSOURCE_FILE tif_tools-unsupported_versioninfo.rc)
  source_group("Resource Files" FILES tif_tools-unsupported_versioninfo.rc)
endif ()


add_executable(tiff2pdf ../../placeholder.h)
target_sources(tiff2pdf PRIVATE tiff2pdf.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiff2pdf PRIVATE tiff tiff_port)

add_executable(tiff2ps ../../placeholder.h)
target_sources(tiff2ps PRIVATE tiff2ps.c ${MSVC_RESSOURCE_FILE})
target_link_libraries(tiff2ps PRIVATE tiff tiff_port CMath::CMath)

# rgb2ycbcr and thumbnail are intended to *NOT* be installed. They are for
# testing purposes only.
install(TARGETS tiff2pdf
                tiff2ps
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/unsupported")

if(WEBP_SUPPORT AND EMSCRIPTEN)
  # Emscripten is pretty finnicky about linker flags.
  # It needs --shared-memory if and only if atomics or bulk-memory is used.
  foreach(target tiff2pdf
                 tiff2ps)
    target_link_options(${target} PUBLIC "-Wl,--shared-memory")
  endforeach()
endif()
