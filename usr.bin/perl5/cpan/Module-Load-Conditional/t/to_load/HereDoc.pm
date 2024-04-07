package HereDoc;
use strict;
$HereDoc::VERSION = 1;

sub magic {
  print <<'END';
package Errno;
-use vars qw($VERSION);
-
-$VERSION = "1.111";
+our $VERSION = "1.111";
END
}

1;
