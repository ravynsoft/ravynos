include(ECMMarkAsTest)

add_executable(filetree_model_test "")
target_include_directories(filetree_model_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Qt5Test ${QT_MIN_VERSION} QUIET REQUIRED)
target_link_libraries(
  filetree_model_test 
  PRIVATE
    KF5::GuiAddons
    KF5::I18n
    KF5::TextEditor
    Qt5::Test
)

target_sources(
  filetree_model_test 
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../katefiletreemodel.cpp
    filetree_model_test.cpp 
    document_dummy.cpp
)

add_test(NAME kateapp-filetree_model_test COMMAND filetree_model_test)
ecm_mark_as_test(filetree_model_test)
