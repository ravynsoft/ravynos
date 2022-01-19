--- usr.sbin/pkg/pkg.c.orig	2021-07-23 22:25:44.207062000 -0400
+++ usr.sbin/pkg/pkg.c	2021-07-23 22:26:05.685808000 -0400
@@ -871,7 +871,7 @@
 	for (int j = 0; bootstrap_names[j] != NULL; j++) {
 		bootstrap_name = bootstrap_names[j];
 
-		snprintf(url, MAXPATHLEN, "%s/Latest/%s", packagesite, bootstrap_name);
+		snprintf(url, MAXPATHLEN, "%s/%s", packagesite, bootstrap_name);
 		snprintf(tmppkg, MAXPATHLEN, "%s/%s.XXXXXX",
 		    getenv("TMPDIR") ? getenv("TMPDIR") : _PATH_TMP,
 		    bootstrap_name);
@@ -889,7 +889,7 @@
 			snprintf(tmpsig, MAXPATHLEN, "%s/%s.sig.XXXXXX",
 			    getenv("TMPDIR") ? getenv("TMPDIR") : _PATH_TMP,
 			    bootstrap_name);
-			snprintf(url, MAXPATHLEN, "%s/Latest/%s.sig",
+			snprintf(url, MAXPATHLEN, "%s/%s.sig",
 			    packagesite, bootstrap_name);
 
 			if ((fd_sig = fetch_to_fd(url, tmpsig, fetchOpts)) == -1) {
@@ -906,7 +906,7 @@
 			    "%s/%s.pubkeysig.XXXXXX",
 			    getenv("TMPDIR") ? getenv("TMPDIR") : _PATH_TMP,
 			    bootstrap_name);
-			snprintf(url, MAXPATHLEN, "%s/Latest/%s.pubkeysig",
+			snprintf(url, MAXPATHLEN, "%s/%s.pubkeysig",
 			    packagesite, bootstrap_name);
 
 			if ((fd_sig = fetch_to_fd(url, tmpsig, fetchOpts)) == -1) {
