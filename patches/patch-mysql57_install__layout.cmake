--- cmake/install_layout.cmake.orig	2021-07-06 21:52:58.226356000 -0400
+++ cmake/install_layout.cmake	2021-07-06 21:58:23.210828000 -0400
@@ -102,7 +102,7 @@
   ELSEIF(INSTALL_LAYOUT MATCHES "SVR4")
     SET(default_prefix "/opt/mysql/mysql")
   ELSE()
-    SET(default_prefix "/usr/local/mysql")
+    SET(default_prefix "/usr")
   ENDIF()
   IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
     SET(CMAKE_INSTALL_PREFIX ${default_prefix}
@@ -115,7 +115,7 @@
     " Choose between ${VALID_INSTALL_LAYOUTS}" )
   ENDIF()
 
-  SET(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc"
+  SET(SYSCONFDIR "/etc"
     CACHE PATH "config directory (for my.cnf)")
   MARK_AS_ADVANCED(SYSCONFDIR)
 ENDIF()
@@ -169,7 +169,7 @@
 #
 SET(INSTALL_DOCDIR_STANDALONE           "docs")
 SET(INSTALL_DOCREADMEDIR_STANDALONE     ".")
-SET(INSTALL_MANDIR_STANDALONE           "man")
+SET(INSTALL_MANDIR_STANDALONE           "share/man")
 SET(INSTALL_INFODIR_STANDALONE          "docs")
 #
 SET(INSTALL_SHAREDIR_STANDALONE         "share")
@@ -225,7 +225,7 @@
 #
 SET(INSTALL_DOCDIR_FREEBSD           "docs")
 SET(INSTALL_DOCREADMEDIR_FREEBSD     ".")
-SET(INSTALL_MANDIR_FREEBSD           "man")
+SET(INSTALL_MANDIR_FREEBSD           "share/man")
 SET(INSTALL_INFODIR_FREEBSD          "docs")
 #
 SET(INSTALL_SHAREDIR_FREEBSD         "share")
