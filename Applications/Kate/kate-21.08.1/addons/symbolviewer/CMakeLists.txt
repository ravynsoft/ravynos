add_library(katesymbolviewerplugin MODULE "")
target_compile_definitions(katesymbolviewerplugin PRIVATE TRANSLATION_DOMAIN="katesymbolviewer")
target_link_libraries(katesymbolviewerplugin PRIVATE KF5::I18n KF5::TextEditor)

target_sources(
  katesymbolviewerplugin 
  PRIVATE
    cpp_parser.cpp 
    tcl_parser.cpp 
    fortran_parser.cpp 
    perl_parser.cpp 
    php_parser.cpp 
    xslt_parser.cpp 
    xml_parser.cpp 
    ruby_parser.cpp 
    python_parser.cpp 
    bash_parser.cpp 
    ecma_parser.cpp 
    plugin_katesymbolviewer.cpp 
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katesymbolviewerplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katesymbolviewerplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
