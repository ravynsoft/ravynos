add_library(katereplicodeplugin MODULE "")
target_compile_definitions(katereplicodeplugin PRIVATE TRANSLATION_DOMAIN="kate-replicode-plugin")

target_link_libraries(
  katereplicodeplugin
  PRIVATE
    KF5::I18n
    KF5::TextEditor
)

ki18n_wrap_ui(UI_SOURCES config.ui)
target_sources(katereplicodeplugin PRIVATE ${UI_SOURCES})

target_sources(
  katereplicodeplugin
  PRIVATE
    replicodeplugin.cpp
    replicodeconfigpage.cpp
    replicodeconfig.cpp
    replicodesettings.cpp
    replicodeview.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katereplicodeplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katereplicodeplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
