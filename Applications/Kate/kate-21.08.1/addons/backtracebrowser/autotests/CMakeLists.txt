include(ECMMarkAsTest)

add_executable(btbrowser_test "")
target_include_directories(btbrowser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Qt5Test ${QT_MIN_VERSION} QUIET REQUIRED)
target_link_libraries(
  btbrowser_test 
  PRIVATE 
    Qt5::Widgets
    Qt5::Test
)

target_sources(btbrowser_test PRIVATE
  btbrowsertest.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/../btparser.cpp
)

add_test(NAME plugin-btbrowser_test COMMAND btbrowser_test)
ecm_mark_as_test(btbrowser_test)
