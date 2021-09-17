add_library(kategitblameplugin MODULE "")
target_compile_definitions(kategitblameplugin PRIVATE TRANSLATION_DOMAIN="kategitblameplugin")
target_link_libraries(kategitblameplugin PRIVATE KF5::I18n KF5::TextEditor)

target_sources(
  kategitblameplugin
  PRIVATE
    kategitblameplugin.cpp
    gitblametooltip.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(kategitblameplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS kategitblameplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
