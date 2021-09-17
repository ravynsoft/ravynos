find_package(Qt5Concurrent ${QT_MIN_VERSION} QUIET)
find_package(KF5ItemViews ${KF5_DEP_VERSION} QUIET)
set_package_properties(KF5ItemViews PROPERTIES PURPOSE "Required to build the search addon")

if(NOT KF5ItemViews_FOUND OR NOT Qt5Concurrent_FOUND)
  return()
endif()

add_library(katesearchplugin MODULE "")
target_compile_definitions(katesearchplugin PRIVATE TRANSLATION_DOMAIN="katesearch")

target_link_libraries(
  katesearchplugin
  PRIVATE
    Qt5::Concurrent
    KF5::I18n
    KF5::ItemViews
    KF5::TextEditor
)

ki18n_wrap_ui(UI_SOURCES search.ui results.ui)
target_sources(katesearchplugin PRIVATE ${UI_SOURCES})

target_sources(
  katesearchplugin
  PRIVATE
    FolderFilesList.cpp
    KateSearchCommand.cpp
    MatchModel.cpp
    SearchDiskFiles.cpp
    htmldelegate.cpp
    plugin.qrc
    plugin_search.cpp
    search_open_files.cpp
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katesearchplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katesearchplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
