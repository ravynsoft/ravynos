cmake_minimum_required(VERSION 3.5)
project(microatf-tests LANGUAGES C)

include(CTest)

find_package(microatf REQUIRED)
include(ATFTest)

macro(atf_test _testname)
  add_executable("${_testname}" "${_testname}.c")
  target_link_libraries("${_testname}" PRIVATE microatf::microatf-c)
  atf_discover_tests("${_testname}" ${ARGN})
endmacro()

#

atf_test(atf-test)
atf_test(atf-test2)
