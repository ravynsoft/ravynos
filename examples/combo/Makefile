# This Makefile and the subdirectories below it will generate a complex App
# bundle called Bar.app in the current directory. This bundle contains a
# bundled Framework and an executable that links against the Framework,
# resulting in a portable application.
#
# Subdir 'app' contains a standard App Makefile that refers to a framework
# using a relative path. This will become relative to the executable's
# runtime location so it can always be found in the bundle.
#
# Subdir 'framework' contains a standard Framework Makefile that installs to
# the App bundle created here.
#
# In order to build the app, we need the framework, and in order to install
# the framework to the app bundle, it must exist. To resolve this circular
# dependency, we create the bundle tree, then build the framework into it,
# and finally build the app against the framework.
#
# Bar.app
# `-- Contents
#     |-- Frameworks
#     |   `-- Foo.framework
#     |       |-- Headers -> Versions/A/Headers
#     |       |-- Modules -> Versions/A/Modules
#     |       |-- Resources -> Versions/A/Resources
#     |       `-- Versions
#     |           |-- A
#     |           |   |-- Headers
#     |           |   |   `-- foo.h
#     |           |   |-- Modules
#     |           |   |-- Resources
#     |           |   |   `-- Info.plist
#     |           |   `-- libFoo.so
#     |           `-- Current -> A
#     |-- Airyx
#     |   `-- Bar
#     |-- Info.plist
#     |-- PkgInfo
#     `-- Resources
#         `-- rsc
#             `-- sample.txt

bundle: .PHONY
	export MAKEOBJDIR=${.CURDIR}; ${MAKE} -C app Bar.app
	mkdir -p ${.CURDIR}/Bar.app/Contents/Frameworks
	(export MAKEOBJDIR=${.CURDIR}/Bar.app/Contents/Frameworks; \
		${MAKE} -C framework; \
		${MAKE} -C framework clean)
	export MAKEOBJDIR=${.CURDIR}; ${MAKE} -C app
