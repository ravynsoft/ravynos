add_library(katecloseexceptplugin MODULE "")
target_compile_definitions(katecloseexceptplugin PRIVATE TRANSLATION_DOMAIN="katecloseexceptplugin")

target_link_libraries(
  katecloseexceptplugin
  PRIVATE
    KF5::I18n
    KF5::TextEditor
)

ki18n_wrap_ui(UI_SOURCES close_confirm_dialog.ui)
target_sources(katecloseexceptplugin PRIVATE ${UI_SOURCES})

target_sources(
  katecloseexceptplugin
  PRIVATE
    close_confirm_dialog.cpp
    close_except_plugin.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katecloseexceptplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katecloseexceptplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
