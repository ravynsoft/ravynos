package Devel::switchd_goto;
package DB;
sub DB { $^P |= 0x80; }
sub goto { print "goto<$DB::sub>;" }
1;

