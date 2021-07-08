--- waflib/extras/autowaf.py.orig	2021-07-08 19:40:21 UTC
+++ waflib/extras/autowaf.py
@@ -70,7 +70,7 @@ def set_options(opt):
     opts.add_option('--bindir', type='string',
                     help="executable programs [default: PREFIX/bin]")
     opts.add_option('--configdir', type='string',
-                    help="configuration data [default: PREFIX/etc]")
+                    help="configuration data [default: /etc]")
     opts.add_option('--datadir', type='string',
                     help="shared data [default: PREFIX/share]")
     opts.add_option('--includedir', type='string',
@@ -521,7 +521,7 @@ def configure(conf):
     opts = Options.options
 
     config_dir('BINDIR',     opts.bindir,     os.path.join(prefix,  'bin'))
-    config_dir('SYSCONFDIR', opts.configdir,  os.path.join(prefix,  'etc'))
+    config_dir('SYSCONFDIR', opts.configdir,  os.path.join(prefix,  '/etc'))
     config_dir('DATADIR',    opts.datadir,    os.path.join(prefix,  'share'))
     config_dir('INCLUDEDIR', opts.includedir, os.path.join(prefix,  'include'))
     config_dir('LIBDIR',     opts.libdir,     os.path.join(prefix,  'lib'))
