cmake_minimum_required(VERSION 3.16)

project(ravynOSMenu)
 
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(KF5 REQUIRED COMPONENTS
    Plasma
    I18n
    KIO
)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Quick
    Core
    Widgets
    DBus
)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
