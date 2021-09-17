# Hasn't been ported to Windows yet.
if(WIN32)
  return()
endif()

add_library(kategdbplugin MODULE "")
target_compile_definitions(kategdbplugin PRIVATE TRANSLATION_DOMAIN="kategdbplugin")
target_link_libraries(kategdbplugin PRIVATE KF5::I18n KF5::TextEditor)

ki18n_wrap_ui(UI_SOURCES advanced_settings.ui)
target_sources(kategdbplugin PRIVATE ${UI_SOURCES})

target_sources(
  kategdbplugin
  PRIVATE
    plugin_kategdb.cpp
    debugview.cpp
    configview.cpp
    ioview.cpp
    localsview.cpp
    advanced_settings.cpp
    plugin.qrc 
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(kategdbplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS kategdbplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
