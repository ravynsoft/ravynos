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

# Images for tiff2bw (8-bit RGB only)
set(RGBIMAGES
    images/palette-1c-8b.tiff
    images/rgb-3c-8b.tiff
    images/quad-lzw-compat.tiff
)

# This list should contain all of the TIFF files in the 'images'
# subdirectory which are intended to be used as input images for
# tests.  All of these files should use the extension ".tiff".
set(TIFFIMAGES
    images/custom_dir_EXIF_GPS.tiff
    images/logluv-3c-16b.tiff
    images/minisblack-1c-16b.tiff
    images/minisblack-1c-8b.tiff
    images/minisblack-2c-8b-alpha.tiff
    images/miniswhite-1c-1b.tiff
    images/palette-1c-1b.tiff
    images/palette-1c-4b.tiff
    images/palette-1c-8b.tiff
    images/rgb-3c-16b.tiff
    images/rgb-3c-8b.tiff
    images/quad-lzw-compat.tiff
    images/lzw-single-strip.tiff
    images/testfax4.tiff
    images/testfax3_bug_513.tiff
)
# TIFF images using old-jpeg.  Unused unless JPEG support is enabled.
set(TIFFIMAGES_OJPEG)

if(JPEG_SUPPORT)
    list(APPEND TIFFIMAGES images/quad-tile.jpg.tiff)
endif()

if (OJPEG_SUPPORT)
    list(APPEND TIFFIMAGES_OJPEG
        images/ojpeg_zackthecat_subsamp22_single_strip.tiff
        images/ojpeg_chewey_subsamp21_multi_strip.tiff
        images/ojpeg_single_strip_no_rowsperstrip.tiff
    )
endif()

set(BMPIMAGES
    images/palette-1c-8b.bmp
    images/rgb-3c-8b.bmp)

set(GIFIMAGES
    images/palette-1c-8b.gif)

set(PNMIMAGES
    images/minisblack-1c-8b.pgm
    images/miniswhite-1c-1b.pbm
    images/rgb-3c-16b.ppm
    images/rgb-3c-8b.ppm)

# All uncompressed image files
set(UNCOMPRESSEDIMAGES
    images/minisblack-1c-16b.tiff
    images/minisblack-1c-8b.tiff
    images/miniswhite-1c-1b.tiff
    images/palette-1c-1b.tiff
    images/palette-1c-4b.tiff
    images/palette-1c-8b.tiff
    images/rgb-3c-8b.tiff)

# This list should include all of the files in the 'images'
# subdirectory which are intended to be distributed.  This may include
# files which are not currently used by the tests.
set(IMAGES_EXTRA_DIST
    images/README.txt
    images/miniswhite-1c-1b.g3
    ${BMPIMAGES}
    ${GIFIMAGES}
    ${PNMIMAGES}
    ${TIFFIMAGES})

set(noinst_HEADERS tifftest.h)

set(simple_tests)

# Test programs
add_executable(test_signed_tags ../placeholder.h)
target_sources(test_signed_tags PRIVATE test_signed_tags.c)
target_link_libraries(test_signed_tags PRIVATE tiff tiff_port)
list(APPEND simple_tests test_signed_tags)

add_executable(ascii_tag ../placeholder.h)
target_sources(ascii_tag PRIVATE ascii_tag.c)
target_link_libraries(ascii_tag PRIVATE tiff tiff_port)
list(APPEND simple_tests ascii_tag)

add_executable(long_tag ../placeholder.h)
target_sources(long_tag PRIVATE long_tag.c check_tag.c)
target_link_libraries(long_tag PRIVATE tiff tiff_port)
list(APPEND simple_tests long_tag)

add_executable(short_tag ../placeholder.h)
target_sources(short_tag PRIVATE short_tag.c check_tag.c)
target_link_libraries(short_tag PRIVATE tiff tiff_port)
list(APPEND simple_tests short_tag)

add_executable(strip_rw ../placeholder.h)
target_sources(strip_rw PRIVATE strip_rw.c strip.c test_arrays.c test_arrays.h)
target_link_libraries(strip_rw PRIVATE tiff tiff_port)
list(APPEND simple_tests strip_rw)

add_executable(rewrite ../placeholder.h)
target_sources(rewrite PRIVATE rewrite_tag.c)
target_link_libraries(rewrite PRIVATE tiff tiff_port)
list(APPEND simple_tests rewrite)

if(JPEG_SUPPORT)
  add_executable(raw_decode ../placeholder.h)
  target_sources(raw_decode PRIVATE raw_decode.c)
  target_link_libraries(raw_decode PRIVATE tiff tiff_port JPEG::JPEG)
  list(APPEND simple_tests raw_decode)
endif()

add_executable(custom_dir ../placeholder.h)
target_sources(custom_dir PRIVATE custom_dir.c)
target_link_libraries(custom_dir PRIVATE tiff tiff_port)
list(APPEND simple_tests custom_dir)

# Uses private functions from static library
if(NOT BUILD_SHARED_LIBS)
  add_executable(rational_precision2double ../placeholder.h)
  target_sources(rational_precision2double PRIVATE rational_precision2double.c)
  target_link_libraries(rational_precision2double PRIVATE tiff tiff_port)
  list(APPEND simple_tests rational_precision2double)
  
  add_executable(test_write_read_tags ../placeholder.h)
  target_sources(test_write_read_tags PRIVATE test_write_read_tags.c)
  target_link_libraries(test_write_read_tags PRIVATE tiff)
  list(APPEND simple_tests test_write_read_tags)
  
  add_executable(test_transferfunction_write_read ../placeholder.h)
  target_sources(test_transferfunction_write_read PRIVATE test_transferfunction_write_read.c)
  target_link_libraries(test_transferfunction_write_read PRIVATE tiff)
  list(APPEND simple_tests test_transferfunction_write_read)
endif()

add_executable(custom_dir_EXIF_231 ../placeholder.h)
target_sources(custom_dir_EXIF_231 PRIVATE custom_dir_EXIF_231.c)
target_link_libraries(custom_dir_EXIF_231 PRIVATE tiff tiff_port)
list(APPEND simple_tests custom_dir_EXIF_231)

add_executable(defer_strile_loading ../placeholder.h)
target_sources(defer_strile_loading PRIVATE defer_strile_loading.c)
target_link_libraries(defer_strile_loading PRIVATE tiff tiff_port)
list(APPEND simple_tests defer_strile_loading)

add_executable(defer_strile_writing ../placeholder.h)
target_sources(defer_strile_writing PRIVATE defer_strile_writing.c)
target_link_libraries(defer_strile_writing PRIVATE tiff tiff_port)
list(APPEND simple_tests defer_strile_writing)

add_executable(test_directory ../placeholder.h)
target_sources(test_directory PRIVATE test_directory.c)
target_link_libraries(test_directory PRIVATE tiff tiff_port)
list(APPEND simple_tests test_directory)

add_executable(testtypes ../placeholder.h)
target_sources(testtypes PRIVATE testtypes.c)
target_link_libraries(testtypes PRIVATE tiff tiff_port)
list(APPEND simple_tests testtypes)

add_executable(test_open_options ../placeholder.h)
target_sources(test_open_options PRIVATE test_open_options.c)
target_link_libraries(test_open_options PRIVATE tiff tiff_port)
list(APPEND simple_tests test_open_options)

add_executable(test_append_to_strip ../placeholder.h)
target_sources(test_append_to_strip PRIVATE test_append_to_strip.c)
target_link_libraries(test_append_to_strip PRIVATE tiff tiff_port)
list(APPEND simple_tests test_append_to_strip)

add_executable(test_ifd_loop_detection ../placeholder.h)
target_sources(test_ifd_loop_detection PRIVATE test_ifd_loop_detection.c)
target_link_libraries(test_ifd_loop_detection PRIVATE tiff tiff_port)
target_compile_definitions(test_ifd_loop_detection PRIVATE SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
list(APPEND simple_tests test_ifd_loop_detection)

if(WEBP_SUPPORT AND EMSCRIPTEN)
  # Emscripten is pretty finnicky about linker flags.
  # It needs --shared-memory if and only if atomics or bulk-memory is used.
  foreach(target
          ascii_tag
          long_tag
          short_tag
          strip_rw
          rewrite
          custom_dir
          defer_strile_loading
          defer_strile_writing
          )
    target_link_options(${target} PUBLIC "-Wl,--shared-memory")
  endforeach()
  if(JPEG_SUPPORT)
    target_link_options(raw_decode PUBLIC "-Wl,--shared-memory")
  endif()
endif()

set(TEST_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/output")
file(MAKE_DIRECTORY "${TEST_OUTPUT}")

set(tiff_test_extra_args "-DLIBTIFF=$<TARGET_FILE:tiff>")

if(tiff-tools)
  list(APPEND tiff_test_extra_args "-DTIFFCP=$<TARGET_FILE:tiffcp>")
  list(APPEND tiff_test_extra_args "-DTIFFINFO=$<TARGET_FILE:tiffinfo>")
  list(APPEND tiff_test_extra_args "-DTIFFSPLIT=$<TARGET_FILE:tiffsplit>")
endif()

if(WIN32)
  list(APPEND tiff_test_extra_args "-DWIN32=${WIN32}")
endif()
if(CYGWIN)
  list(APPEND tiff_test_extra_args "-DCYGWIN=${CYGWIN}")
endif()

function(tiff_test_convert name command1 command2 command3 infile outfile validate)
  add_test(NAME "${name}"
           COMMAND "${CMAKE_COMMAND}"
           "-DCONVERT_COMMAND1=${command1}"
           "-DCONVERT_COMMAND2=${command2}"
           "-DCONVERT_COMMAND3=${command3}"
           "-DINFILE=${infile}"
           "-DOUTFILE=${outfile}"
           "-DVALIDATE=${validate}"
           ${tiff_test_extra_args}
           -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffTest.cmake")
endfunction()

function(tiff_test_stdout name suffix command infile outfile)
  if (suffix)
     set(name "${name}-${suffix}")
  endif()
  add_test(NAME "${name}"
           COMMAND "${CMAKE_COMMAND}"
           "-DSTDOUT_COMMAND=${command}"
           "-DINFILE=${infile}"
           "-DOUTFILE=${outfile}"
           ${tiff_test_extra_args}
           -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffTest.cmake")
endfunction()

function(tiff_test_stdout_noargs name command)
  add_test(NAME "${name}"
          COMMAND "${CMAKE_COMMAND}"
          "-DSTDOUT_COMMAND=$<TARGET_FILE:${command}>"
          ${tiff_test_extra_args}
          -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffTest.cmake"
          WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  set_tests_properties("${name}" PROPERTIES ENVIRONMENT
          "srcdir=${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(tiff_test_reader name command infile)
  add_test(NAME "${name}"
           COMMAND "${CMAKE_COMMAND}"
           "-DREADER_COMMAND=${command}"
           "-DINFILE=${infile}"
           ${tiff_test_extra_args}
           -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffTest.cmake")
endfunction()

function(add_convert_test_multi commandname1 commandname2 commandname3
      categoryname commandargs1 commandargs2 commandargs3
      image validate)
  string(REPLACE " " "^" escaped_commandargs1 "${commandargs1}")
  string(REPLACE " " "^" escaped_commandargs2 "${commandargs2}")
  string(REPLACE " " "^" escaped_commandargs3 "${commandargs3}")
  get_filename_component(name "${image}" NAME)
  get_filename_component(base "${image}" NAME_WE)
  set(testname "${commandname1}-${categoryname}-${base}")
  if(commandname1)
    set(command1
        "$<TARGET_FILE:${commandname1}>^${escaped_commandargs1}")
  else()
    set(command1)
  endif()
  if(commandname2)
    set(command2
        "$<TARGET_FILE:${commandname2}>^${escaped_commandargs2}")
  else()
    set(command2)
  endif()
  if(commandname3)
    set(command3
        "$<TARGET_FILE:${commandname3}>^${escaped_commandargs3}")
  else()
    set(command3)
  endif()
  set(infile "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
  set(outfile "${TEST_OUTPUT}/${commandname1}-${categoryname}-${base}.tiff")
  tiff_test_convert("${testname}" "${command1}" "${command2}" "${command3}"
                    "${infile}" "${outfile}" "${validate}")
endfunction()

function(add_convert_test commandname
      categoryname commandargs
      image validate)
    add_convert_test_multi("${commandname}" "" ""
                           "${categoryname}" "${commandargs}" "" ""
                           "${image}" "${validate}")
endfunction()

function(add_convert_tests_multi commandname1 commandname2 commandname3
      categoryname
      commandargs1 commandargs2 commandargs3
      images validate)
  foreach(file ${${images}})
    add_convert_test_multi("${commandname1}" "${commandname2}"
                           "${commandname3}" "${categoryname}"
                           "${commandargs1}" "${commandargs2}"
                           "${commandargs3}" "${file}" "${validate}")
  endforeach()
endfunction()

function(add_convert_tests commandname categoryname commandargs images validate)
  foreach(file ${${images}})
    add_convert_test("${commandname}" "${categoryname}"
                     "${commandargs}" "${file}" "${validate}")
  endforeach()
endfunction()

function(add_stdout_test commandname suffix commandargs image)
  string(REPLACE " " "^" escaped_commandargs "${commandargs}")
  get_filename_component(name "${image}" NAME)
  get_filename_component(base "${image}" NAME_WE)
  set(testname "${commandname}-${base}")
  set(command "$<TARGET_FILE:${commandname}>^${escaped_commandargs}")
  set(infile "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
  set(outfile "${TEST_OUTPUT}/${commandname}-${name}")
  string(REGEX REPLACE "\\.tiff\$" "" name "${name}")
  tiff_test_stdout("${testname}" "${suffix}" "${command}" "${infile}" "${outfile}")
endfunction()

function(add_reader_test commandname commandargs image)
  string(REPLACE " " "^" escaped_commandargs "${commandargs}")
  get_filename_component(name "${image}" NAME)
  get_filename_component(base "${image}" NAME_WE)
  set(testname "${commandname}-${base}")
  set(command "$<TARGET_FILE:${commandname}>^${escaped_commandargs}")
  set(infile "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
  tiff_test_reader("${testname}" "${command}" "${infile}")
endfunction()

# Plain executables with no arguments
foreach(target IN LISTS simple_tests)
  tiff_test_stdout_noargs("${target}" "${target}")
endforeach()

if(tiff-tools)
# Reading / writing TIFF images with different encoding schemes.
add_convert_tests(tiffcp TIFFIMG    ""                         TIFFIMAGES TRUE)

# Convert some images to another coding scheme.
  add_convert_test(tiffcp   g3         "-c g3"         "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test(tiffcp   g31d       "-c g3:1d"      "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test(tiffcp   g31dfill   "-c g3:1d:fill" "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test(tiffcp   g32d       "-c g3:2d"      "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test(tiffcp   g32dfill   "-c g3:2d:fill" "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test(tiffcp   g4         "-c g4"         "images/miniswhite-1c-1b.tiff" FALSE)
  add_convert_test_multi(tiffcp tiffcp "" logluv "-c none" "-c sgilog" ""
                         "images/logluv-3c-16b.tiff"    FALSE)
  add_convert_test(tiffcp   none       "-c none"       "images/quad-lzw-compat.tiff" FALSE)
  add_convert_test(tiffcp   noner1     "-c none -r 1"  "images/lzw-single-strip.tiff" FALSE)
  add_convert_test(tiffcp   float64_lzw_2_le "-c lzw:2"       "images/test_float64_predictor2_le_lzw.tif" FALSE)
  add_convert_test(tiffcp   float64_lzw_2_be "-c lzw:2"       "images/test_float64_predictor2_be_lzw.tif" FALSE)
  if(WEBP_SUPPORT)
      add_convert_test(tiffcp   webp_lossless_rgba_alpha_fully_opaque "-c none"       "images/webp_lossless_rgba_alpha_fully_opaque.tif" FALSE)
  endif()
# Old-JPEG tests
#--- tiffcp does not support subsampled images, as the OldJPEG ones are ---
#    ToDo: a test for OldJPEG is missing.
#add_convert_tests(test_tiffcp OldJPEG    ""                         TIFFIMAGES_OJPEG TRUE)

  # tiffdump
  add_reader_test(tiffdump "" "images/miniswhite-1c-1b.tiff")

  # tiffinfo
  add_reader_test(tiffinfo "-c -D -d -j -s" "images/custom_dir_EXIF_GPS.tiff")
  add_reader_test(tiffinfo "-c -D -d -j -s" "images/minisblack-1c-16b.tiff")
  add_reader_test(tiffinfo " " "images/tiff_with_subifd_chain.tif")

  # tiffcp split/join
  foreach(image ${UNCOMPRESSEDIMAGES})
    list(APPEND ESCAPED_UNCOMPRESSED "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
  endforeach()
  string(REPLACE ";" "^" ESCAPED_UNCOMPRESSED "${ESCAPED_UNCOMPRESSED}")
  add_test(NAME "tiffcp-split"
           COMMAND "${CMAKE_COMMAND}"
           "-DTESTFILES=${ESCAPED_UNCOMPRESSED}"
           "-DCONJOINED=${TEST_OUTPUT}/tiffcp-split-conjoined.tiff"
           "-DSPLITFILE=${TEST_OUTPUT}/tiffcp-split-split-"
           ${tiff_test_extra_args}
           -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffSplitTest.cmake")
  add_test(NAME "tiffcp-split-join"
           COMMAND "${CMAKE_COMMAND}"
           "-DTESTFILES=${ESCAPED_UNCOMPRESSED}"
           "-DCONJOINED=${TEST_OUTPUT}/tiffcp-split-join-conjoined.tiff"
           "-DSPLITFILE=${TEST_OUTPUT}/tiffcp-split-join-split-"
           "-DRECONJOINED=${TEST_OUTPUT}/tiffcp-split-join-reconjoined.tiff"
           ${tiff_test_extra_args}
           -P "${CMAKE_CURRENT_SOURCE_DIR}/TiffSplitTest.cmake")

endif()
