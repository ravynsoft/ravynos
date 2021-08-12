--- compiler/rustc_metadata/src/native_libs.rs.orig	2021-08-11 11:27:28.399009000 -0400
+++ compiler/rustc_metadata/src/native_libs.rs	2021-08-11 11:28:41.809565000 -0400
@@ -152,13 +152,13 @@
             return;
         }
         let is_osx = self.tcx.sess.target.is_like_osx;
-        if lib.kind == NativeLibKind::Framework && !is_osx {
-            let msg = "native frameworks are only available on macOS targets";
-            match span {
-                Some(span) => struct_span_err!(self.tcx.sess, span, E0455, "{}", msg).emit(),
-                None => self.tcx.sess.err(msg),
-            }
-        }
+        //if lib.kind == NativeLibKind::Framework && !is_osx {
+        //    let msg = "native frameworks are only available on macOS targets";
+        //    match span {
+        //        Some(span) => struct_span_err!(self.tcx.sess, span, E0455, "{}", msg).emit(),
+        //        None => self.tcx.sess.err(msg),
+        //    }
+        //}
         if lib.cfg.is_some() && !self.tcx.features().link_cfg {
             feature_err(
                 &self.tcx.sess.parse_sess,
