# CMake build for libtiff
# Run "cmake" to generate the build files for your platform
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Copyright © 2021-2022 Roger Leigh <rleigh@codelibre.net>
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

cmake_minimum_required(VERSION 3.2.0)

include(Sphinx)

if (BUILD_SPHINX AND SPHINX_BUILD)
  # Re-run CMake when the Sphinx configuration is updated
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/conf.py")

  message(STATUS "Checking manual page dependencies")

  # Create build directory and conf.py
  set(sphinx_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
  set(sphinx_builddir "${CMAKE_CURRENT_BINARY_DIR}")
  file(MAKE_DIRECTORY "${sphinx_builddir}")

  sphinx_manpages("${sphinx_srcdir}" "${sphinx_builddir}/man" MAN_PAGES)
  sphinx_manpage_dependencies("${sphinx_srcdir}" MAN_PAGE_DEPENDENCIES)
  sphinx_dependencies("${sphinx_srcdir}" SPHINX_DEPENDENCIES)

  # Generate and install man pages

  unset(man_target)
  if(MAN_PAGES)
    add_custom_command(OUTPUT ${MAN_PAGES}
                       COMMAND ${CMAKE_COMMAND} -E make_directory "${sphinx_builddir}/cache"
                       COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/man"
                       COMMAND ${SPHINX_BUILD}
                               -D "release=${LIBTIFF_VERSION_FULL}"
                               -D "version=${tiff_VERSION_MAJOR}.${tiff_VERSION_MINOR}"
                               -d "${sphinx_builddir}/cache"
                               -b man
                               "${sphinx_srcdir}" "${sphinx_builddir}/man"
                       WORKING_DIRECTORY "${sphinx_srcdir}"
                       DEPENDS ${MAN_PAGE_DEPENDENCIES})

    if(NOT TARGET doc-man)
      add_custom_target(doc-man ALL)
    endif()
    add_custom_target(${PROJECT_NAME}-doc-man DEPENDS ${MAN_PAGES})
    add_dependencies(doc-man ${PROJECT_NAME}-doc-man)
    set(man_target ${PROJECT_NAME}-doc-man)

    foreach (man ${MAN_PAGES})
      string(REGEX REPLACE ".*\\.([0-9])[a-z]*\$" "\\1" man_section "${man}")
      install(FILES "${man}"
              DESTINATION "${CMAKE_INSTALL_MANDIR}/man${man_section}"
              COMPONENT "runtime")
    endforeach()
    endif()

  # Generate and install HTML manual
  # Depends on man to allow sharing of cache with parallel build.

  add_custom_command(OUTPUT "${sphinx_builddir}/html/index.html"
                     COMMAND ${CMAKE_COMMAND} -E make_directory "${sphinx_builddir}/cache"
                     COMMAND ${CMAKE_COMMAND} -E make_directory "${sphinx_builddir}/html"
                     COMMAND ${SPHINX_BUILD}
                             -D "release=${tiff_VERSION}"
                             -D "version=${tiff_VERSION_MAJOR}.${tiff_VERSION_MINOR}"
                             -d "${sphinx_builddir}/cache"
                             -b html
                             "${sphinx_srcdir}" "${sphinx_builddir}/html"
                     WORKING_DIRECTORY "${sphinx_srcdir}"
                     DEPENDS ${man_target} ${SPHINX_DEPENDENCIES})

  if(NOT TARGET doc-html)
    add_custom_target(doc-html ALL)
  endif()
  add_custom_target(${PROJECT_NAME}-doc-html DEPENDS "${sphinx_builddir}/html/index.html")
  add_dependencies(doc-html ${PROJECT_NAME}-doc-html)

  install(DIRECTORY "${sphinx_builddir}/html"
          DESTINATION "${CMAKE_INSTALL_DOCDIR}/manual"
          COMPONENT "runtime")

  if(sphinx-linkcheck)
    set(sphinx_linkcheck_all ALL)
  endif()

  if(NOT TARGET doc-linkcheck)
    add_custom_target(doc-linkcheck)
  endif()
  add_custom_target(${PROJECT_NAME}-doc-linkcheck ${sphinx_linkcheck_all}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${sphinx_builddir}/cache"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${sphinx_builddir}/linkcheck"
                    COMMAND ${SPHINX_BUILD}
                            -D "release=${tiff_VERSION}"
                            -D "version=${tiff_VERSION_MAJOR}.${tiff_VERSION_MINOR}"
                            -c "${sphinx_builddir}"
                            -d "${sphinx_builddir}/cache"
                            -b linkcheck
                            "${sphinx_srcdir}" "${sphinx_builddir}/linkcheck"
                     COMMENT "Checking remote links in sphinx documentation"
                     WORKING_DIRECTORY "${sphinx_srcdir}"
                     DEPENDS ${man_target} ${SPHINX_DEPENDENCIES})
  add_dependencies(doc-linkcheck ${PROJECT_NAME}-doc-linkcheck)

  if(NOT TARGET doc-locallinkcheck)
    add_custom_target(doc-locallinkcheck)
  endif()
  add_custom_target(${PROJECT_NAME}-doc-locallinkcheck ${sphinx_linkcheck_all}
                    COMMAND ${CMAKE_COMMAND}
                            "-DOUTPUT=${sphinx_builddir}/linkcheck/output.txt"
                            "-DSPHINX_INSTALL_PATH=${PROJECT_NAME}/manual/html"
                            "-DINTERNAL_REFERENCE=${sphinx_builddir}/html"
                            "-DEXTERNAL_REFERENCE=${CMAKE_INSTALL_FULL_DATAROOTDIR}/doc"
                            "-DDOXYGEN_REFERENCE=${PROJECT_BINARY_DIR}/docs/doxygen/api"
                            "-DDOXYGEN_INSTALL_PATH=${PROJECT_NAME}/api"
                            -P "${PROJECT_SOURCE_DIR}/cmake/SphinxCheckInternalLinks.cmake"
                     COMMENT "Checking local links in sphinx documentation"
                     WORKING_DIRECTORY "${sphinx_srcdir}"
                     DEPENDS doc-linkcheck)
  add_dependencies(doc-locallinkcheck ${PROJECT_NAME}-doc-locallinkcheck)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/html-prebuilt/")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/html-prebuilt/"
          DESTINATION "${CMAKE_INSTALL_DOCDIR}/manual/html"
          COMPONENT "runtime")
else()
  message(WARNING "Manual pages and HTML manual will not be generated or installed")
endif()
