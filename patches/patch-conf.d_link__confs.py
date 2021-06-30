--- conf.d/link_confs.py.orig	2021-06-29 09:23:26.764484000 -0400
+++ conf.d/link_confs.py	2021-06-29 09:33:56.016718000 -0400
@@ -11,7 +11,7 @@
     parser.add_argument('links', nargs='+')
     args = parser.parse_args()
 
-    confpath = os.path.join(os.environ['MESON_INSTALL_DESTDIR_PREFIX'], args.confpath)
+    confpath = '%%STAGEDIR%%' + args.confpath
 
     if not os.path.exists(confpath):
         os.makedirs(confpath)
