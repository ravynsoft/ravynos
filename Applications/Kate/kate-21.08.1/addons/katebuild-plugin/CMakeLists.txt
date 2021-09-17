add_library(katebuildplugin MODULE "")
target_compile_definitions(katebuildplugin PRIVATE TRANSLATION_DOMAIN="katebuild-plugin")
target_link_libraries(katebuildplugin PRIVATE KF5::I18n KF5::TextEditor)

ki18n_wrap_ui(UI_SOURCES build.ui SelectTargetUi.ui)
target_sources(katebuildplugin PRIVATE ${UI_SOURCES})

target_sources(
  katebuildplugin
  PRIVATE
    plugin_katebuild.cpp
    targets.cpp
    TargetHtmlDelegate.cpp
    TargetModel.cpp
    UrlInserter.cpp
    SelectTargetView.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katebuildplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katebuildplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
