--- libtaskmanager/xwindowtasksmodel.cpp.orig	2021-09-04 00:56:57 UTC
+++ libtaskmanager/xwindowtasksmodel.cpp
@@ -239,7 +239,7 @@ void XWindowTasksModel::Private::addWindow(WId window)
     }
 
     // Ignore NET::Tool and other special window types; they are not considered tasks.
-    if (wType != NET::Normal && wType != NET::Override && wType != NET::Unknown && wType != NET::Dialog && wType != NET::Utility) {
+    if (wType != NET::Desktop && wType != NET::Normal && wType != NET::Override && wType != NET::Unknown && wType != NET::Dialog && wType != NET::Utility) {
         return;
     }
 
