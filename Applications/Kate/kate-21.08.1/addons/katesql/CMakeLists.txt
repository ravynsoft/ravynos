find_package(KF5Wallet ${KF5_DEP_VERSION} QUIET)
set_package_properties(KF5Wallet PROPERTIES PURPOSE "Required to build the katesql addon")

find_package(Qt5Sql ${QT_MIN_VERSION} QUIET)
set_package_properties(Qt5Sql PROPERTIES PURPOSE "Required to build the katesql addon")

if(NOT KF5Wallet_FOUND OR NOT Qt5Sql_FOUND)
  return()
endif()

add_library(katesqlplugin MODULE "")
target_compile_definitions(katesqlplugin PRIVATE TRANSLATION_DOMAIN="katesql")

target_link_libraries(
  katesqlplugin 
  PRIVATE
    KF5::I18n
    KF5::TextEditor
    KF5::Wallet
    Qt5::Sql
)

target_sources(
  katesqlplugin
  PRIVATE
    katesqlplugin.cpp
    katesqlview.cpp
    connectionmodel.cpp
    sqlmanager.cpp
    cachedsqlquerymodel.cpp
    dataoutputmodel.cpp
    dataoutputview.cpp
    dataoutputwidget.cpp
    textoutputwidget.cpp
    schemawidget.cpp
    schemabrowserwidget.cpp
    connectionwizard.cpp
    katesqlconfigpage.cpp
    exportwizard.cpp
    outputstylewidget.cpp
    outputwidget.cpp
    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(katesqlplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS katesqlplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
