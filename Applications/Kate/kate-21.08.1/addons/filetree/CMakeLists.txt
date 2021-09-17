find_package(KF5GuiAddons ${KF5_DEP_VERSION} QUIET)
set_package_properties(KF5GuiAddons PROPERTIES PURPOSE "Required to build the katefiletree addon")

if(NOT KF5GuiAddons_FOUND)
  return()
endif()

add_library(katefiletreeplugin MODULE "")
target_compile_definitions(katefiletreeplugin PRIVATE TRANSLATION_DOMAIN="katefiletree")

target_link_libraries(
  katefiletreeplugin
  PUBLIC
    KF5::I18n
    KF5::TextEditor
    KF5::GuiAddons
)

target_sources(
  katefiletreeplugin
  PRIVATE
    katefiletree.cpp
    katefiletreeconfigpage.cpp
    katefiletreemodel.cpp
    katefiletreeplugin.cpp
    katefiletreepluginsettings.cpp
    katefiletreeproxymodel.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katefiletreeplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katefiletreeplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)

if(BUILD_TESTING)
  add_subdirectory(autotests)
endif()
