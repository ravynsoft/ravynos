add_library (tabswitcherplugin MODULE "")
target_compile_definitions(tabswitcherplugin PRIVATE TRANSLATION_DOMAIN="tabswitcherplugin")
target_link_libraries(tabswitcherplugin PRIVATE KF5::I18n KF5::TextEditor)

target_sources(
  tabswitcherplugin 
  PRIVATE
    tabswitcher.cpp
    tabswitcherfilesmodel.cpp
    tabswitchertreeview.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(tabswitcherplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS tabswitcherplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)

if(BUILD_TESTING)
  add_subdirectory(autotests)
  add_subdirectory(tests)
endif()
