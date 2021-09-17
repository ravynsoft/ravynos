find_package(KF5I18n ${KF5_DEP_VERSION} QUIET)
if(NOT KF5I18n_FOUND)
    return()
endif()

find_package(KF5IconThemes ${KF5_DEP_VERSION} QUIET)
if(NOT KF5IconThemes_FOUND)
    return()
endif()

find_package(Qt5Test ${QT_MIN_VERSION} QUIET REQUIRED)

add_library(externaltoolsplugin MODULE "")
target_compile_definitions(externaltoolsplugin PRIVATE TRANSLATION_DOMAIN="kateexternaltoolsplugin")
target_link_libraries(externaltoolsplugin PRIVATE
    KF5::CoreAddons
    KF5::IconThemes
    KF5::TextEditor
    KF5::I18n
)

ki18n_wrap_ui(UI_SOURCES configwidget.ui
    tooldialog.ui
    toolview.ui
)

target_sources(
  externaltoolsplugin
  PRIVATE
    externaltoolsplugin.cpp
    kateexternaltoolsview.cpp
    katetoolrunner.cpp
    kateexternaltool.cpp
    kateexternaltoolscommand.cpp
    kateexternaltoolsconfigwidget.cpp
    plugin.qrc
    ${UI_SOURCES}
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(externaltoolsplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS externaltoolsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor )

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
