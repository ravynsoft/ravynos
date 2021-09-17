add_library(katectagsplugin MODULE "")
target_compile_definitions(katectagsplugin PRIVATE TRANSLATION_DOMAIN="kate-ctags-plugin")
target_link_libraries(katectagsplugin PRIVATE KF5::I18n KF5::TextEditor)

ki18n_wrap_ui(UI_SOURCES kate_ctags.ui CTagsGlobalConfig.ui)
target_sources(katectagsplugin PRIVATE ${UI_SOURCES})

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
  DEBUG_SOURCES
  HEADER kate_ctags_debug.h
  IDENTIFIER KTECTAGS
  CATEGORY_NAME "katectagsplugin"
)
target_sources(katectagsplugin PRIVATE ${DEBUG_SOURCES})

target_sources(
  katectagsplugin
  PRIVATE
    readtags.c
    tags.cpp
    ctagskinds.cpp
    kate_ctags_view.cpp
    kate_ctags_plugin.cpp
    gotosymbolmodel.cpp
    gotoglobalsymbolmodel.cpp
    gotosymboltreeview.cpp
    gotosymbolwidget.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katectagsplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katectagsplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
