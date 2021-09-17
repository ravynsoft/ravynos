if(WIN32)
  return()
endif()

add_library(katekonsoleplugin MODULE "")
target_compile_definitions(katekonsoleplugin PRIVATE TRANSLATION_DOMAIN="katekonsoleplugin")
target_link_libraries(katekonsoleplugin PRIVATE KF5::I18n KF5::TextEditor)

target_sources(
  katekonsoleplugin 
  PRIVATE
    kateconsole.cpp 
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katekonsoleplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katekonsoleplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
