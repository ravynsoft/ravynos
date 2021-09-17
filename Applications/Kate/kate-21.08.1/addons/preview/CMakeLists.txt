add_library(ktexteditorpreviewplugin MODULE "")
target_compile_definitions(ktexteditorpreviewplugin PRIVATE TRANSLATION_DOMAIN="ktexteditorpreviewplugin")
target_link_libraries(ktexteditorpreviewplugin PRIVATE KF5::I18n KF5::TextEditor)

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
  DEBUG_SOURCES
  HEADER ktexteditorpreview_debug.h
  IDENTIFIER KTEPREVIEW
  CATEGORY_NAME "ktexteditorpreviewplugin"
)
target_sources(ktexteditorpreviewplugin PRIVATE ${DEBUG_SOURCES})

target_sources(
  ktexteditorpreviewplugin
  PRIVATE
    ktexteditorpreviewplugin.cpp
    ktexteditorpreviewview.cpp
    previewwidget.cpp
    kpartview.cpp
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(ktexteditorpreviewplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS ktexteditorpreviewplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
