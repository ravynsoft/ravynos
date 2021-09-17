add_library(katecolorpickerplugin MODULE "")
target_compile_definitions(katecolorpickerplugin PRIVATE TRANSLATION_DOMAIN="katecolorpickerplugin")
target_link_libraries(katecolorpickerplugin PRIVATE KF5::I18n KF5::TextEditor)

target_sources(
  katecolorpickerplugin
  PRIVATE
    katecolorpickerplugin.cpp
    colorpickerconfigpage.cpp
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katecolorpickerplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katecolorpickerplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
