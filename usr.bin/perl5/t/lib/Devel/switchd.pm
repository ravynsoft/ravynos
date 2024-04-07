package Devel::switchd;
use strict; BEGIN { } # use strict; BEGIN { ... } to incite [perl #21890]
sub import { print "import<@_>;" }
sub unimport { print "unimport<@_>;" }
package DB;
sub DB { print "DB<", join(",", caller), ">;" }
sub sub { print "sub<$DB::sub>;"; goto &$DB::sub }
1;

