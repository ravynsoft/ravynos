set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PythonLibrary)

include(PythonMacros)

find_package(SIP)
include(SIPMacros)

if(SIP_VERSION STRLESS "040f03")    # These version numbers also appear in ../CMakeLists.txt
    message(FATAL_ERROR "The version of SIP found is too old. 4.15.3 or later is needed.")
endif()

find_package(PyQt5)
if(PYQT5_VERSION STRLESS "050101")  # These version numbers also appear in ../CMakeLists.txt
    message(FATAL_ERROR "The version of PyQt found is too old. 5.1.1 or later is required.")
endif()

set(SIP_INCLUDES ${PYQT5_SIP_DIR} sip)
set(SIP_CONCAT_PARTS 8)
set(SIP_TAGS ALL WS_X11 ${PYQT5_VERSION_TAG})
set(SIP_DISABLE_FEATURES VendorID PyQt_NoPrintRangeBug)

# Use an extra option when compiling on Python 3.
if (PYTHON_VERSION_MAJOR GREATER 2)
    if(PYQT5_VERSION STRGREATER "040904")
	    # Disable for newer PyQt
	    set(SIP_EXTRA_OPTIONS -P -g)
    else ()
	    set(SIP_EXTRA_OPTIONS -g)
    endif()
else (PYTHON_VERSION_MAJOR GREATER 2)
    if(PYQT5_VERSION STRGREATER "040904")
	# Disable for newer PyQt
        set(SIP_EXTRA_OPTIONS -P -g  -x Py_v3)
    else ()
	    set(SIP_EXTRA_OPTIONS -g -x Py_v3)
    endif()
endif ()

set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} ${PYQT5_SIP_FLAGS})

include_directories(
    "${SIP_INCLUDE_DIR}"
)

add_definitions(-D_REENTRANT -DSIP_PROTECTED_IS_PUBLIC -Dprotected=public)

file(GLOB qtermwidget_files_sip sip/*.sip)
set(SIP_EXTRA_FILES_DEPEND "${qtermwidget_files_sip}")
add_sip_python_module(QTermWidget sip/qtermwidget.sip qtermwidget5)

python_install(__init__.py "${PYTHON_SITE_PACKAGES_INSTALL_DIR}/PyQt5/qtermwidget")

set (SIP_FILES_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/sip")

install(DIRECTORY sip/ DESTINATION "${SIP_FILES_INSTALL_DIR}/PyQt5/qtermwidget"
    PATTERN "*~" EXCLUDE    # This sucks, why can't I just whitelist what I _do_ want?
    PATTERN ".svn" EXCLUDE
    PATTERN "*.in" EXCLUDE
)
