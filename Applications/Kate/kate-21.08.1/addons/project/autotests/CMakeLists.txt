include(ECMMarkAsTest)

add_executable(projectplugin_test "")
target_include_directories(projectplugin_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Qt5Test ${QT_MIN_VERSION} QUIET REQUIRED)
target_link_libraries(
  projectplugin_test 
  PRIVATE
    KF5::I18n
    KF5::TextEditor
    Qt5::Test
)

target_sources(
  projectplugin_test 
  PRIVATE
    test1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../fileutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../kateprojectcodeanalysistool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/kateprojectcodeanalysistoolshellcheck.cpp
)

add_test(NAME plugin-project_test COMMAND projectplugin_test)
ecm_mark_as_test(projectplugin_test)
