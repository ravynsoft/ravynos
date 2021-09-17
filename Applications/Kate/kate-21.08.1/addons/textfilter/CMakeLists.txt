add_library(textfilterplugin MODULE "")
target_compile_definitions(textfilterplugin PRIVATE TRANSLATION_DOMAIN="katetextfilter")
target_link_libraries(textfilterplugin PRIVATE KF5::I18n KF5::TextEditor)

ki18n_wrap_ui(UI_SOURCES textfilterwidget.ui)
target_sources(textfilterplugin PRIVATE ${UI_SOURCES})

target_sources(
  textfilterplugin
  PRIVATE
    plugin_katetextfilter.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(textfilterplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS textfilterplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
