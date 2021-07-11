--- cmakescripts/GNUInstallDirs.cmake.orig	2021-06-30 12:42:54 UTC
+++ cmakescripts/GNUInstallDirs.cmake
@@ -307,11 +307,11 @@ GNUInstallDirs_set_install_dir(INFODIR
   "The directory into which info documentation files should be installed")
 
 if(NOT DEFINED CMAKE_INSTALL_DEFAULT_MANDIR)
-  if(CMAKE_SYSTEM_NAME MATCHES "^(.*BSD|DragonFly)$")
-    set(CMAKE_INSTALL_DEFAULT_MANDIR "man")
-  else()
+#  if(CMAKE_SYSTEM_NAME MATCHES "^(.*BSD|DragonFly)$")
+#    set(CMAKE_INSTALL_DEFAULT_MANDIR "man")
+#  else()
     set(CMAKE_INSTALL_DEFAULT_MANDIR "<CMAKE_INSTALL_DATAROOTDIR>/man")
-  endif()
+#  endif()
 endif()
 GNUInstallDirs_set_install_dir(MANDIR
   "The directory under which man pages should be installed")
