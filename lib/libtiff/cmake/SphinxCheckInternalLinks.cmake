# Check "local" sphinx links, using files in the build tree and in the
# install tree (other components)

message(STATUS "Checking internal links")

set(failed OFF)

file(STRINGS "${OUTPUT}" output)
file(REMOVE "${OUTPUT}.new")
foreach(line IN LISTS output)
  # Parse the filename, link type, link and anchor from each line
  string(REGEX MATCH "^.*: \\[.*\\] .*" line_match "${line}")
  string(REGEX MATCH "^.*: \\[.*\\] .*#.*" anchor_match "${line}")
  if(line_match)
    string(REGEX REPLACE "^(.*): \\[(.*)\\] (.*)" "\\1" filename "${line}")
    string(REGEX REPLACE "^(.*): \\[(.*)\\] (.*)" "\\2" type "${line}")
    if(anchor_match)
      string(REGEX REPLACE "^(.*): \\[(.*)\\] (.*)#(.*)" "\\3" link "${line}")
      string(REGEX REPLACE "^(.*): \\[(.*)\\] (.*)#(.*)" "\\4" link "${anchor}")
    else()
      string(REGEX REPLACE "^(.*): \\[(.*)\\] (.*)" "\\3" link "${line}")
      unset(anchor)
    endif()

    # Only check local links
    if(type STREQUAL "local")
      # Compute path relative to our install path
      file(RELATIVE_PATH relpath "/" "/${SPHINX_INSTALL_PATH}/${link}")

      # Check sphinx link in the install tree
      unset(linkfile)
      if(EXISTS "${EXTERNAL_REFERENCE}/${relpath}")
        set(linkfile "${EXTERNAL_REFERENCE}/${relpath}")
      endif()
      # Check sphinx link in the build tree
      if(EXISTS "${INTERNAL_REFERENCE}/${link}")
        set(linkfile "${INTERNAL_REFERENCE}/${link}")
      endif()
      # Check doxygen link in the install tree
      if(EXISTS "${DOXYGEN_REFERENCE}/${link}")
        set(linkfile "${DOXYGEN_REFERENCE}/${link}")
      endif()
      # Check doxygen link in the build tree
      if(DOXYGEN_REFERENCE AND DOXYGEN_INSTALL_PATH)
        string(REGEX REPLACE "^${DOXYGEN_INSTALL_PATH}/(.*)" "\\1" doxygen_path "${relpath}")
        if(EXISTS "${DOXYGEN_REFERENCE}/${doxygen_path}")
          set(linkfile "${DOXYGEN_REFERENCE}/${doxygen_path}")
        endif()
      endif()
      # Check link and any anchor.  If the link does not exist, or the
      # anchor does not exist, set the link type to broken when
      # rewriting the output file with our test results.
      if(linkfile)
        if(anchor)
          file(STRINGS "${linkfile}" anchor_matches REGEX "<a +class=\"anchor\" +id=\"${anchor}\"")
          if(anchor_matches)
            file(APPEND "${OUTPUT}.new" "${filename}: [${type}] ${link}#${anchor}\n")
          else()
            set(failed ON)
            file(APPEND "${OUTPUT}.new" "${filename}: [broken] ${link}#${anchor}\n")
            message(STATUS "Broken link: ${filename}: ${link}#${anchor}")
          endif()
        else()
          file(APPEND "${OUTPUT}.new" "${filename}: [${type}] ${link}\n")
        endif()
      else()
        set(failed ON)
        file(APPEND "${OUTPUT}.new" "${filename}: [broken] ${link}\n")
        message(STATUS "Broken link: ${filename}: ${link}")
      endif()
    else()
      if(type STREQUAL "broken")
        set(failed ON)
        message(STATUS "Broken link: ${filename}: ${link}")
      endif()
      file(APPEND "${OUTPUT}.new" "${filename}: [${type}] ${link}\n")
    endif()
  else()
    file(APPEND "${OUTPUT}.new" "${line}\n")
  endif()
endforeach()

file(RENAME "${OUTPUT}.new" "${OUTPUT}")

if(failed)
  message(FATAL_ERROR "Broken links detected")
endif()
