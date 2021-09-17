find_package(Qt5Concurrent ${QT_MIN_VERSION} QUIET)
find_package(
  KF5 ${KF5_DEP_VERSION}
  QUIET
  COMPONENTS
    GuiAddons
    NewStuff
)

set_package_properties(KFGuiAddons PROPERTIES PURPOSE "Required to build the project addon")
set_package_properties(KF5NewStuff PROPERTIES PURPOSE "Required to build the project addon")

if(NOT KF5_FOUND OR NOT Qt5Concurrent_FOUND)
  return()
endif()

add_library(kateprojectplugin MODULE "")
target_compile_definitions(kateprojectplugin PRIVATE TRANSLATION_DOMAIN="kateproject")

target_link_libraries(
  kateprojectplugin
  PRIVATE
    Qt5::Concurrent
    KF5::GuiAddons
    KF5::I18n
    KF5::NewStuff
    KF5::TextEditor
)

target_include_directories(
    kateprojectplugin
    PUBLIC
    ${CMAKE_SOURCE_DIR}/shared
)

include(CheckFunctionExists)
check_function_exists(ctermid HAVE_CTERMID)

if(HAVE_CTERMID)
  target_compile_definitions(kateprojectplugin PRIVATE HAVE_CTERMID)
endif()

target_sources(
  kateprojectplugin
  PRIVATE
    fileutil.cpp
    kateprojectplugin.cpp
    kateprojectpluginview.cpp
    kateproject.cpp
    kateprojectworker.cpp
    kateprojectitem.cpp
    kateprojectview.cpp
    kateprojectviewtree.cpp
    kateprojecttreeviewcontextmenu.cpp
    kateprojectinfoview.cpp
    kateprojectcompletion.cpp
    kateprojectindex.cpp
    kateprojectinfoviewindex.cpp
    kateprojectinfoviewterminal.cpp
    kateprojectinfoviewcodeanalysis.cpp
    kateprojectinfoviewnotes.cpp
    kateprojectconfigpage.cpp
    kateprojectcodeanalysistool.cpp
    branchesdialog.cpp
    branchcheckoutdialog.cpp
    branchesdialogmodel.cpp
    gitwidget.cpp
    gitstatusmodel.cpp
    gitcommitdialog.cpp
    stashdialog.cpp
    filehistorywidget.cpp
    ${CMAKE_SOURCE_DIR}/shared/quickdialog.cpp
    pushpulldialog.cpp
    comparebranchesview.cpp

    tools/kateprojectcodeanalysistoolcppcheck.cpp
    tools/kateprojectcodeanalysistoolflake8.cpp
    tools/kateprojectcodeanalysistoolshellcheck.cpp
    tools/kateprojectcodeanalysisselector.cpp
    tools/kateprojectcodeanalysistoolclazy.cpp
    tools/kateprojectcodeanalysistoolclazycurrent.cpp

    git/gitdiff.cpp
    git/gitutils.cpp
    git/gitstatus.cpp

    plugin.qrc
)

# ensure we are able to load plugins pre-install, too, directories must match!
set_target_properties(kateprojectplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ktexteditor")
install(TARGETS kateprojectplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
install(FILES kateproject.example DESTINATION ${KDE_INSTALL_DATADIR}/kateproject)

if(BUILD_TESTING)
  add_subdirectory(autotests)
endif()
